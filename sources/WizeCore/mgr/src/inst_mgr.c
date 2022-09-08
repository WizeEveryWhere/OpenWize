/**
  * @file: inst_mgr.c
  * @brief This file implement everything required to deal with install session.
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

#include "inst_mgr.h"

#include "rtos_macro.h"

/*!
 * @cond INTERNAL
 * @{
 */
SYS_MUTEX_CREATE_DEF(instmgr);

static void _inst_mgr_ini_(struct ses_ctx_s *pCtx);
static uint32_t _inst_mgr_fsm_(struct ses_ctx_s *pCtx, uint32_t u32Evt);

/******************************************************************************/

#define SES_NAME "INST"
/*!
 * @}
 * @endcond
 */

/*!
 * @addtogroup wize_inst_mgr
 * @{
 *
 */

/*!
 * @brief This function initialize the session context
 * @param [in] pCtx    Pointer in the current context
 *
 * @return None
 */
void InstMgr_Setup(struct ses_ctx_s *pCtx)
{
	assert(pCtx);
	pCtx->hMutex = SYS_MUTEX_CREATE_CALL(instmgr);
	assert(pCtx->hMutex);
	pCtx->ini = _inst_mgr_ini_;
	pCtx->fsm = _inst_mgr_fsm_;
	pCtx->eState = SES_STATE_DISABLE;
	pCtx->eType = SES_INST;
	assert( 0 == TimeEvt_TimerInit( &pCtx->sTimeEvt, pCtx->hTask, TIMEEVT_CFG_ONESHOT) );
}

/*!
 * @static
 * @brief Initialize the fsm internal private context
 *
 * @param [in] pCtx    Pointer in the current context
 *
 * @return      None
 */
static void _inst_mgr_ini_(struct ses_ctx_s *pCtx)
{
	struct inst_mgr_ctx_s *pPrvCtx;
	assert(pCtx);
	pPrvCtx = (struct inst_mgr_ctx_s*)pCtx->pPrivate;
	assert(pPrvCtx);
	pPrvCtx->sRspMsg.pData = pPrvCtx->aRecvBuff;
	pPrvCtx->sCmdMsg.pData = pPrvCtx->aSendBuff;
	pCtx->eState = SES_STATE_IDLE;
}

/*!
 * @static
 * @brief This is the FSM that treat input/output events
 *
 * @param [in] pCtx    Pointer in the current context
 * @param [in] u32Evt Input event from outside (see ses_evt_e)
 *
 * @retval SES_FLG_NONE (see @link ses_flag_e::SES_FLG_NONE @endlink)
 * @retval SES_FLG_ERROR (see @link ses_flag_e::SES_FLG_ERROR @endlink)
 * @retval SES_FLG_COMPLETE (see @link ses_flag_e::SES_FLG_COMPLETE @endlink)
 * @retval SES_FLG_TIMEOUT (see @link ses_flag_e::SES_FLG_TIMEOUT @endlink)
 * @retval SES_FLG_PING_SENT (see @link ses_flag_e::SES_FLG_PING_SENT @endlink)
 * @retval SES_FLG_PONG_RECV (see @link ses_flag_e::SES_FLG_PONG_RECV @endlink)
 */
