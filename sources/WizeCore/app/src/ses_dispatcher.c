/**
  * @file: ses_dispatcher.c
  * @brief This file implement event session dispatching process
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
  * @par 1.0.0 : 2020/11/22[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup wize_app
 * @{
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "ses_dispatcher.h"

#include "inst_mgr.h"
#include "adm_mgr.h"
#include "dwn_mgr.h"

#include "net_mgr.h"
#include "parameters.h"
#include "parameters_lan_ids.h"
#include "crypto.h"

#include "wize_api.h"

#include "rtos_macro.h"

/******************************************************************************/
/*!
 * @cond INTERNAL
 * @{
 */

#define SES_DISP_TIMEOUT_EVT 0xFFFFFFFF
// Wize Session Dispatcher Task
#define SES_DISP_TASK_NAME sesdisp
#define SES_DISP_TASK_FCT _ses_disp_main_
#define SES_DISP_STACK_SIZE 400
#define SES_DISP_PRIORITY (UBaseType_t)(tskIDLE_PRIORITY+2)
SYS_TASK_CREATE_DEF(sesdisp, SES_DISP_STACK_SIZE, SES_DISP_PRIORITY);
SYS_BINSEM_CREATE_DEF(sesdisp);
SYS_EVENT_CREATE_DEF(sesdisp);

static void _ses_disp_main_(void const * argument);
static void _ses_disp_fsm_(struct ses_disp_ctx_s *pCtx, uint32_t u32Flag);
static void _ses_disp_bckflg_(struct ses_disp_ctx_s *pCtx, uint32_t u32Flag);
static uint32_t _ses_disp_postCmd_(struct ses_disp_ctx_s *pCtx);
static uint32_t _ses_disp_OnDayPass_(struct ses_disp_ctx_s *pCtx);

static void _ses_disp_get_param_(void);
static inline void _adm_mgr_get_param_(struct adm_mgr_ctx_s *pCtx);
static inline void _inst_mgr_get_param_(struct inst_mgr_ctx_s *pCtx);

/*!
 * @}
 * @endcond
 */

/******************************************************************************/

/*!
 * @brief This function setup the SesDisp (Session Dispatcher) module
 *
 * @param [in] pCtx Pointer on the current context
 *
 * @return None
 */
void SesDisp_Setup(struct ses_disp_ctx_s *pCtx)
{
	uint8_t i;
	assert(pCtx);

	pCtx->hTask = SYS_TASK_CREATE_CALL(sesdisp, SES_DISP_TASK_FCT, pCtx);
	assert(pCtx->hTask);
	pCtx->hLock = SYS_BINSEM_CREATE_CALL(sesdisp);
	assert(pCtx->hLock);
	pCtx->hEvents = SYS_EVENT_CREATE_CALL(sesdisp);
	assert(pCtx->hEvents);

	pCtx->sSesCtx[SES_INST].pPrivate = &(pCtx->sInstMgrCtx);
	pCtx->sSesCtx[SES_ADM].pPrivate = &(pCtx->sAdmMgrCtx);
	pCtx->sSesCtx[SES_DWN].pPrivate = &(pCtx->sDwnMgrCtx);

	for ( i = 0; i < SES_NB; i++)
	{
		pCtx->sSesCtx[i].hTask = pCtx->hTask;
		pCtx->sSesCtx[i].eType = i;
	}

	InstMgr_Setup(&(pCtx->sSesCtx[SES_INST]));
	AdmMgr_Setup(&(pCtx->sSesCtx[SES_ADM]));
	DwnMgr_Setup(&(pCtx->sSesCtx[SES_DWN]));

	pCtx->eState = SES_DISP_STATE_DISABLE;
}

/*!
 * @brief This function initialize the SesDisp (Session Dispatcher) module
 *
 * @param [in] pCtx    Pointer on the current context
 * @param [in] bEnable Enable / Disable the Session dispatcher
 *
 * @return None
 */
void SesDisp_Init(struct ses_disp_ctx_s *pCtx, uint8_t bEnable)
{
	uint8_t i;
	assert(pCtx);
	for (i = 0; i < 3; i++)
	{
		pCtx->sSesCtx[i].ini(&pCtx->sSesCtx[i]);
		TimeEvt_TimerStop( &(pCtx->sSesCtx[i].sTimeEvt) );
	}
	pCtx->pActive = NULL;
	pCtx->eActiveId = SES_NONE;
	pCtx->eReqId = SES_NONE;
	if (bEnable)
	{
		pCtx->eState = SES_DISP_STATE_ENABLE;
		xSemaphoreGive(pCtx->hLock);
	}
	else
	{
		pCtx->eState = SES_DISP_STATE_DISABLE;
	}
}

