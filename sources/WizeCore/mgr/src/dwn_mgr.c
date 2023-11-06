/**
  * @file dwn_mgr.c
  * @brief This file implement everything required to deal with Download session.
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
  * @par 1.0.0 : 2021/03/15 12:14:27 [GBI]
  * Initial version
  *
  *
  */

#ifdef __cplusplus
extern "C" {
#endif

#include "dwn_mgr.h"

/*!
 * @addtogroup wize_dwn_mgr
 * @{
 *
 */

/******************************************************************************/
/*!
 * @cond INTERNAL
 * @{
 */

#define SES_NAME "DWN"

static void _dwn_mgr_ini_(struct ses_ctx_s *pCtx, uint8_t bCtrl);
static uint32_t _dwn_mgr_fsm_(struct ses_ctx_s *pCtx, uint32_t u32Evt);

static int32_t _dwn_mgr_nextday_(struct ses_ctx_s *pCtx);
static int32_t _dwn_mgr_adjustInit_(struct dwn_mgr_ctx_s *pCtx);

/*!
 * @}
 * @endcond
 */

/******************************************************************************/
/*!
 * @brief This function initialize the session context
 * @param [in] pCtx    Pointer in the current context
 *
 * @return None
 */
void DwnMgr_Setup(struct ses_ctx_s *pCtx)
{
	struct dwn_mgr_ctx_s *pPrvCtx;
	assert(pCtx);
	pPrvCtx = (struct dwn_mgr_ctx_s*)pCtx->pPrivate;
	assert(pPrvCtx);

	pCtx->ini = _dwn_mgr_ini_;
	pCtx->fsm = _dwn_mgr_fsm_;
	pCtx->eState = SES_STATE_DISABLE;
	pCtx->eType = SES_DWN;

	pPrvCtx->sRecvMsg.pData = pPrvCtx->aRecvBuff;
}

/******************************************************************************/
/*!
 * @static
 * @brief Initialize the fsm internal private context
 *
 * @param [in] pCtx    Pointer in the current context
 * @param [in] bCtrl   Enable / Disable the Session
 *
 * @return      None
 */
static void _dwn_mgr_ini_(struct ses_ctx_s *pCtx, uint8_t bCtrl)
{
	assert(pCtx);
	pCtx->eState = (bCtrl)?(SES_STATE_IDLE):(SES_STATE_DISABLE);
}

/*!
 * @static
 * @brief This is the FSM that treat input/output events
 *
 * @param [in] pCtx    Pointer in the current context
 * @param [in] u32Evt  Input event from outside (see ses_evt_e)
 *
 * @retval SES_FLG_NONE (see @link ses_flag_e::SES_FLG_NONE @endlink)
 * @retval SES_FLG_DWN_ERROR (see @link ses_flag_e::SES_FLG_DWN_ERROR @endlink)
 * @retval SES_FLG_DWN_COMPLETE (see @link ses_flag_e::SES_FLG_DWN_COMPLETE @endlink)
 * @retval SES_FLG_DWN_TIMEOUT (see @link ses_flag_e::SES_FLG_DWN_TIMEOUT @endlink)
 * @retval SES_FLG_BLK_RECV (see @link ses_flag_e::SES_FLG_BLK_RECV @endlink)
 * @retval SES_FLG_DWN_OUT_DATE (see @link ses_flag_e::SES_FLG_DWN_OUT_DATE @endlink)
 */
static uint32_t _dwn_mgr_fsm_(struct ses_ctx_s *pCtx, uint32_t u32Evt)
{
	struct dwn_mgr_ctx_s *pPrvCtx = (struct dwn_mgr_ctx_s*)pCtx->pPrivate;
	uint32_t u32BackEvt = SES_FLG_NONE;
	ses_state_e ePrevState;

	ePrevState = pCtx->eState;

	if (u32Evt & SES_EVT_DWN_CANCEL)
	{
		TimeEvt_TimerStop(&pCtx->sTimeEvt);
		pCtx->eState = SES_STATE_IDLE;
		u32BackEvt = SES_FLG_DWN_COMPLETE;
	}

	if (u32Evt & SES_EVT_DWN_READY)
	{
		pPrvCtx->u8Pending = 0;
	}

	switch(pCtx->eState)
	{
		case SES_STATE_DISABLE:
			break;
		case SES_STATE_IDLE:  // From SES_STATE_IDLE : SES_FLG_DWN_ERROR, SES_FLG_NONE
			if (u32Evt & SES_EVT_DWN_OPEN)
			{
				int32_t i32NextBlkOffset = _dwn_mgr_adjustInit_(pPrvCtx);
				if ( i32NextBlkOffset < 0 )
				{
					// no more block in this day and/or no more day available
					u32BackEvt = SES_FLG_DWN_COMPLETE;
					break;
				}

				pPrvCtx->u8Pending = 0;
				// Init. absolute timer
				TimeEvt_TimerInit( &pCtx->sTimeEvt, pCtx->hTask, TIMEEVT_CFG_ABSOLUTE);
				// Start the timer for the first download day
				if ( TimeEvt_TimerStart(
						&pCtx->sTimeEvt,
						pPrvCtx->_u32DayNext + i32NextBlkOffset,
						pPrvCtx->i16DeltaRxMs,
						(uint32_t)SES_EVT_DWN_DELAY_EXPIRED
						)
					)
				{
					// failed
					u32BackEvt = SES_FLG_DWN_COMPLETE | SES_FLG_DWN_ERROR;
					break;
				}
				pCtx->eState = SES_STATE_WAITING;
			}
			break;
		case SES_STATE_WAITING:  // From SES_STATE_WAITING : SES_FLG_DWN_ERROR, SES_FLG_NONE
			// Start New day
			if (u32Evt & SES_EVT_DWN_DELAY_EXPIRED)
			{
				// Program the periodic receiving window
#ifdef USE_DWN_PERIODIC
				TimeEvt_TimerInit( &pCtx->sTimeEvt, pCtx->hTask, TIMEEVT_CFG_PERIODIC);
				if ( TimeEvt_TimerStart(
						&pCtx->sTimeEvt,
						pPrvCtx->u8DeltaSec,
						pPrvCtx->i16DeltaRxMs,
						(uint32_t)SES_EVT_DWN_DELAY_EXPIRED
						))
				{
					pCtx->eState = SES_STATE_IDLE;
					u32BackEvt = SES_FLG_DWN_ERROR;
					break;
				}
#endif
				pCtx->eState = SES_STATE_WAITING_RX_DELAY;
			}
			//break;
		case SES_STATE_WAITING_RX_DELAY:  // From SES_STATE_WAITING_RX_DELAY : SES_FLG_DWN_ERROR
			if (u32Evt & SES_EVT_DWN_DELAY_EXPIRED)
			{
				// check if there are remaining block in this day
				if (pPrvCtx->_u16BlocksCount)
				{
					pPrvCtx->_u16BlocksCount--;
#ifndef USE_DWN_PERIODIC
					// Program the next receiving window
					uint32_t u32NextBlkOffset = pPrvCtx->u8DeltaSec * (pPrvCtx->u16BlocksCount - pPrvCtx->_u16BlocksCount);
					if ( TimeEvt_TimerStart(
							&pCtx->sTimeEvt,
							pPrvCtx->_u32DayNext + u32NextBlkOffset,
							pPrvCtx->i16DeltaRxMs,
							(uint32_t)SES_EVT_DWN_DELAY_EXPIRED
							))
					{
						pCtx->eState = SES_STATE_IDLE;
						u32BackEvt = SES_FLG_DWN_ERROR;
						break;
					}
#endif
					// Setup downlink
					if ( NetMgr_SetDwlink(pPrvCtx->u8ChannelId, pPrvCtx->u8ModulationId))
					{
						// failed
						u32BackEvt = SES_FLG_DWN_ERROR;
					}
					else
					{
						// Listen
						pPrvCtx->sRecvMsg.u8Type = APP_DOWNLOAD;
						if ( NetMgr_Listen(&(pPrvCtx->sRecvMsg), 1000*pPrvCtx->u8RxLength, NET_LISTEN_TYPE_ONE) )
						{
							// failed : keep it under WAITIN_RX_DELAY to try for the next block
							//pCtx->eState = SES_STATE_IDLE;
							u32BackEvt = SES_FLG_DWN_ERROR;
							break;
						}
						pCtx->eState = SES_STATE_LISTENING;
					}
				}
				else
				{
					/*
					 * From _dwn_mgr_nextday_ call :
					 * - pCtx->eState is set to SES_STATE_WAITING or SES_STATE_IDLE
					 * - periodic timer is stopped
					 * - absolute timer is started (if required)
					 */
					if ( _dwn_mgr_nextday_(pCtx) )
					{
						u32BackEvt = SES_FLG_DWN_ERROR;
						break;
					}
				}
			}
			break;
		case SES_STATE_LISTENING:  // From SES_STATE_LISTENING : SES_FLG_BLK_RECV, SES_FLG_NONE, SES_FLG_DWN_OUT_DATE, SES_FLG_DWN_TIMEOUT
			// A block is received
			if (u32Evt & SES_EVT_RECV_DONE)
			{
				if ( !pPrvCtx->u8Pending )
				{
					// Normally, timer for "days" or "delta sec" is still running
					u32BackEvt |= SES_FLG_BLK_RECV;
					pPrvCtx->u8Pending = 1;
					LOG_DBG("DWN BLOCK %x received\n", pPrvCtx->sRecvMsg.u16Id);
				}
				else
				{
					u32BackEvt |= SES_FLG_DWN_OUT_DATE;
				}
				pCtx->eState = SES_STATE_WAITING_RX_DELAY;
			}

			// Timeout, so no block has been received
			if (u32Evt & SES_EVT_TIMEOUT)
			{
				// Normally, timer for "days" or "delta sec" is still running
				pCtx->eState = SES_STATE_WAITING_RX_DELAY;
				u32BackEvt |= SES_FLG_DWN_TIMEOUT;
			}

			if (!pPrvCtx->_u16BlocksCount)
			{
				/*
				 * From _dwn_mgr_nextday_ call :
				 * - pCtx->eState is set to SES_STATE_WAITING or SES_STATE_IDLE
				 * - periodic timer is stopped
				 * - absolute timer is started (if required)
				 */
				if ( _dwn_mgr_nextday_(pCtx) )
				{
					u32BackEvt = SES_FLG_DWN_ERROR;
					break;
				}
			}

			break;
		default:
			break;
	}

	if (pCtx->eState != ePrevState)
	{
		LOG_DBG(SES_NAME" : %s\n", _ses_state_str_[pCtx->eState]);
		if (pCtx->eState == SES_STATE_IDLE)
		{
			u32BackEvt |= SES_FLG_DWN_COMPLETE;
		}
	}
	return u32BackEvt;
}

/******************************************************************************/
/*!
 * @static
 * @brief This function
 *
 * @param [in] pCtx    Pointer in the current context
 *
 * @retval -1 Filed to start the timer
 *          0 Otherwise
 *
 */
static int32_t _dwn_mgr_nextday_(struct ses_ctx_s *pCtx)
{
	struct dwn_mgr_ctx_s *pPrvCtx = (struct dwn_mgr_ctx_s*)pCtx->pPrivate;
	// Stop periodic timer
	TimeEvt_TimerStop(&pCtx->sTimeEvt);
	// check if there are remaining day
	if (pPrvCtx->_u8DayCount)
	{
		pPrvCtx->_u8DayCount--;
		pPrvCtx->_u32DayNext += 86400;
		pPrvCtx->_u16BlocksCount = pPrvCtx->u16BlocksCount;

		// Program the next receiving day
		TimeEvt_TimerInit( &pCtx->sTimeEvt, pCtx->hTask, TIMEEVT_CFG_ABSOLUTE);
		if ( TimeEvt_TimerStart(
				&pCtx->sTimeEvt,
				pPrvCtx->_u32DayNext, 0,
				(uint32_t)SES_EVT_DWN_DELAY_EXPIRED
				))
		{
			pCtx->eState = SES_STATE_IDLE;
			// Error
			return -1;
		}
		pCtx->eState = SES_STATE_WAITING;
	}
	else
	{
		// Session is done
		pCtx->eState = SES_STATE_IDLE;
	}
	return 0;
}

/*!
 * @static
 * @brief This function adjust context parameters in case download windows has
 *        already been started
 *
 * @param [in] pCtx    Pointer in the current context
 *
 * @retval offset value (in second) of the next block
 *         -1 if there is no more block or day in download windows
 */
static int32_t _dwn_mgr_adjustInit_(struct dwn_mgr_ctx_s *pCtx)
{
	time_t currentEpoch;
	int32_t i32NextBlkOffset;
	uint16_t u16BlocksPassed = 0;
	uint8_t u8DayPassed = 0;

	time(&currentEpoch);

	pCtx->_u8DayCount = pCtx->u8DayRepeat -1;
	pCtx->_u32DayNext = pCtx->u32DaysProg;
	pCtx->_u16BlocksCount = pCtx->u16BlocksCount;

	pCtx->_u32RemainInDay = 86400 - ( (pCtx->u16BlocksCount - 1 )* pCtx->u8DeltaSec + pCtx->u8RxLength);
	pCtx->_u32RemainInDay *= 1000;
	pCtx->_u32RemainInBlock = pCtx->u8DeltaSec - pCtx->u8RxLength;
	pCtx->_u32RemainInBlock *= 1000;

	// some day, some block have already been passed, so adjust that
	if (currentEpoch > pCtx->u32DaysProg)
	{
		// get the number of already passed days
		u8DayPassed = (currentEpoch - pCtx->u32DaysProg)/86400;
		// check if it remains repeat day
		if (u8DayPassed >= pCtx->u8DayRepeat)
		{
			// no more days available
			return -1;
		}
		// set the remains repeat days
		pCtx->_u8DayCount -= u8DayPassed;
		// set the "next day", that is the current day
		pCtx->_u32DayNext += u8DayPassed*86400;

		// get the number of already passed block
		u16BlocksPassed = (currentEpoch - pCtx->_u32DayNext)/pCtx->u8DeltaSec;
		u16BlocksPassed += ((currentEpoch - pCtx->_u32DayNext)%pCtx->u8DeltaSec)?(1):(0);

		// check that remains block in this day exist
		if (u16BlocksPassed >= pCtx->u16BlocksCount)
		{
			// pCtx->_u8DayCount does not take into account the "current day"
			if (pCtx->_u8DayCount > 1)
			{
				pCtx->_u8DayCount--;
				pCtx->_u32DayNext += 86400;
				u16BlocksPassed = 0;
			}
			else
			{
				// no more block in this day and no more day available
				return -1;
			}
		}
		pCtx->_u16BlocksCount -= u16BlocksPassed;
	}


	// the current download day has already began
	if (currentEpoch > pCtx->_u32DayNext)
	{
		// get the next starting block time
		i32NextBlkOffset = pCtx->u8DeltaSec - (currentEpoch - pCtx->_u32DayNext)%pCtx->u8DeltaSec;

		// check if delay to next block is enough
		if (i32NextBlkOffset < pCtx->u16InitDelayMinMs)
		{
			// if not enough then by pass one block more
			i32NextBlkOffset += pCtx->u8DeltaSec;
			pCtx->_u16BlocksCount--;
		}
	}
	// the current download day has not began
	else // currentEpoch <= pPrvCtx->_u32DayNext
	{
		i32NextBlkOffset = 0;
	}

	return i32NextBlkOffset;
}

/*! @} */

#ifdef __cplusplus
}
#endif
