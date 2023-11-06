/**
  * @file time_evt.c
  * @brief This file implement a basic time event management.
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
  * @par 1.0.0 : 2020/09/09[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup time_evt
 * @{
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "time_evt.h"
#include <assert.h>

#include "rtos_macro.h"

/******************************************************************************/

/*!
 * @brief This struct define the TimeEvt context
 */
struct time_evt_ctx_s
{
	struct time_evt_s *pCurrent; //!< Pointer on the current timer
	void *pLock;                 //!< Lock the access to the context (RTOS)
	uint64_t u64LastUpdate;      //!< Last time update
};

/*!
 * @brief This define the TimeEvt context instance
 */
struct time_evt_ctx_s sTimeEvtCtx;


/*!
 * @cond INTERNAL
 * @{
 */

/*
SYS_MUTEX_CREATE_DEF(timeevt);
#define SETUP_LOCK() SYS_MUTEX_CREATE_CALL(timeevt)
*/
SYS_BINSEM_CREATE_DEF(timeevt);
#define SETUP_LOCK() SYS_BINSEM_CREATE_CALL(timeevt);

#define LOCK_ISR() sys_mutex_acquire_isr(sTimeEvtCtx.pLock)
#define UNLOCK_ISR() sys_mutex_release_isr(sTimeEvtCtx.pLock)
#define LOCK() sys_mutex_acquire(sTimeEvtCtx.pLock, TIME_EVT_LOCK_TMO)
#define UNLOCK() sys_mutex_release(sTimeEvtCtx.pLock)

void _reset(struct time_evt_ctx_s *pCtx);
void* _current(struct time_evt_ctx_s *pCtx);
void _update(struct time_evt_ctx_s *pCtx, uint64_t u64Now);
void _insert(struct time_evt_ctx_s *pCtx, struct time_evt_s *pNew);
void _remove(struct time_evt_ctx_s *pCtx, struct time_evt_s *pDel);

/*!
 * @}
 * @endcond
 */

/******************************************************************************/

/*!
  * @brief Initialize the given timer
  *
  * @param [in] pTimeEvt     Pointer a Timer to initialize
  * @param [in] pvTaskHandle Task handler to notify
  * @param [in] eCfg         The timer configuration
  * @retval 0 Success
  * @retval 1 Failed (given is/are null)
  */
#if defined ( __OS__ ) &&  ( OS_FreeRTOS == 1 )
uint8_t TimeEvt_TimerInit(time_evt_t *pTimeEvt, void *pvTaskHandle, time_evt_cfg_e eCfg)
#else
uint8_t TimeEvt_TimerInit(time_evt_t *pTimeEvt, void (*pvTaskHandle)(uint32_t evt), time_evt_cfg_e eCfg)
#endif
{
	if ( pTimeEvt && pvTaskHandle)
	{
		if (pTimeEvt->pNext)
		{
			LOCK();
			_remove(&sTimeEvtCtx, pTimeEvt);
			UNLOCK();
		}
		pTimeEvt->eCfg = eCfg;
		pTimeEvt->u64InitVal = 0;
		pTimeEvt->u64Value = 0;
		pTimeEvt->u32Event = 0;
		pTimeEvt->pvTaskHandle = pvTaskHandle;
		pTimeEvt->pNext = NULL;
		return 0;
	}
	else
	{
		return 1;
	}
}

/*!
  * @brief Start the given timer
  *
  * @param [in] pTimeEvt   Pointer a Timer to start (insert)
  * @param [in] u32Value   The Timer value in second
  * @param [in] i16DeltaMs The Delta timer value in millisecond
  * @param [in] u32Event   Event to notify on timer expire
  * @retval 0 Success
  * @retval 1 Failed (the given time value is passed or negative)
  */
uint8_t TimeEvt_TimerStart(time_evt_t *pTimeEvt, uint32_t u32Value, int16_t i16DeltaMs, uint32_t u32Event)
{
	uint64_t u64temp;
	time_evt_t *p = pTimeEvt;
	uint8_t ret = 0;
	// must be greater than 0 and lesser than 86399 second (1 day minus 1 second)

	u64temp = ((uint64_t)u32Value) *1000;

	if ( i16DeltaMs < 0 && (u64temp <= (-i16DeltaMs) ) )
	{
		p = NULL;
	}
	u64temp += (int64_t)i16DeltaMs;

	if (u64temp == 0)
	{
		p = NULL;
	}

	if ( p && (p->pNext == NULL) )
	{
		p->u32Event = u32Event;
		p->u64InitVal = u64temp;
		p->u64Value = u64temp;
		LOCK();
		// Stop Alarm
		_timer_stop(TIME_EVT_ALARM_ID);
		_get_current_time_ms(&u64temp);
		_update(&sTimeEvtCtx, u64temp);

		// case not absolute (so elapse)
		if( p->eCfg == TIMEEVT_CFG_ABSOLUTE)
		{
			// Current Epoch is before given absolute value
			if ( p->u64Value > u64temp )
			{
				p->u64Value -= u64temp;
			}
			// The given absolute value is in the passed time
			else
			{
				ret = 1;
				p = NULL;
			}
		}
		_insert(&sTimeEvtCtx, p);
		if (sTimeEvtCtx.pCurrent != NULL)
		{
			_timer_start(TIME_EVT_ALARM_ID, sTimeEvtCtx.pCurrent->u64Value);
		}
		UNLOCK();
	}
	else
	{
		ret = 1;
	}
	return ret;
}