/******************************************************************************/

/*!
 * @static
 * @brief This is the session dispatcher task. It call its FSM to treat events from
 * Wize API and NetMgr
 *
 * @param [in] argument (not used)
 *
 * @return None
 */
static void _ses_disp_main_(void const * argument)
{
	uint32_t ulEvent;

	struct ses_disp_ctx_s *pCtx = (struct ses_disp_ctx_s *)argument;
	SesDisp_Init(pCtx, 1);
	while(1)
	{
		if (xTaskNotifyWait(0, ULONG_MAX, &ulEvent, SES_DISP_TIMEOUT_EVT ))
		{
			_ses_disp_fsm_(pCtx, ulEvent);
		}
	}
}

/******************************************************************************/

/*!
 * @static
 * @brief This is the session dispatcher FSM that treat events from
 * Wize API and NetMgr
 *
 * @param [in] pCtx     Pointer on the current session dispatcher context
 * @param [in] u32Event Received event to treat
 *
 * @return None
 */
static void _ses_disp_fsm_(struct ses_disp_ctx_s *pCtx, uint32_t u32Event)
{
	uint32_t ulEvt;
	uint32_t ulFlg;
	uint32_t ulBckFlg;


	// back from INSTALL On :
	// SES_EVT_CLOSE
	// SES_EVT_INST_READY
	// From SES_STATE_IDLE
	//    (ses)   - SES_EVT_OPEN                    : SES_FLG_ERROR
	// From SES_STATE_SENDING
	//    (net)   - SES_EVT_SEND_DONE               : SES_FLG_ERROR, SES_FLG_PING_SENT
	//    (net)   - SES_EVT_TIMEOUT                 : SES_FLG_TIMEOUT
	// From SES_STATE_WAITING_RX_DELAY
	//            - SES_EVT_INST_DELAY_EXPIRED      : SES_FLG_ERROR,
	// From SES_STATE_LISTENING
	//    (net)   - SES_EVT_RECV_DONE               : SES_FLG_PONG_RECV,
	//    (net)   - SES_EVT_TIMEOUT                 : SES_FLG_TIMEOUT

	// back from ADM On :
	// SES_EVT_CLOSE
	// SES_EVT_ADM_READY
	// From SES_STATE_IDLE
	//    (ses)   - SES_EVT_OPEN                    : SES_FLG_ERROR
	// From SES_STATE_WAITING_RX_DELAY
	//            - SES_EVT_ADM_DELAY_EXPIRED       : SES_FLG_ERROR, SES_FLG_SUCCESS
	// From SES_STATE_LISTENING
	//    (net)   - SES_EVT_RECV_DONE               : SES_FLG_ERROR,
	//    (net)   - SES_EVT_TIMEOUT                 : SES_FLG_TIMEOUT
	// From SES_STATE_WAITING_TX_DELAY
	//            - SES_EVT_ADM_DELAY_EXPIRED       : SES_FLG_ERROR, SES_FLG_OUT_DATE
	// From SES_STATE_SENDING
	//    (net)   - SES_EVT_SEND_DONE               : SES_FLG_ERROR, SES_FLG_RSP_SENT, SES_FLG_DATA_SENT
	//    (net)   - SES_EVT_TIMEOUT                 : SES_FLG_TIMEOUT

	// back from DWN on :
	// SES_EVT_CLOSE
	// SES_EVT_DWN_READY
	// From SES_STATE_IDLE
	//    (ses)  - SES_EVT_OPEN                    : SES_FLG_COMPLETE, SES_FLG_ERROR
	// From SES_STATE_WAITING
	//           - SES_EVT_DWN_DELAY_EXPIRED       : SES_FLG_ERROR
	// From SES_STATE_WAITING_RX_DELAY
	//           - SES_EVT_DWN_DELAY_EXPIRED       : SES_FLG_ERROR, SES_FLG_SUCCESS
	// From SES_STATE_LISTENING
	//    (net)  - SES_EVT_RECV_DONE               : SES_FLG_BLK_RECV
	//    (net)  - SES_EVT_TIMEOUT                 : SES_FLG_TIMEOUT


	ulBckFlg = SES_MGR_FLG_NONE;

	// Event from TimeMgr
	if (u32Event & SES_EVT_EXT_MSK)
	{
		if (u32Event & SES_EVT_DAY_PASSED)
		{
			ulBckFlg = _ses_disp_OnDayPass_(pCtx);
			if (ulBckFlg & GLO_FLG_PERIODIC_INST)
			{
				pCtx->eReqId = SES_INST;
			}
		}
	}

	// Event from API
	if (u32Event & SES_EVT_MSK)
	{
		ses_type_t eApiReqSesId = (u32Event & SES_MGR_EVT_MSK) >> SES_MGR_EVT_POS;

		// None active session exist
		if( pCtx->eActiveId == SES_NONE )
		{
			pCtx->eReqId = eApiReqSesId;

			pCtx->eActiveId = eApiReqSesId;
			pCtx->pActive = &(pCtx->sSesCtx[pCtx->eActiveId]);
			NetMgr_Open(NULL);
			_ses_disp_get_param_();

			if (eApiReqSesId == SES_INST)
			{
				struct inst_mgr_ctx_s *pPrvCtx = &(pCtx->sInstMgrCtx);
				// Init delays
				_inst_mgr_get_param_(pPrvCtx);

				// Init and prepare the message
				InstInt_Init(&(pCtx->sPingReplyCtx), &(pPrvCtx->sCmdMsg));
			}
			else
			{
				_adm_mgr_get_param_(&(pCtx->sAdmMgrCtx));
			}
			ulFlg = pCtx->pActive->fsm(pCtx->pActive, SES_EVT_OPEN);
			if (ulFlg & ( SES_FLG_COMPLETE | SES_FLG_ERROR) )
			{
				// the current active session is completed
				_ses_disp_bckflg_(pCtx, ulFlg);
			}
		}
		// An active session already exist => FAILED
		else
		{
			ulBckFlg = (eApiReqSesId <<  SES_MGR_FLG_POS) | SES_MGR_FLG_FAILED;
			xEventGroupSetBits(pCtx->hEvents, ulBckFlg);
		}
	}







	// Event from NetManager or TimerEvt Tmo => active session already exist
	if (u32Event & SES_EVT_NET_MGR_MSK)
	{
		if (pCtx->pActive)
		{
			ulFlg = pCtx->pActive->fsm(pCtx->pActive, u32Event);
			if (ulFlg & SES_FLG_CMD_RECV)
			{
				struct adm_mgr_ctx_s *pPrvCtx = &(pCtx->sAdmMgrCtx);
				// treat CMD and prepare RSP
				uint8_t eRet = AdmInt_PreCmd( &(pPrvCtx->sCmdMsg), &(pPrvCtx->sRspMsg) );
				if ( eRet == 0) // response not yet available (EXECPING case)
				{
					pCtx->bPendAction |= SES_ADM_RSP_PEND;
					//ulGlobalFlg |= SES_FLG_CMD_EXEC;

					/*
					struct inst_mgr_ctx_s *pPrvCtx = &(pCtx->sInstMgrCtx);
					// Init and prepare the message
					InstInt_Init(&(pCtx->sPingReplyCtx), &(pPrvCtx->sCmdMsg));
					pCtx->eActiveId = SES_INST;
					pCtx->pActive = &(pCtx->sSesCtx[pCtx->eActiveId]);
					ulFlg = pCtx->pActive->fsm(pCtx->pActive, SES_EVT_OPEN);
					*/

				}
				else
				{
					if ( eRet == 1) // response is available
					{
						pCtx->bPendAction |= SES_ADM_CMD_PEND;
					}
					else if ( eRet == 2) // action already done
					{
						pCtx->bPendAction &= ~(SES_ADM_CMD_PEND | SES_ADM_RSP_PEND);
					}
					// signal that RSP is ready
					// TODO : could return COMPLETE
					pCtx->pActive->fsm(pCtx->pActive, SES_EVT_ADM_READY);
				}
				pCtx->u16FullPower = 0;
			}

			if(ulFlg & SES_FLG_RSP_SENT)
			{
				if (pCtx->bPendAction & SES_ADM_CMD_PEND)
				{
					//ulGlobalFlg |= _ses_disp_postCmd_(pCtx);
					_ses_disp_postCmd_(pCtx);
					pCtx->bPendAction &= (~SES_ADM_CMD_PEND);
				}
			}

			if (ulFlg & SES_FLG_PONG_RECV)
			{
				struct inst_mgr_ctx_s *pPrvCtx = &(pCtx->sInstMgrCtx);
				// treat the received message that should be store in sRspMsg
				InstInt_Add(&(pCtx->sPingReplyCtx), &(pPrvCtx->sRspMsg));

				// signal that PONG msg has been treated, buffer is ready
				// TODO : could return COMPLETE
				pCtx->pActive->fsm(pCtx->pActive, SES_EVT_INST_READY);
			}

			if (ulFlg & SES_FLG_COMPLETE )
			{
				if(pCtx->eActiveId == SES_ADM)
				{
				}

				if(pCtx->eActiveId == SES_INST)
				{
					InstInt_End( &(pCtx->sPingReplyCtx) );
					// EXEC_PING was pending
					if (pCtx->bPendAction & ADM_RSP_PEND)
					{
						pCtx->bPendAction &= (~ADM_RSP_PEND);
						struct adm_mgr_ctx_s *pPrvCtx = &(pCtx->sAdmMgrCtx);
						if ( AdmInt_PreCmd(&(pPrvCtx->sCmdMsg), &(pPrvCtx->sRspMsg)) == 2 )
						{
							// signal that RSP is ready
							// TODO : could return COMPLETE
							// ...pCtx->pActive->fsm(pCtx->pActive, SES_EVT_ADM_READY);
						}
					}
					pCtx->u16PeriodInst = 0;
				}
			}

			if (ulFlg & ( SES_FLG_COMPLETE | SES_FLG_ERROR ) )
			{
				// the current active session is completed
				_ses_disp_bckflg_(pCtx, ulFlg);
			}
			// From NET_EVT : SEND_DONE, RECV_DONE
			// From ulFlag : CORRUPTED, PASSED
			// From ulFlag : TIMEOUT
		}
	}

	// Event from TimeEvt DELAY_EXPIRED => one session should become active
	if (u32Event & SES_EVT_SES_MGR_MSK)
	{

#if 0

		uint32_t u32Evt = u32Event & SES_EVT_SES_MGR_MSK;
		ses_type_t eReqSesId = SES_NONE;

		if (u32Event & SES_EVT_DWN_DELAY_EXPIRED)
		{
			eReqSesId = SES_DWN;
		}
		else if (u32Event & SES_EVT_ADM_DELAY_EXPIRED)
		{
			eReqSesId = SES_ADM;
		}
		else if (u32Event & SES_EVT_INST_DELAY_EXPIRED)
		{
			eReqSesId = SES_INST;
		}


		if (eReqSesId != pCtx->eActiveId)
		{
			// eReqSesId is not the active one, so fsm should give back COMPLETE flag
			ulFlg = pCtx->sSesCtx[eReqSesId].fsm(&(pCtx->sSesCtx[eReqSesId]), u32Evt);
			if ( !(ulFlg & SES_FLG_COMPLETE) )
			{
				// something wrong happen....force to close
				ulFlg = pCtx->sSesCtx[eReqSesId].fsm(&(pCtx->sSesCtx[eReqSesId]), SES_EVT_CLOSE);
			}
		}
		else
		{
			// eReqSesId is the active session, so pActive should be valid
			// check it in cases of
			if (pCtx->pActive)
			{
				ulFlg = pCtx->pActive->fsm(pCtx->pActive, u32Evt);
			}

		}

		if (ulFlg & SES_FLG_COMPLETE)
		{
			// If session was requested by API
			if (eReqSesId == pCtx->eReqId)
			{
				// then back notify it
				_ses_disp_bckflg_(pCtx, ulFlg);
			}
		}

#endif

		if( pCtx->pActive )
		{
			if (u32Event & SES_EVT_DWN_DELAY_EXPIRED)
			{
				pCtx->eActiveId = SES_DWN;
			}
			else if (u32Event & SES_EVT_ADM_DELAY_EXPIRED)
			{
				pCtx->eActiveId = SES_ADM;
			}
			else if (u32Event & SES_EVT_INST_DELAY_EXPIRED)
			{
				pCtx->eActiveId = SES_INST;
			}


			if ( pCtx->eActiveId != SES_NONE)
			{
				pCtx->pActive = &(pCtx->sSesCtx[pCtx->eActiveId]);
			}

			if (pCtx->pActive)
			{
				ulFlg = pCtx->pActive->fsm(pCtx->pActive, u32Event);
				if (ulFlg & ( SES_FLG_COMPLETE | SES_FLG_ERROR) )
				{
					_ses_disp_bckflg_(pCtx, ulFlg);
				}
			}
		}
	}
}