static uint32_t _inst_mgr_fsm_(struct ses_ctx_s *pCtx, uint32_t u32Evt)
{
	struct inst_mgr_ctx_s *pPrvCtx = (struct inst_mgr_ctx_s*)pCtx->pPrivate;
	uint32_t u32BackEvt = SES_FLG_NONE;
	ses_state_e ePrevState;

	ePrevState = pCtx->eState;

	if (u32Evt & SES_EVT_CLOSE)
	{
		TimeEvt_TimerStop(&pCtx->sTimeEvt);
		pCtx->eState = SES_STATE_IDLE;
		u32BackEvt = SES_FLG_COMPLETE;
	}

	if (u32Evt & SES_EVT_INST_READY)
	{
		pPrvCtx->u8Pending = 0;
		if (pCtx->eState == SES_STATE_LISTENING)
		{
			if ( NetMgr_ListenReady() )
			{
				// failed, go back into IDLE
				pCtx->eState = SES_STATE_IDLE;
				u32BackEvt = SES_FLG_ERROR;
			}
		}
	}

	switch(pCtx->eState)
	{
		case SES_STATE_DISABLE:
			break;
		case SES_STATE_IDLE: // From SES_STATE_IDLE : SES_FLG_ERROR, SES_FLG_NONE
			if (u32Evt & SES_EVT_OPEN)
			{
				pPrvCtx->u8Pending = 0;
				// send INST PING request
				if ( NetMgr_Send( &(pPrvCtx->sCmdMsg), 1000 ) )
				{
					// failed, go back into IDLE
					pCtx->eState = SES_STATE_IDLE;
					u32BackEvt = SES_FLG_ERROR;
					break;
				}
				pCtx->eState = SES_STATE_SENDING;
			}
			break;
		case SES_STATE_SENDING: // From SES_STATE_SENDING : SES_FLG_NONE, SES_FLG_ERROR, SES_FLG_PING_SENT, SES_FLG_TIMEOUT
			if (u32Evt & SES_EVT_SEND_DONE)
			{
				// Program an event to wait before listen INST PONG
				if ( TimeEvt_TimerStart(
						&pCtx->sTimeEvt,
						pPrvCtx->u8InstRxDelay, 0,
						(uint32_t)SES_EVT_INST_DELAY_EXPIRED
						))
				{
					pCtx->eState = SES_STATE_IDLE;
					u32BackEvt = SES_FLG_ERROR;
				}
				else
				{
					// set next state
					pCtx->eState = SES_STATE_WAITING_RX_DELAY;
					u32BackEvt |= SES_FLG_PING_SENT;
					LOG_INF("PING sent\n");
					LOG_DBG("\t-> DwnCh: %x; DwnMod: %x; RxRelay: %x; RxLen: %x\n"
						, pPrvCtx->sCmdMsg.pData[0]
						, pPrvCtx->sCmdMsg.pData[1]
						, pPrvCtx->sCmdMsg.pData[2]
						, pPrvCtx->sCmdMsg.pData[3]
						);
				}
			}
			if (u32Evt & SES_EVT_TIMEOUT)
			{
				pCtx->eState = SES_STATE_IDLE;
				u32BackEvt |= SES_FLG_TIMEOUT;
				LOG_WRN("Send Timeout\n");
			}
			break;
		case SES_STATE_WAITING_RX_DELAY: // From SES_STATE_WAITING_RX_DELAY : SES_FLG_NONE, SES_FLG_ERROR
			if (u32Evt & SES_EVT_INST_DELAY_EXPIRED)
			{
				pPrvCtx->sRspMsg.u8Type = APP_INSTALL;
				if ( NetMgr_Listen(&(pPrvCtx->sRspMsg), 1000*pPrvCtx->u8InstRxLength, NET_LISTEN_TYPE_MANY) )
				{
					// failed, go back into IDLE
					pCtx->eState = SES_STATE_IDLE;
					u32BackEvt = SES_FLG_ERROR;
					break;
				}
				// set next state
				pCtx->eState = SES_STATE_LISTENING;
			}
			break;
		case SES_STATE_LISTENING: // From SES_STATE_LISTENING : SES_FLG_NONE, SES_FLG_PONG_RECV, SES_FLG_TIMEOUT
			if (u32Evt & SES_EVT_RECV_DONE)
			{
				if ( ! pPrvCtx->u8Pending)
				{
					LOG_INF("PONG Received\n");
					LOG_DBG(
						"\t<- K: %02x %02x %02x %02x %02x %02x; MLAN: %02x; RSSI down: %02x; up: %02x\n"
						, ((inst_pong_t*)pPrvCtx->sRspMsg.pData)->L7ConcentId[0]
						, ((inst_pong_t*)pPrvCtx->sRspMsg.pData)->L7ConcentId[1]
						, ((inst_pong_t*)pPrvCtx->sRspMsg.pData)->L7ConcentId[2]
						, ((inst_pong_t*)pPrvCtx->sRspMsg.pData)->L7ConcentId[3]
						, ((inst_pong_t*)pPrvCtx->sRspMsg.pData)->L7ConcentId[4]
						, ((inst_pong_t*)pPrvCtx->sRspMsg.pData)->L7ConcentId[5]
						, ((inst_pong_t*)pPrvCtx->sRspMsg.pData)->L7ModemId
						, ((inst_pong_t*)pPrvCtx->sRspMsg.pData)->L7RssiDownstream
						, ((inst_pong_t*)pPrvCtx->sRspMsg.pData)->L7RssiUpstream
						);
					pPrvCtx->u8Pending = 1;
					u32BackEvt |= SES_FLG_PONG_RECV;
				}
				else
				{
					LOG_DBG("INST-> FRM lost\n");
				}			
			}
			if (u32Evt & SES_EVT_TIMEOUT)
			{
				// duration is passed
				pCtx->eState = SES_STATE_IDLE;
				u32BackEvt |= SES_FLG_TIMEOUT;
				break;
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
			u32BackEvt |= SES_FLG_COMPLETE;
		}
	}
	return u32BackEvt;
}

/*! @} */

#ifdef __cplusplus
}
#endif
