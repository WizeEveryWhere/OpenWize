/**
  * @file: adm_mgr.c
  * @brief This file implement everything required to deal with administration
  * session.
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
  * @par 1.0.0 : 2019/11/21 09:06:26 [GBI]
  * Initial version
  *
  *
  */

#ifdef __cplusplus
extern "C" {
#endif

#include "adm_mgr.h"

#include "rtos_macro.h"

/*!
 * @cond INTERNAL
 * @{
 */
SYS_MUTEX_CREATE_DEF(admmgr);

static void _adm_mgr_ini_(struct ses_ctx_s *pCtx);
static uint32_t _adm_mgr_fsm_(struct ses_ctx_s *pCtx, uint32_t u32Evt);

/******************************************************************************/
#define SES_NAME "ADM"
#define ADM_MGR_EXP_TIMEOUT_MSK 0x10
/*!
 * @}
 * @endcond
 */

/*!
 * @addtogroup wize_admin_mgr
 * @{
 *
 */

/*!
 * @brief This function initialize the session context
 * @param [in] pCtx    Pointer in the current context
 *
 * @return None
 */
void AdmMgr_Setup(struct ses_ctx_s *pCtx)
{
	assert(pCtx);
	pCtx->hMutex = SYS_MUTEX_CREATE_CALL(admmgr);
	assert(pCtx->hMutex);
	pCtx->ini = _adm_mgr_ini_;
	pCtx->fsm = _adm_mgr_fsm_;
	pCtx->eState = SES_STATE_DISABLE;
	pCtx->eType = SES_ADM;
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
static void _adm_mgr_ini_(struct ses_ctx_s *pCtx)
{
	struct adm_mgr_ctx_s *pPrvCtx;
	assert(pCtx);
	pPrvCtx = (struct adm_mgr_ctx_s*)pCtx->pPrivate;
	assert(pPrvCtx);
	pPrvCtx->sRspMsg.pData = pPrvCtx->aSendBuff;
	pPrvCtx->sCmdMsg.pData = pPrvCtx->aRecvBuff;
	pPrvCtx->u8Pending = 0;
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
 * @retval SES_FLG_DATA_SENT (see @link ses_flag_e::SES_FLG_DATA_SENT @endlink)
 * @retval SES_FLG_RSP_SENT (see @link ses_flag_e::SES_FLG_RSP_SENT @endlink)
 * @retval SES_FLG_CMD_RECV (see @link ses_flag_e::SES_FLG_CMD_RECV @endlink)
 * @retval SES_FLG_OUT_DATE (see @link ses_flag_e::SES_FLG_OUT_DATE @endlink)
 *
 */
static uint32_t _adm_mgr_fsm_(struct ses_ctx_s *pCtx, uint32_t u32Evt)
{
	struct adm_mgr_ctx_s *pPrvCtx = (struct adm_mgr_ctx_s*)pCtx->pPrivate;
	uint32_t u32BackEvt = SES_FLG_NONE;
	ses_state_e ePrevState;

	ePrevState = pCtx->eState;

	if (u32Evt & SES_EVT_CLOSE)
	{
		TimeEvt_TimerStop(&pCtx->sTimeEvt);
		pCtx->eState = SES_STATE_IDLE;
		u32BackEvt = SES_FLG_COMPLETE;
	}

	if (u32Evt & SES_EVT_ADM_READY)
	{
		pPrvCtx->u8Pending |= ADM_RSP_READY;
	}

	switch(pCtx->eState)
	{
		case SES_STATE_DISABLE:
			break;
		case SES_STATE_IDLE: // From SES_STATE_IDLE : SES_FLG_ERROR, SES_FLG_NONE
			if (u32Evt & SES_EVT_OPEN)
			{
				pPrvCtx->u8ByPassCmd = 0;
				pPrvCtx->u8Pending = ADM_RSP_NONE;
				// send DATA
				if ( NetMgr_Send( &(pPrvCtx->sDataMsg), 1000 ) )
				{
					// failed, go back into IDLE
					pCtx->eState = SES_STATE_IDLE;
					u32BackEvt = SES_FLG_ERROR;
					break;
				}
				pCtx->eState = SES_STATE_SENDING;
			}
			break;
		case SES_STATE_WAITING_RX_DELAY: // From SES_STATE_WAITING_RX_DELAY : SES_FLG_ERROR, SES_FLG_SUCCESS
			if (u32Evt & SES_EVT_ADM_DELAY_EXPIRED)
			{
				if (!pPrvCtx->u8ByPassCmd)
				{
					pPrvCtx->sCmdMsg.u8Type = APP_ADMIN;
					if ( NetMgr_Listen(&(pPrvCtx->sCmdMsg), 5*pPrvCtx->u8ExchRxLength, NET_LISTEN_TYPE_DETECT) )
					{
						pCtx->eState = SES_STATE_IDLE;
						u32BackEvt = SES_FLG_ERROR;
						break;
					}
					pCtx->eState = SES_STATE_LISTENING;
				}
				else
				{
					pCtx->eState = SES_STATE_IDLE;
				}
			}
			break;
		case SES_STATE_LISTENING: // From SES_STATE_LISTENING : SES_FLG_ERROR, SES_FLG_NONE, SES_FLG_TIMEOUT
			if(u32Evt & SES_EVT_RECV_DONE)
			{
				u32BackEvt |= SES_FLG_CMD_RECV;
				pPrvCtx->u8Pending = ADM_RSP_PEND;
				LOG_INF("CMD Received\n");

				// check if delay before response
				if (pPrvCtx->u8ExchRespDelay)
				{
					// program an event to wait before send ADM RSP
					if ( TimeEvt_TimerStart(
							&pCtx->sTimeEvt,
							pPrvCtx->u8ExchRespDelay, 0,
							(uint32_t)SES_EVT_ADM_DELAY_EXPIRED
							))
					{
						pCtx->eState = SES_STATE_IDLE;
						u32BackEvt = SES_FLG_ERROR;
						break;
					}
				}
				// send immediately, but wait for timeout on response available
				else
				{
					// program an event to wait before send ADM RSP
					if ( TimeEvt_TimerStart(
							&pCtx->sTimeEvt,
							0, 30,
							(uint32_t)SES_EVT_ADM_DELAY_EXPIRED
							))
					{
						pCtx->eState = SES_STATE_IDLE;
						u32BackEvt = SES_FLG_ERROR;
						break;
					}
				}
				pCtx->eState = SES_STATE_WAITING_TX_DELAY;
			}

			if (u32Evt & SES_EVT_TIMEOUT)
			{
				pCtx->eState = SES_STATE_IDLE;
				u32BackEvt |= SES_FLG_TIMEOUT;
			}
			break;
		case SES_STATE_WAITING_TX_DELAY: // From SES_STATE_WAITING_TX_DELAY : SES_FLG_ERROR, SES_FLG_OUT_DATE
			if (u32Evt & SES_EVT_ADM_DELAY_EXPIRED)
			{
				// Check if response is ready to be send
				if (pPrvCtx->u8Pending & ADM_RSP_READY )
				{
					// Send RSP
					if ( NetMgr_Send( &(pPrvCtx->sRspMsg), 1000 ) )
					{
						// fail to send
						pCtx->eState = SES_STATE_IDLE;
						u32BackEvt = SES_FLG_ERROR;
						break;
					}
					pCtx->eState = SES_STATE_SENDING;
					break;
				}
				// else, response is out of date and will be send in the next window
				else
				{
					pCtx->eState = SES_STATE_IDLE;
					LOG_INF("RSP out of date\n");
					u32BackEvt |= SES_FLG_OUT_DATE;
				}
			}
			break;
		case SES_STATE_SENDING: // From SES_STATE_SENDING : SES_FLG_ERROR, SES_FLG_NONE, SES_FLG_RSP_SENT, SES_FLG_DATA_SENT, SES_FLG_TIMEOUT
			if (u32Evt & SES_EVT_SEND_DONE)
			{
				// if pending response
				if (pPrvCtx->u8Pending & ADM_RSP_PEND)
				{
					// RSP successfully sent
					pCtx->eState = SES_STATE_IDLE;
					pPrvCtx->u8Pending = ADM_RSP_NONE;
					LOG_INF("RSP sent\n");
					u32BackEvt |= SES_FLG_RSP_SENT;
				}
				else // that is DATA
				{
					if (!pPrvCtx->u8ByPassCmd)
					{
						// FIXME : with MCU low frequency, take into account the time required to initialize the PHY device
						if ( TimeEvt_TimerStart(
								&pCtx->sTimeEvt,
								pPrvCtx->u8ExchRxDelay, 0,
								(uint32_t)SES_EVT_ADM_DELAY_EXPIRED
								))
						{
							pCtx->eState = SES_STATE_IDLE;
							u32BackEvt = SES_FLG_ERROR;
						}
						else
						{
							pCtx->eState = SES_STATE_WAITING_RX_DELAY;
						}
					}
					else
					{
						pCtx->eState = SES_STATE_IDLE;
					}
					// DATA successfully sent
					LOG_INF("DATA sent\n");
					u32BackEvt |= SES_FLG_DATA_SENT;
				}
			}
			if (u32Evt & SES_EVT_TIMEOUT)
			{
				pCtx->eState = SES_STATE_IDLE;
				u32BackEvt |= SES_FLG_TIMEOUT;
				LOG_WRN("Send Timeout\n");
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