/*!
 * @static
 * @brief This is function send back to the caller the session result
 *
 * @param [in] pCtx    Pointer on the current session dispatcher context
 * @param [in] u32Flag
 *
 * @return None
 */
static void _ses_disp_bckflg_(struct ses_disp_ctx_s *pCtx, uint32_t u32Flag)
{
	uint32_t ulBckFlg;
	if (u32Flag & SES_FLG_ERROR)
	{
		//ulBckFlg = SES_MGR_FLG_FAILED;
		ulBckFlg = WIZE_API_FLG_FAILED;
	}
	//else if (u32Flag & SES_FLG_SENT)
	else if (u32Flag & SES_FLG_CMD_RECV)
	{
		//ulBckFlg = SES_MGR_FLG_REQUEST;
		ulBckFlg = WIZE_API_FLG_REQUEST;
	}
	else
	{
		//ulBckFlg = SES_MGR_FLG_SUCCESS;
		ulBckFlg = WIZE_API_FLG_SUCCESS;
	}
	//ulBckFlg |= (pCtx->eReqId << SES_MGR_FLG_POS);
	ulBckFlg |= (pCtx->eActiveId << SES_MGR_FLG_POS);

	// session is closed
	if (u32Flag & SES_FLG_COMPLETE)
	{
		pCtx->eReqId = SES_NONE;
		pCtx->eActiveId = SES_NONE;
		pCtx->pActive = NULL;
		NetMgr_Close();
		xSemaphoreGive(pCtx->hLock);

#ifdef WIZEAPI_NOT_BLOCKING
		if(pCtx->hCaller)
		{
			xTaskNotify(pCtx->hCaller, ulBckFlg, eSetBits);
		}
#else
		xEventGroupSetBits(pCtx->hEvents, ulBckFlg);
#endif
	}
}


