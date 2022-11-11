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

/*!
 * @addtogroup wize_app
 * @{
 *
 */

/******************************************************************************/
/*!
 * @cond INTERNAL
 * @{
 */

static uint32_t _ses_disp_get_param_(void);
static uint32_t _check_ability_(struct ses_disp_ctx_s *pCtx);

// From these flags, it indicates that Net resource is available
const uint32_t u32ReleaseMsk =
	SES_FLG_DATA_SENT | SES_FLG_CMD_RECV | SES_FLG_RSP_SENT
	| SES_FLG_BLK_RECV
	| SES_FLG_PING_SENT
	| SES_FLG_SES_COMPLETE_MSK
	;

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
	assert(pCtx);

	InstMgr_Setup(&(pCtx->sSesCtx[SES_INST]));
	AdmMgr_Setup(&(pCtx->sSesCtx[SES_ADM]));
	DwnMgr_Setup(&(pCtx->sSesCtx[SES_DWN]));

	pCtx->eState = SES_DISP_STATE_DISABLE;
}

/******************************************************************************/
/*!
 * @brief This function initialize the SesDisp (Session Dispatcher) module
 *
 * @param [in] pCtx    Pointer on the current context
 * @param [in] bCtrl   Enable / Disable the Session dispatcher
 *
 * @return None
 */
void SesDisp_Init(struct ses_disp_ctx_s *pCtx, uint8_t bCtrl)
{
	uint8_t i;
	assert(pCtx);
	pCtx->pActive = NULL;
	pCtx->u8ActiveSes = 0;
	pCtx->forbidden_msk = SES_EVT_NONE;
	if (bCtrl)
	{
		pCtx->eState = SES_DISP_STATE_ENABLE;
	}
	else
	{
		pCtx->eState = SES_DISP_STATE_DISABLE;
	}
	for (i = 0; i < SES_NB; i++)
	{
		pCtx->sSesCtx[i].ini(&pCtx->sSesCtx[i], bCtrl);
		TimeEvt_TimerStop( &(pCtx->sSesCtx[i].sTimeEvt) );
	}
}

/*!
 * @static
 * @brief This function is the session dispatcher FSM that treat all events
 * from "outside" (Wize API, NetMgr, TimeEvt)?
 *
 * @details :
 *
 *
 *
 *
 * @param [in] pCtx     Pointer on the current session dispatcher context
 * @param [in] u32Event Received event to treat
 *
 * @return ORed Events (or flags) from each session (ADM, INST, DWN)
 */

/*
 * Use case 1 : maintenance window is defined
 * |             |       Already Open       |
 * |  Request    |  DWN   |  ADM   |  INST  |
 * |  --------:  | :----: | :----: | :----: |
 * |  DWN        |  X     |  X     |    X   |
 * |  ADM        |  A (a) |  X     |    X   |
 * |  INST       |  X     |  X     |    X   |
 * |  EXECPING   |  X     |  A     |    X   |
 * |  PERIO PING |  A (b) |  X     |    X   |
 *
 *   X : forbidden
 *   A : accept
 *   (a) : DATA only (no CMD, no RSP)
 *   (b) : after DWN session but during the maintenance window
 *
 * Use case 2 : no maintenance window is defined
 * |             |       Already Open       |
 * |  Request    |  DWN   |  ADM   | INST   |
 * |  --------:  | :----: | :----: | :----: |
 * |  DWN        |  X     |  X     |    X   |
 * |  ADM        |  A (1) |  X     |    X   |
 * |  INST       |  A (2) |  X     |    X   |
 * |  EXECPING   |  X     |  A     |    X   |
 * |  PERIO PING |  A (2) |  X     |    X   |
 *
 *   X : forbidden
 *   A : accept
 *   (1) : only if one of DATA or DATA+CMD or DATA+CMD+RSP timing is compatible with DWN ones
 *   (2) : only if INST timing is compatible with DWN ones, else next day
 *
 */