/*!
  * @brief Stop the given timer
  *
  * @param [in] pTimeEvt Pointer a Timer to stop (remove)
  * @return None
  */
void TimeEvt_TimerStop(time_evt_t *pTimeEvt)
{
	uint64_t u64Now;
	if (pTimeEvt)
	{
		LOCK();
		if ( sTimeEvtCtx.pCurrent == pTimeEvt)
		{
			_timer_stop(TIME_EVT_ALARM_ID);
			sTimeEvtCtx.pCurrent = sTimeEvtCtx.pCurrent->pNext;
			pTimeEvt->pNext = NULL;
			_get_current_time_ms(&u64Now);
			_update(&sTimeEvtCtx, u64Now);
			if (sTimeEvtCtx.pCurrent != NULL)
			{
				_timer_start(TIME_EVT_ALARM_ID, sTimeEvtCtx.pCurrent->u64Value);
			}
		}
		else
		{
			_remove(&sTimeEvtCtx, pTimeEvt);
		}
		UNLOCK();
	}
}

/******************************************************************************/

/*!
  * @brief Initialize the TimeEvt Context
  *
  * @return None
  */
void TimeEvt_Setup(void)
{
	// create the mutex
	sTimeEvtCtx.pLock = SETUP_LOCK();
	assert(sTimeEvtCtx.pLock);
	TimeEvt_Init();
	// register RTC Alarm cb
	_timer_set_handler(TIME_EVT_ALARM_ID, TimeEvt_EventHandler);
}

/*!
  * @brief Initialize the TimeEvt module
  *
  * @return None
  */
void TimeEvt_Init(void)
{
	// Stop Alarm (just to be sure)
	_timer_stop(TIME_EVT_ALARM_ID);
	// clear the internal list
	_reset(&sTimeEvtCtx);
	UNLOCK();
}

/*!
  * @brief This function update time and internal structure
  *
  * @param [in] t The new time value to update
  *
  * @return None
  */
void TimeEvt_UpdateTime(time_t t)
{
	uint64_t u64TimeMs;
	struct time_evt_s *p;

	LOCK();
	p = sTimeEvtCtx.pCurrent;
	if (p != NULL)
	{
		_timer_stop(TIME_EVT_ALARM_ID);
		// Update the timers
		_get_current_time_ms(&u64TimeMs);
		_update(&sTimeEvtCtx, u64TimeMs);

		if (t)
		{
			u64TimeMs = (uint64_t)t * 1000;
			// Update absolute timers
			while (p != NULL)
			{
				if (p->eCfg == TIMEEVT_CFG_ABSOLUTE)
				{
					if (p->u64InitVal > u64TimeMs)
					{
						p->u64Value = p->u64InitVal - u64TimeMs;
					}
					else
					{
						p->u64Value = 0;
					}
				}
				p = p->pNext;
			}
		}
		// Update ref. time with the new one
		sTimeEvtCtx.u64LastUpdate = u64TimeMs;
	}

	if (t)
	{
		// Set the new clock
		_set_current_time(t);
	}
	// Restart Alarm
	p = sTimeEvtCtx.pCurrent;
	if (p != NULL)
	{
		_timer_start(TIME_EVT_ALARM_ID, p->u64Value);
	}
	UNLOCK();
}

/*!
  * @brief Callback handler on alarm interrupt
  *
  * @return None
  */