/*!
 * @static
 * @brief This function effectively execute the "COMMAND" treatment
 *
 * @param [in]  pCtx Pointer on the current session dispatcher context
 *
 * @retval
 * @li @link glo_flg_e::GLO_FLG_NONE @endlink
 * @li @link glo_flg_e::GLO_FLG_CMD_RECV_READ @endlink
 * @li @link glo_flg_e::GLO_FLG_CMD_RECV_WRITE @endlink
 * @li @link glo_flg_e::GLO_FLG_CMD_RECV_KEY @endlink
 * @li @link glo_flg_e::GLO_FLG_CMD_RECV_ANN @endlink
 *
 */
static uint32_t _ses_disp_postCmd_(struct ses_disp_ctx_s *pCtx)
{
	uint32_t u32BckFlag = GLO_FLG_NONE;
	net_msg_t *pCmdMsg = &(pCtx->sAdmMgrCtx.sCmdMsg);
	net_msg_t *pRspMsg = &(pCtx->sAdmMgrCtx.sRspMsg);

	// Check if error, then do action
	if (((admin_rsp_t*)(pRspMsg->pData))->L7ErrorCode == ADM_NONE)
	{
		if (pCmdMsg->pData[0] == ADM_READ_PARAM)
		{
			u32BckFlag = GLO_FLG_CMD_RECV_READ;
		}
		else if (pCmdMsg->pData[0] == ADM_WRITE_PARAM)
		{
			uint8_t u8ParamId;
			// update the position with header
			uint8_t *pIn = &(pCmdMsg->pData[1]);

			while (pIn < &(pCmdMsg->pData[pCmdMsg->u8Size]))
			{
				// get paramId
				u8ParamId = *pIn;
				// update the position
				pIn++;
				// fill the parameter value
				Param_RemoteAccess(u8ParamId, pIn, 1);
				// get the parameter and update the position
				pIn += Param_GetSize(u8ParamId);
			}
			u32BckFlag = GLO_FLG_CMD_RECV_WRITE;
		}
		else if (pCmdMsg->pData[0] == ADM_WRITE_KEY)
		{
			if ( ((admin_cmd_writekey_t*)(pCmdMsg->pData))->L7KeyId == 0x01 )
			{
				Crypto_WriteKey( ((admin_cmd_writekey_t*)(pCmdMsg->pData))->L7KeyVal, KEY_MOB_ID );
			}
			else if ( ((admin_cmd_writekey_t*)(pCmdMsg->pData))->L7KeyId == 0x02 )
			{
				Crypto_WriteKey( ((admin_cmd_writekey_t*)(pCmdMsg->pData))->L7KeyVal, KEY_MAC_ID );
			}
			u32BckFlag = GLO_FLG_CMD_RECV_KEY;
		}
		else if (pCmdMsg->pData[0] == ADM_ANNDOWNLOAD)
		{
			admin_cmd_anndownload_t *pCmd = (admin_cmd_anndownload_t*)(pCmdMsg->pData);
			// Set the klog
			Crypto_WriteKey( pCmd->L7Klog, KEY_LOG_ID);
			pCtx->sDwnMgrCtx.u8ChannelId    = (pCmd->L7ChannelId -100)/10;
			pCtx->sDwnMgrCtx.u16BlocksCount = __ntohs( *(uint16_t*)(pCmd->L7BlocksCount) );
			pCtx->sDwnMgrCtx.u8ModulationId = pCmd->L7ModulationId;
			pCtx->sDwnMgrCtx.u8DayRepeat    = pCmd->L7DayRepeat;
			pCtx->sDwnMgrCtx.u8DeltaSec     = pCmd->L7DeltaSec;
			pCtx->sDwnMgrCtx.u32DaysProg    = __ntohl( *(uint32_t*)(pCmd->L7DaysProg) ) + EPOCH_UNIX_TO_OURS;
			pCtx->sDwnMgrCtx.u32HashSW      = __ntohl( *(uint32_t*)(pCmd->L7HashSW) );
			pCtx->sDwnMgrCtx.L7SwVersionTarget[0] = pCmd->L7SwVersionTarget[0];
			pCtx->sDwnMgrCtx.L7SwVersionTarget[1] = pCmd->L7SwVersionTarget[1];
			pCtx->sDwnMgrCtx.L7DwnId[0]     = pCmd->L7DwnId[0];
			pCtx->sDwnMgrCtx.L7DwnId[1]     = pCmd->L7DwnId[1];
			pCtx->sDwnMgrCtx.L7DwnId[2]     = pCmd->L7DwnId[2];

			pCtx->sDwnMgrCtx.u32InitDelayMin = pCtx->sAdmMgrCtx.u8ExchRespDelay +1;
			pCtx->sDwnMgrCtx.u8DownRxLength = 1;

			u32BckFlag = GLO_FLG_CMD_RECV_ANN;
		}
		else if (pCmdMsg->pData[0] == ADM_EXECINSTPING)
		{
			// do install
			//pCtx->sSesCtx[SES_INST].fsm(&(pCtx->sSesCtx[SES_INST]), SES_EVT_OPEN);
		}
		//else // nothing to do
	}
	if (u32BckFlag) {
		LOG_INF("%s CMD done\n", _ses_log_str_[_get_pos(u32BckFlag)]);
	}
	return u32BckFlag;
}