uint32_t SesDisp_Fsm(struct ses_disp_ctx_s *pCtx, uint32_t u32Event)
{
	uint32_t ulEvt;
	uint32_t ulSesFlg;

	uint32_t ulBckFlg;
	uint32_t filter_flg;

	uint32_t forbidden_msk;

	ses_type_t eReqId;

	ulBckFlg = SES_FLG_NONE;

	/**************************************************************************/
	// The ses_dispatcher is disable
	if (pCtx->eState == SES_DISP_STATE_DISABLE)
	{
		return ulBckFlg;
	}

	/**************************************************************************/
	// Treat Event from Net device
	/**************************************************************************/

	//--------------------------------------------------------------------------
	// init. session return flag
	ulSesFlg = SES_FLG_NONE;

	//--------------------------------------------------------------------------
	// Event from NetManager => active session already exist
	ulEvt = u32Event & SES_EVT_NET_MSK;
	if (ulEvt)
	{
		// Here, one message has been sent or received
		// Who is it for ?
		if (pCtx->pActive)
		{
			ulSesFlg |= pCtx->pActive->fsm(pCtx->pActive, ulEvt);
		}
	}
	// Clear Net Event
	u32Event &= ~SES_EVT_NET_MSK;

	//--------------------------------------------------------------------------
	// Check if Net resource should be released
	if (ulSesFlg & u32ReleaseMsk)
	{
		// release pCtx->pActive
		pCtx->pActive = NULL;
	}
	// Treat session's return flag
	//-----------------
	// DWN session flag
	// Treated outside // if (ulSesFlg & SES_FLG_BLK_RECV) { }
	//-----------------
	// ADM session flag
	// Treated outside or/and Don't care // if (ulSesFlg & SES_FLG_DATA_SENT) { }
	// Treated outside // if (ulSesFlg & SES_FLG_CMD_RECV) {}
	// Treated outside // if (ulSesFlg & SES_FLG_RSP_SENT) {}
	//-----------------
	// INST session flag
	// Treated outside or/and Don't care // if (ulSesFlg & SES_FLG_PING_SENT) { }
	// Treated outside // if (ulSesFlg & SES_FLG_PONG_RECV) {}


	/**************************************************************************/
	// Treat Event from TimeEvt and API (except OPEN)
	/**************************************************************************/

	//--------------------------------------------------------------------------
	// All other event except OPEN
	ulEvt = u32Event & (~(SES_EVT_SES_OPEN_MSK)) & SES_EVT_SES_MSK;
	if (ulEvt)
	{
		//------------------------------------------------
		// Who should become active ?
		if (ulEvt & SES_EVT_DWN_DELAY_EXPIRED)
		{
			if( pCtx->sSesCtx[SES_DWN].eState == SES_STATE_WAITING_RX_DELAY)
			{
				pCtx->pActive = &(pCtx->sSesCtx[SES_DWN]);
			}
		}

		if (ulEvt & SES_EVT_INST_DELAY_EXPIRED)
		{
			pCtx->pActive = &(pCtx->sSesCtx[SES_INST]);
		}

		if (ulEvt & SES_EVT_ADM_DELAY_EXPIRED)
		{
			// safe with :
			// - pPrvCtx->u8Pending & ADM_RSP_READY
			// - !pPrvCtx->u8ByPassCmd
			pCtx->pActive = &(pCtx->sSesCtx[SES_ADM]);
		}

		//------------------------------------------------
		// Treat Event : CANCEL, READY, DELAY_EXPIRED
		if (ulEvt & SES_EVT_DWN_MSK)
		{
			ulSesFlg |= pCtx->sSesCtx[SES_DWN].fsm(&(pCtx->sSesCtx[SES_DWN]), ulEvt);
		}

		if (ulEvt & SES_EVT_INST_MSK)
		{
			ulSesFlg |= pCtx->sSesCtx[SES_INST].fsm(&(pCtx->sSesCtx[SES_INST]), ulEvt);
		}

		if (ulEvt & SES_EVT_ADM_MSK)
		{
			ulSesFlg |= pCtx->sSesCtx[SES_ADM].fsm(&(pCtx->sSesCtx[SES_ADM]), ulEvt);
		}
	}
	// Clear All Event except OPEN
	u32Event &= SES_EVT_SES_OPEN_MSK;

	/**************************************************************************/
	// Treat OPEN Event from API
	/**************************************************************************/
	eReqId = SES_NONE;
	ulEvt = u32Event & SES_EVT_SES_OPEN_MSK;
	if (ulEvt)
	{
		forbidden_msk = 0;
		//---------------------------------------------------------------
		if (ulEvt & SES_EVT_DWN_OPEN)
		{
			if (pCtx->forbidden_msk & SES_EVT_DWN_OPEN)
			{
				// back reject
				ulBckFlg |= SES_FLG_DWN_ERROR;
			}
			else
			{
				eReqId = SES_DWN;
				pCtx->forbidden_msk |= SES_EVT_DWN_OPEN;
			}
		}
		else
		{
			// A DWN session is currently running
			if (pCtx->forbidden_msk & SES_EVT_DWN_OPEN)
			{
				// A DWN session is waiting, so potentially some time is available
				if ( (pCtx->sSesCtx[SES_DWN].eState == SES_STATE_WAITING) ||
					 (pCtx->sSesCtx[SES_DWN].eState == SES_STATE_WAITING_RX_DELAY))
				{
					// Update DWN TimeEvt value
					TimeEvt_UpdateTime(0);
					// Check if available time is enough for requested session
					forbidden_msk = _check_ability_(pCtx);
				}
				// A DWN session is listening
				else
				{
					// reject
					forbidden_msk = SES_EVT_ADM_OPEN | SES_EVT_INST_OPEN;
				}
			}
		}

		forbidden_msk |= pCtx->forbidden_msk;
		//---------------------------------------------------------------
		if (ulEvt & SES_EVT_ADM_OPEN)
		{
			if (forbidden_msk & SES_EVT_ADM_OPEN)
			{
				// back reject
				ulBckFlg |= SES_FLG_ADM_ERROR;
			}
			else
			{
				if(forbidden_msk & SES_EVT_ADM_DELAY_EXPIRED)
				{
					((struct adm_mgr_ctx_s*)(pCtx->sSesCtx[SES_ADM].pPrivate))->u8ByPassCmd = 1;
				}

				eReqId = SES_ADM;
				pCtx->forbidden_msk |= SES_EVT_ADM_OPEN | SES_EVT_INST_OPEN | forbidden_msk;
			}
		}
		//---------------------------------------------------------------
		if (ulEvt & SES_EVT_INST_OPEN)
		{
			if (forbidden_msk & SES_EVT_INST_OPEN)
			{
				// back reject
				ulBckFlg |= SES_FLG_INST_ERROR;
			}
			else
			{
				eReqId = SES_INST;
				pCtx->forbidden_msk |= SES_EVT_ADM_OPEN | SES_EVT_INST_OPEN | forbidden_msk;
			}
		}
	}

	/**************************************************************************/
	// Action
	/**************************************************************************/
	// Is there any session to open ?
	if (eReqId != SES_NONE)
	{
		// Check if we should open and configure the Net device
		if(!pCtx->u8ActiveSes)
		{
			NetMgr_Open(NULL);
			if ( _ses_disp_get_param_() != NETDEV_STATUS_OK)
			{
				ulBckFlg = SES_FLG_UNKNOWN_ERROR;
			}
		}

		//
		if ( ulBckFlg != SES_FLG_UNKNOWN_ERROR )
		{
			struct ses_ctx_s *pSes;
			pCtx->u8ActiveSes++;

			pSes = &(pCtx->sSesCtx[eReqId]);
			if (eReqId == SES_DWN)
			{
				ulEvt = SES_EVT_DWN_OPEN;
			}
			else
			{
				if (eReqId == SES_ADM)
				{
					ulEvt = SES_EVT_ADM_OPEN;
				}
				else if (eReqId == SES_INST)
				{
					ulEvt = SES_EVT_INST_OPEN;
				}
				pCtx->pActive = pSes;
			}

			ulSesFlg |= pSes->fsm(pSes, ulEvt);

		}
		eReqId = SES_NONE;
	}

	/**************************************************************************/
	// Treat Flags
	/**************************************************************************/

	if ( ulSesFlg & SES_FLG_INST_COMPLETE )
	{
		pCtx->forbidden_msk &= ~(SES_EVT_INST_MSK);
	}

	if (ulSesFlg & SES_FLG_ADM_COMPLETE )
	{
		pCtx->forbidden_msk &= ~(SES_EVT_ADM_MSK);
	}

	if (ulSesFlg & SES_FLG_DWN_COMPLETE )
	{
		pCtx->forbidden_msk &= ~(SES_EVT_DWN_MSK);
	}

	// At least one session is closed
	if (ulSesFlg & SES_FLG_SES_COMPLETE_MSK)
	{
		if(pCtx->u8ActiveSes)
		{
			pCtx->u8ActiveSes--;
		}

		if(!pCtx->u8ActiveSes)
		{
			NetMgr_Close();
		}
	}

	//--------------------------------------------------------------------------
	filter_flg = SES_FLG_RSP_SENT | SES_FLG_CMD_RECV | SES_FLG_BLK_RECV | SES_FLG_PONG_RECV;
	filter_flg |= SES_FLG_SES_MSK;

	//ulBckFlg = (ulBckFlg | ulSesFlg) & filter_flg;
	//ulSesFlg &= ~ulBckFlg;

	ulBckFlg = (ulBckFlg | ulSesFlg);

	return ulBckFlg;
}

