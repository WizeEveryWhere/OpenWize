/**
  * @file: time_mgr.c
  * @brief This file implement the time correction related functions
  * 
  * @details
  *
  * @copyright 2019, GRDF, Inc.  All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted (subject to the limitations in the disclaimer
  * below) provided that the following conditions are met:
  *    - Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *    - Redistributions in binary form must reproduce the above copyright 
  *      notice, this list of conditions and the following disclaimer in the 
  *      documentation and/or other materials provided with the distribution.
  *    - Neither the name of GRDF, Inc. nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  *
  * @par Revision history
  *
  * @par 1.0.0 : 2021/06/28[GBI]
  * Initial version
  *
  *
  */

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <machine/endian.h>

#include "time_mgr.h"
#include "rtos_macro.h"

#include "logger.h"

#include "app_layer.h"
#include "parameters.h"
#include "parameters_lan_ids.h"

/******************************************************************************/
/*!
 * @cond INTERNAL
 * @{
 */

#define TIME_MGR_MUTEX_TIMEOUT() 0x80000 // around 500k cycles
#define TIME_MGR_EVT_PERIOD() 1000 // period in ms

// Time update Task, Mutex,
#define TIME_MGR_TASK_NAME timemgr
#define TIME_MGR_TASK_FCT _time_mgr_main_
#define TIME_MGR_TASK_STACK_SIZE 300
#define TIME_MGR_TASK_PRIORITY (UBaseType_t)(tskIDLE_PRIORITY+2)
SYS_TASK_CREATE_DEF(timemgr, TIME_MGR_TASK_STACK_SIZE, TIME_MGR_TASK_PRIORITY);
SYS_MUTEX_CREATE_DEF(timemgr);

static void _time_mgr_main_(void const * argument);
static uint32_t _time_mgr_check_upd_time_(struct time_upd_s *pCtx, uint32_t cur_epoch);
static void _time_mgr_evtCb_(void);

/*!
 * @}
 * @endcond
 */

/*!
 * @addtogroup wize_time_mgr
 * @{
 *
 */

/*!
 * @brief This struct define the time manager task context
 */
struct time_mgr_ctx_s
{
	void *hTask;                 //!< Task handler
	void *hMutex;                //!< Mutex handler
};

/*!
 * @static
 * @brief This define the Time manager context
 */
static struct time_mgr_ctx_s sTimeMgrCtx;

/******************************************************************************/
/*!
 * @brief This function initialize the TimeMgr context
 *
 * @return None
 */
void TimeMgr_Setup(struct time_upd_s *pTimeUpdCtx)
{
	assert(pTimeUpdCtx);
	sTimeMgrCtx.hMutex = SYS_MUTEX_CREATE_CALL(timemgr);
	assert(sTimeMgrCtx.hMutex);
	sTimeMgrCtx.hTask = SYS_TASK_CREATE_CALL(timemgr, _time_mgr_main_, pTimeUpdCtx);
	assert(sTimeMgrCtx.hTask);
}

/******************************************************************************/

/*!
 * @static
 * @brief This is the main task function, as FSM that treat events from periodic
 * wake-up timer
 *
 * @param [in] argument (not used)
 *
 * @return      None
 */