/*!
 * @static
 * @brief This is update "Periodic Install" and "Back to Full Power" counter
 *
 * @param [in] pCtx Pointer on the current session dispatcher context
 *
 * @retval
 * @li @link glo_flg_e::GLO_FLG_NONE @endlink
 * @li @link glo_flg_e::GLO_FLG_PERIODIC_INST @endlink
 * @li @link glo_flg_e::GLO_FLG_FULL_POWER @endlink
 *
 */
static uint32_t _ses_disp_OnDayPass_(struct ses_disp_ctx_s *pCtx)
{
	uint32_t ulBckFlg = GLO_FLG_NONE;
	uint8_t temp;
	uint16_t v;

	Param_Access(EXECPING_PERIODE, &temp, 0);
	v = temp*30;
	if (pCtx->u16PeriodInst >= v)
	{
		// request for periodic install
		ulBckFlg |= GLO_FLG_PERIODIC_INST;
	}
	else
	{
		pCtx->u16PeriodInst++;
	}

	Param_Access(TX_DELAY_FULLPOWER,  (uint8_t*)&(v), 0 );
	v = __ntohs(v);
	if (pCtx->u16FullPower >= v)
	{
		// go back in full power
		temp = PHY_PMAX_minus_0db;
		Param_Access(TX_POWER, &temp, 1 );

		ulBckFlg |= GLO_FLG_FULL_POWER;
		pCtx->u16FullPower = 0;
	}
	else
	{
		pCtx->u16FullPower++;
	}
	return ulBckFlg;
}