/******************************************************************************/
/*!
 * @static
 * @brief This function get parameters from global table and setup internal variables.
 *
 * @return The NetDev status
 */
static uint32_t _ses_disp_get_param_(void)
{
	uint32_t ret = NETDEV_STATUS_OK;

	struct medium_cfg_s sMediumCfg;
	struct proto_config_s sProto_Cfg;

	Param_Access(RF_UPLINK_CHANNEL,    (uint8_t*)(&(sMediumCfg.eTxChannel)), 0 );
	sMediumCfg.eTxChannel = (sMediumCfg.eTxChannel -100)/10;
	Param_Access(RF_UPLINK_MOD,         (uint8_t*)(&(sMediumCfg.eTxModulation)), 0 );
	Param_Access(TX_POWER,              (uint8_t*)(&(sMediumCfg.eTxPower)), 0 );
	Param_Access(TX_FREQ_OFFSET,        (uint8_t*)&(sMediumCfg.i16TxFreqOffset), 0 );
	sMediumCfg.i16TxFreqOffset = __ntohs(sMediumCfg.i16TxFreqOffset);
	Param_Access(RF_DOWNLINK_CHANNEL,   (uint8_t*)(&(sMediumCfg.eRxChannel)), 0 );
	sMediumCfg.eRxChannel = (sMediumCfg.eRxChannel -100)/10;
	Param_Access(RF_DOWNLINK_MOD,       (uint8_t*)(&(sMediumCfg.eRxModulation)), 0 );
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

	return ret;
}

