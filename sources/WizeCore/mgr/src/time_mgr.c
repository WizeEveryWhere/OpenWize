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

#include <stdio.h>
#include <time.h>
#include <machine/endian.h>

#include "time_mgr.h"

#include "time_evt.h"

/******************************************************************************/
/*!
 * @cond INTERNAL
 * @{
 */

static uint32_t _time_mgr_check_upd_time_(struct time_upd_s *pCtx, uint32_t cur_epoch, uint32_t *pStatus);

/*!
 * @}
 * @endcond
 */

/*!
 * @addtogroup wize_time_mgr
 * @{
 *
 */

/******************************************************************************/
/*!
 * @brief This is the main function, as FSM that treat events from periodic
 * wake-up timer
 *
 * @param [in, out] pCtx    The time update context to check
 * @param [in]      bNewDay If 0, then check if clock parameters have changed, else check for time update process,
 *
 * @retval TIME_FLG_NONE (see @link time_flg_e::TIME_FLG_NONE @endlink)
 * @retval TIME_FLG_CLOCK_ADJ (see @link time_flg_e::TIME_FLG_CLOCK_ADJ @endlink)
 * @retval TIME_FLG_OFFSET_ADJ (see @link time_flg_e::TIME_FLG_OFFSET_ADJ @endlink)
 * @retval TIME_FLG_DRIFT_ADJ (see @link time_flg_e::TIME_FLG_DRIFT_ADJ @endlink)
 * @retval TIME_FLG_DAY_PASSED (see @link time_flg_e::TIME_FLG_DAY_PASSED @endlink)
 * @retval TIME_FLG_CLOCK_CHANGE (see @link time_flg_e::TIME_FLG_CLOCK_CHANGE @endlink)
 * @retval TIME_FLG_OFFSET_CHANGE (see @link time_flg_e::TIME_FLG_OFFSET_CHANGE @endlink)
 * @retval TIME_FLG_DRIFT_CHANGE (see @link time_flg_e::TIME_FLG_DRIFT_CHANGE @endlink)
 *
 */
uint32_t TimeMgr_Main(time_upd_ctx_t *pCtx, uint8_t bNewDay)
{
	uint32_t eRet = TIME_FLG_NONE;
	time_t t;
	// update epoch with current epoch
	time( &t );
	t -=  pCtx->u32OffsetToUnix;

	if ( bNewDay )
	{
		// get current time
		pCtx->u32Epoch = _time_mgr_check_upd_time_(pCtx->pTimeUpd, (uint32_t)t, &eRet);

		// check if correction must be applied
		if ( !pCtx->u32Epoch ) // no clock correction pending, just one day passed
		{
			t = 0;
		}
		else // a clock correction is pending
		{
			// compute next_time to set;
			t = pCtx->u32OffsetToUnix + pCtx->u32Epoch;
		}
		TimeEvt_UpdateTime(t);
		pCtx->u32Epoch = 0;
	}
	else
	{
		// Parameters have been changed
		if ( *(pCtx->pCurDrift) != pCtx->pTimeUpd->drift_.drift )
		{
			// CLOCK_DRIFT_CORRECTION has changed
			pCtx->pTimeUpd->drift_.drift = *(pCtx->pCurDrift);
			pCtx->pTimeUpd->days_cnt = pCtx->pTimeUpd->drift_.period;
			eRet |= TIME_FLG_DRIFT_CHANGE;
		}

		if ( *(pCtx->pCurOffset) != 0 )
		{
			// CLOCK_OFFSET_CORRECTION has changed
			// save next offset to set
			pCtx->pTimeUpd->value = (uint32_t)__ntohs(*(pCtx->pCurOffset));
			// mark offset correction as pending
			pCtx->pTimeUpd->state_.offset_pend = 1;
			pCtx->pTimeUpd->state_.epoch_pend = 0;
			*(pCtx->pCurOffset) = 0;
			eRet |= TIME_FLG_OFFSET_CHANGE;
		}
		if (pCtx->u32Epoch)
		{
			if ( *(pCtx->pCurEpoch) != pCtx->u32Epoch )
			{
				// CLOCK_CURRENT_EPOC has changed
				// save next epoch to set
				pCtx->pTimeUpd->value = (uint32_t)__ntohl(*(pCtx->pCurEpoch));
				// mark epoch correction as pending
				pCtx->pTimeUpd->state_.epoch_pend = 1;
				pCtx->pTimeUpd->state_.offset_pend = 0;
				eRet |= TIME_FLG_CLOCK_CHANGE;
			}
		}
		pCtx->u32Epoch = __htonl(t);
		*(pCtx->pCurEpoch) = pCtx->u32Epoch;
	}
	return eRet;
}

/******************************************************************************/
/*!
  * @static
  * @brief This function check if there are any pending time/clock correction to process
  *
  * @param [in, out] pCtx      The time update context to check
  * @param [in]      cur_epoch The current epoch
  * @param [in, out] pStatus   The current status
  *
  * @return The corrected epoch value to apply; 0 if none;
  */
static uint32_t _time_mgr_check_upd_time_(struct time_upd_s *pCtx, uint32_t cur_epoch, uint32_t *pStatus)
{
	uint32_t epoch = 0;

	int16_t offset;
	int8_t drift;

	*pStatus = TIME_FLG_NONE;
	// check if epoch is pending
	if ( pCtx->state_.epoch_pend )
	{
		epoch = pCtx->value;
		*pStatus = TIME_FLG_CLOCK_ADJ;
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
				*pStatus = TIME_FLG_OFFSET_ADJ;
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
					*pStatus = TIME_FLG_DRIFT_ADJ;
				}
			}
		}
	}
	pCtx->value = 0;
	pCtx->state_.epoch_pend = 0;
	pCtx->state_.offset_pend = 0;
	return epoch;
}

/*! @} */

#ifdef __cplusplus
}
#endif