/*!
 * @static
 * @brief This function get parameters from global table and setup internal variables.
 *
 * @return      None
 */
static void _ses_disp_get_param_(void)
{
	int32_t ret = NETDEV_STATUS_OK;

	struct medium_cfg_s sMediumCfg;
	struct proto_config_s sProto_Cfg;

	Param_Access(RF_UPLINK_CHANNEL,     &(sMediumCfg.eTxChannel), 0 );
	sMediumCfg.eTxChannel = (sMediumCfg.eTxChannel -100)/10;
	Param_Access(RF_UPLINK_MOD,         &(sMediumCfg.eTxModulation), 0 );
	Param_Access(TX_POWER,              &(sMediumCfg.eTxPower), 0 );
	Param_Access(TX_FREQ_OFFSET,        (uint8_t*)&(sMediumCfg.i16TxFreqOffset), 0 );
	sMediumCfg.i16TxFreqOffset = __ntohs(sMediumCfg.i16TxFreqOffset);
	Param_Access(RF_DOWNLINK_CHANNEL,   &(sMediumCfg.eRxChannel), 0 );
	sMediumCfg.eRxChannel = (sMediumCfg.eRxChannel -100)/10;
	Param_Access(RF_DOWNLINK_MOD,       &(sMediumCfg.eRxModulation), 0 );
	ret = NetMgr_Ioctl(NETDEV_CTL_CFG_MEDIUM, (uint32_t)(&sMediumCfg));

	Param_Access(L7TRANSMIT_LENGTH_MAX, &(sProto_Cfg.u8TransLenMax), 0 );
	Param_Access(L7RECEIVE_LENGTH_MAX,  &(sProto_Cfg.u8RecvLenMax), 0 );
	Param_Access(L6NetwIdSelect,        &(sProto_Cfg.u8NetId), 0 );

	sProto_Cfg.AppInst = L6APP_INST;
	sProto_Cfg.AppAdm = L6APP_ADM;
#ifdef L6App
	Param_Access(L6App,                 &(sProto_Cfg.AppData), 0 ) );