/*!
 * @static
 * @brief This function check the ability to run concurrently an other session
 * (INST or ADM) during a download session
 *
 * @details
 * Use case 1 : maintenance window is defined
 * |             |       Already Open       |
 * |  Request    |  DWN   |  ADM   |  INST  |
 * |  --------:  | :----: | :----: | :----: |
 * |  DWN        |  X     |  X     |    X   |
 * |  ADM        |  A (a) |  X     |    X   |
 * |  INST       |  X     |  X     |    X   |
 * |  EXECPING   |  X     |  A     |    X   |
 * |  PERIO PING |  A (b) |  X     |    X   |
 *
 *   X : forbidden
 *   A : accept
 *   (a) : DATA only (no CMD, no RSP)
 *   (b) : after DWN session but during the maintenance window
 *
 * Use case 2 : no maintenance window is defined
 * |             |       Already Open       |
 * |  Request    |  DWN   |  ADM   | INST   |
 * |  --------:  | :----: | :----: | :----: |
 * |  DWN        |  X     |  X     |    X   |
 * |  ADM        |  A (1) |  X     |    X   |
 * |  INST       |  A (2) |  X     |    X   |
 * |  EXECPING   |  X     |  A     |    X   |
 * |  PERIO PING |  A (2) |  X     |    X   |
 *
 *   X : forbidden
 *   A : accept
 *   (1) : only if one of DATA or DATA+CMD or DATA+CMD+RSP timing is compatible with DWN ones
 *   (2) : only if INST timing is compatible with DWN ones, else next day
 *
 * @param [in]
 * @param [in]
 *
 * @return  The "forbidden" mask
 */
static uint32_t _check_ability_(struct ses_disp_ctx_s *pCtx)
{
	uint32_t forbidden_msk = 0;

	if ( pCtx->u32InstDurationMs > pCtx->sSesCtx[SES_DWN].sTimeEvt.u64Value)
	{
		// INST session is forbidden
		forbidden_msk |= SES_EVT_INST_OPEN;
	}
	// else { /* Accept INST session */ }

	if ( (pCtx->u32DataDurationMs +1 ) > pCtx->sSesCtx[SES_DWN].sTimeEvt.u64Value)
	{
		// DATA+CMD+RSP are forbidden
		forbidden_msk |= SES_EVT_ADM_OPEN;
	}
	else
	{
		// Accept data
		if ( (pCtx->u32CmdDurationMs + pCtx->u32DataDurationMs +1 ) > pCtx->sSesCtx[SES_DWN].sTimeEvt.u64Value)
		{
			// DATA only, CMD and RSP are forbidden
			forbidden_msk |= SES_EVT_ADM_DELAY_EXPIRED;
		}
		else
		{
			// Accept cmd
			// - EXECPING will be rejected
			if ( (pCtx->u32RspDurationMs + pCtx->u32CmdDurationMs + pCtx->u32DataDurationMs +1 ) > pCtx->sSesCtx[SES_DWN].sTimeEvt.u64Value)
			{
				// DATA+CMD only, RSP is forbidden
				// - CMD WRITE_PARAMETER will not be effectively treated
				forbidden_msk |= SES_EVT_ADM_READY;
			}
			// else { /* Accept rsp */ }
		}
	}

	return forbidden_msk;
}

/*! @} */

#ifdef __cplusplus
}
#endif