void TimeEvt_EventHandler(void)
{
	time_evt_t *pCurrent, *p;
	uint64_t u64Now;

	LOCK_ISR();
	if (sTimeEvtCtx.pCurrent != NULL)
	{
		_get_current_time_ms(&u64Now);
		_update(&sTimeEvtCtx, u64Now);
		// Now the pCurrent->u32Value = 0 and potentially other ones
		pCurrent = sTimeEvtCtx.pCurrent;
		do
		{
			p = pCurrent;
			pCurrent = pCurrent->pNext;
			if (p->u64Value == 0)
			{
				// Notify task with event
				if (p->pvTaskHandle != NULL)
				{
#if defined ( __OS__ ) && ( OS_FreeRTOS == 1 )
					sys_flag_set_isr(p->pvTaskHandle, p->u32Event);
#else
					p->pvTaskHandle(p->u32Event);
#endif
				}
				_remove(&sTimeEvtCtx, p);
				// if is periodic, reprogram it
				if ( p->eCfg ==  TIMEEVT_CFG_PERIODIC)
				{
					p->u64Value = p->u64InitVal;
					_insert(&sTimeEvtCtx, p);
					pCurrent = sTimeEvtCtx.pCurrent;
				}
			}
		} while (pCurrent != NULL);
	}

	if (sTimeEvtCtx.pCurrent != NULL)
	{
		_timer_start(TIME_EVT_ALARM_ID, sTimeEvtCtx.pCurrent->u64Value);
	}
	else
	{
		_timer_stop(TIME_EVT_ALARM_ID);
	}
	UNLOCK_ISR();
}

/******************************************************************************/

/*!
  * @static
  * @brief Reset the TimeEvt context
  *
  * @param [in] pCtx Pointer a TimeEvt context
  * @return None
  */
void _reset(struct time_evt_ctx_s *pCtx)
{
	if (pCtx)
	{
		pCtx->pCurrent = NULL;
		pCtx->u64LastUpdate = 0;
	}
}

/*!
  * @static
  * @brief Get the pointer on the current timer
  *
  * @param [in] pCtx Pointer a TimeEvt context
  * @return the pointer on the current timer
  */
void * _current(struct time_evt_ctx_s *pCtx)
{
	return (void*)(pCtx->pCurrent);
}

/*!
  * @static
  * @brief Update the TimeEvt
  *
  * @param [in] pCtx   Pointer a TimeEvt context
  * @param [in] u64Now Current time to update with
  * @return None
  */
void _update(struct time_evt_ctx_s *pCtx, uint64_t u64Now)
{
	struct time_evt_s *p;
	uint64_t u64Delta;
	p = pCtx->pCurrent;
	if (p == NULL)
	{
		pCtx->u64LastUpdate = u64Now;
	}

	u64Delta = u64Now - pCtx->u64LastUpdate;
	if ( u64Delta > 0)
	{
		while (p != NULL)
		{
			if (p->u64Value >= u64Delta)
			{
				p->u64Value -= u64Delta;
			}
			else {
				p->u64Value = 0;
			}
			p = p->pNext;
		}
	}
	pCtx->u64LastUpdate = u64Now;
}

/*!
  * @static
  * @brief Insert a Timer from the TimeEvt
  *
  * @param [in] pCtx Pointer a TimeEvt context
  * @param [in] pNew Pointer on Timer to insert
  * @return None
  */
void _insert(struct time_evt_ctx_s *pCtx, struct time_evt_s *pNew)
{
	struct time_evt_s *pCurrent, *p;
	if(pNew == NULL) { return; }

	pCurrent = pCtx->pCurrent;
	if(pCurrent) // at least one element exist
	{
		p = pCurrent;
		while(pCurrent != NULL)
		{
			// insert before
			if (pNew->u64Value < pCurrent->u64Value)
			{
				if (pCurrent == p) // no predecessor
				{
					pNew->pNext = pCurrent;
					pCtx->pCurrent = pNew;
					break;
				}
				else
				{
					pNew->pNext = p->pNext;
					p->pNext = pNew;
					break;
				}
			}
			// insert after
			else
			{
				if (pCurrent->pNext == NULL) // no successor
				{
					pCurrent->pNext = pNew;
					pNew->pNext = NULL;
					break;
				}
				else
				{
					p = pCurrent;
					pCurrent = pCurrent->pNext;
				}
			}
		}
	}
	else // the list is empty
	{
		pNew->pNext = NULL;
		pCtx->pCurrent = pNew;
	}
}

/*!
  * @static
  * @brief Remove a Timer from the TimeEvt
  *
  * @param [in] pCtx Pointer a TimeEvt context
  * @param [in] pDel Pointer on Timer to remove
  * @return None
  */
void _remove(struct time_evt_ctx_s *pCtx, struct time_evt_s *pDel)
{
	struct time_evt_s *p;
	if(pDel == NULL) { return; }

	p = pCtx->pCurrent;
	if (p)
	{
		if ( p == pDel)
		{
			pCtx->pCurrent = pCtx->pCurrent->pNext;
			pDel->pNext = NULL;
		}
		else
		{
			while (p->pNext != NULL)
			{
				if (pDel == p->pNext)
				{
					p->pNext = p->pNext->pNext;
					pDel->pNext = NULL;
					break;
				}
				p = p->pNext;
			}
		}
	}
	// else, list is empty
}

#ifdef __cplusplus
}
#endif

/*! @} */