static void _time_mgr_main_(void const * argument)
{
	struct time_upd_s *pCtx = (struct time_upd_s *)argument;

	uint32_t *pCurEpoch  = (uint32_t*)Param_GetAddOf(CLOCK_CURRENT_EPOC);
	uint16_t *pCurOffset = (uint16_t*)Param_GetAddOf(CLOCK_OFFSET_CORRECTION);
	uint16_t *pCurDrift  = (uint16_t*)Param_GetAddOf(CLOCK_DRIFT_CORRECTION);

	uint32_t ulPeriod = pdMS_TO_TICKS(TIME_MGR_EVT_PERIOD());
	uint32_t ulRet = 0;
	time_t t;
	uint32_t epoch = 0;

	assert(pCtx);

	// register the wakeup cb
	_time_update_set_handler(_time_mgr_evtCb_);
	_time_wakeup_enable();

	while (1)
	{
		// update epoch with current epoch
		time( &t );
		t -= EPOCH_UNIX_TO_OURS;

		if ( ulRet )
		{
			// get current time
			epoch = _time_mgr_check_upd_time_(pCtx, (uint32_t)t);

			// check if correction must be applied
			if ( !epoch ) // no clock correction pending, just one day passed
			{
				t = 0;
				LOG_INF("TIME day update\n");
			}
			else // a clock correction is pending
			{
				// compute next_time to set;
				t = EPOCH_UNIX_TO_OURS + epoch;
				//TimeMgr_TimeUpdSave(pCtx->state_.state);

				LOG_INF("TIME correction proceed\n");
			}
			TimeEvt_UpdateTime(t);
			epoch = 0;
			// reprogram periodic wake-up timer
			_time_wakeup_reload();
		}
		else
		{
			if ( *pCurDrift != pCtx->drift_.drift )
			{
				// CLOCK_DRIFT_CORRECTION has changed
				LOG_INF("TIME DRIFT corr. changed\n");
				pCtx->drift_.drift = *pCurDrift;
				pCtx->days_cnt = pCtx->drift_.period;
			}

			if ( *pCurOffset != 0 )
			{
				// CLOCK_OFFSET_CORRECTION has changed
				LOG_INF("TIME OFFSET corr. req.\n");
				// save next offset to set
				pCtx->value = (uint32_t)__ntohs(*pCurOffset);
				// mark offset correction as pending
				pCtx->state_.offset_pend = 1;
				pCtx->state_.epoch_pend = 0;
				*pCurOffset = 0;
			}
			if (epoch)
			{
				if ( *pCurEpoch != epoch )
				{
					// CLOCK_CURRENT_EPOC has changed
					LOG_INF("TIME EPOCH corr. req.\n");
					// save next epoch to set
					pCtx->value = (uint32_t)__ntohl(*pCurEpoch);
					// mark epoch correction as pending
					pCtx->state_.epoch_pend = 1;
					pCtx->state_.offset_pend = 0;
					if ( pCtx->state_.clock_init == 0 )
					{
						// first clock time setup, so set it immediately
						_time_wakeup_force();
						pCtx->state_.clock_init = 1;
					}
				}
			}
			epoch = __htonl(t);
			*pCurEpoch = epoch;
		}
		// waiting for event
		ulRet = ulTaskNotifyTake(pdTRUE, ulPeriod);
	}
}

/*!
  * @static
  * @brief This function check if there are pending time/clock correction
  *
  * @param [in] cur_epoch The current epoch
  *
  * @return The corrected epoch value to apply; 0 is none;
  */
static uint32_t _time_mgr_check_upd_time_(struct time_upd_s *pCtx, uint32_t cur_epoch)
{
	uint32_t epoch = 0;

	int16_t offset;
	int8_t drift;

	// check if epoch is pending
	if ( pCtx->state_.epoch_pend )
	{
		epoch = pCtx->value;
	}
	else
	{
		if ( pCtx->state_.offset_pend )
		{
			offset = (int16_t)(pCtx->value);
			if ( offset )
			{
				// get current epoch
				epoch = cur_epoch;
				if (offset < 0)
				{
					// check if negative offset is greater than epoch
					if (epoch < ((-1)*offset))
					{
						offset = (-1)*epoch;
					}
				}
				epoch += (int32_t)offset;
			}
		}
		else
		{
			pCtx->days_cnt--;
			if ( pCtx->days_cnt == 0)
			{
				pCtx->days_cnt = pCtx->drift_.period;
				drift = pCtx->drift_.value;
				if ( drift )
				{
					// get current epoch
					epoch = cur_epoch;
					if (drift < 0)
					{
						// check if negative drift is greater than epoch
						if (epoch < ((-1)*drift))
						{
							drift = (-1)*epoch;
						}
					}
					epoch += (int32_t)drift;
				}
			}
		}
	}
	pCtx->value = 0;
	pCtx->state_.epoch_pend = 0;
	pCtx->state_.offset_pend = 0;
	return epoch;
}

/*!
 * @static
 * @brief  ISR Callback function to notify an event occurs
 *
 * @return      None
 */
static void _time_mgr_evtCb_(void)
{
	BaseType_t xHigherPriorityTaskWoken;
	vTaskNotifyGiveFromISR(sTimeMgrCtx.hTask, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

/*! @} */

#ifdef __cplusplus
}
#endif