#else
	sProto_Cfg.AppData = 0xFE;
#endif
	sProto_Cfg.filterDisL2 = 0;
	sProto_Cfg.filterDisL6 = 0;

	ret |= NetMgr_Ioctl(NETDEV_CTL_CFG_PROTO, (uint32_t)(&sProto_Cfg));

	if ( ret != NETDEV_STATUS_OK)
	{

	}
}

/*!
 * @static
 * @brief This function get parameters from global table and setup internal variables.
 *
 * @param [in] pCtx Pointer in the current context
 *
 * @return      None
 */
static inline void _adm_mgr_get_param_(struct adm_mgr_ctx_s *pCtx)
{
	Param_Access(EXCH_RX_DELAY,       (uint8_t*)&(pCtx->u8ExchRxDelay), 0);
	Param_Access(EXCH_RESPONSE_DELAY, (uint8_t*)&(pCtx->u8ExchRespDelay), 0);
	Param_Access(EXCH_RX_LENGTH,      (uint8_t*)&(pCtx->u8ExchRxLength), 0);
	Param_Access(CIPH_CURRENT_KEY,    (uint8_t*)&(pCtx->sDataMsg.u8KeyId), 0);

	pCtx->u8ByPassCmd = (pCtx->u8ExchRxLength)?(0):(1);
}

/*!
 * @static
 * @brief This function get parameters from global table and setup internal variables.
 *
 * @param [in] pCtx Pointer in the current context
 *
 * @return     None
 */
static inline void _inst_mgr_get_param_(struct inst_mgr_ctx_s *pCtx)
{
	Param_Access(PING_RX_DELAY, (uint8_t*)&(pCtx->u8InstRxDelay), 0);
	Param_Access(PING_RX_LENGTH, (uint8_t*)&(pCtx->u8InstRxLength), 0);
}

#ifdef __cplusplus
}
#endif

/*! @} */
