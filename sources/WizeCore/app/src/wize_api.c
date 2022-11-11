/**
  * @file wize_api.c
  * @brief This file expose API to the external.
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
  * @par 1.0.0 : 2020/10/14[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup wize_api
 * @{
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <string.h>

#include "wize_api.h"
#include "rtos_macro.h"

#include "parameters.h"
#include "parameters_lan_ids.h"

#include "net_api_private.h"
#include "net_mgr.h"

#include "inst_mgr.h"
#include "adm_mgr.h"
#include "dwn_mgr.h"
#include "time_mgr.h"

#include "ses_dispatcher.h"

#include "wize_app.h"

#include "crypto.h"

/******************************************************************************/
#ifdef WIZEAPI_NOT_BLOCKING
	#ifndef WIZEAPI_INST_REQ_TMO
		#define WIZEAPI_INST_REQ_TMO 2 // in RTOS cycles
	#endif
#else
	#define WIZEAPI_INST_REQ_TMO 0xFFFFFFFFU // in RTOS cycles
	#define WIZEAPI_INST_FLG_TMO 0xFFFFFFFFU
	#define WIZEAPI_INST_FLG_ALL 0x0000FFFFU | (SES_INST << SES_MGR_FLG_POS)
#endif

#if WIZEAPI_INST_REQ_TMO < 2
#warning ("WIZEAPI_INST_REQ_TMO may be too small")
#endif
// -------------------------------

#ifdef WIZEAPI_NOT_BLOCKING
	#ifndef WIZEAPI_ADM_REQ_TMO
		#define WIZEAPI_ADM_REQ_TMO 2 // in RTOS cycles
	#endif
#else
	#define WIZEAPI_ADM_REQ_TMO 0xFFFFFFFFU
	#define WIZEAPI_ADM_FLG_TMO 0xFFFFFFFFU
	#define WIZEAPI_ADM_FLG_ALL 0x0000FFFFU | (SES_ADM << SES_MGR_FLG_POS)
#endif

#if WIZEAPI_ADM_REQ_TMO < 2
#warning ("WIZEAPI_ADM_REQ_TMO may be too small")
#endif
// -------------------------------

#ifdef WIZEAPI_NOT_BLOCKING
	#ifndef WIZEAPI_DWN_REQ_TMO
		#define WIZEAPI_DWN_REQ_TMO 2 // in RTOS cycles
	#endif
#else
	#define WIZEAPI_DWN_REQ_TMO 0xFFFFFFFFU
	#define WIZEAPI_DWN_FLG_TMO 0xFFFFFFFFU
	#define WIZEAPI_DWN_FLG_ALL 0x0000FFFFU | (SES_DWN << SES_MGR_FLG_POS)
#endif

#if WIZEAPI_ADM_REQ_TMO < 2
#warning ("WIZEAPI_ADM_REQ_TMO may be too small")
#endif
// -------------------------------

#ifndef WIZEAPI_SES_DISP_TMO
	#define WIZEAPI_SES_DISP_TMO 0xFFFFFFFF
#endif

#if WIZEAPI_SES_DISP_TMO < 2
#warning ("WIZEAPI_SES_DISP_TMO may be too small")
#endif
// -------------------------------

// Wize Api Time Task
#define TIME_MGR_TASK_STACK_SIZE 300
#define TIME_MGR_TASK_PRIORITY (UBaseType_t)(tskIDLE_PRIORITY+2)

#define TIME_MGR_EVT_PERIOD() 1000 // period in ms

#define TIME_MGR_TASK_NAME timemgr
#define TIME_MGR_TASK_FCT _time_mgr_main_

SYS_TASK_CREATE_DEF(timemgr, TIME_MGR_TASK_STACK_SIZE, TIME_MGR_TASK_PRIORITY);
// -------------------------------

// Wize Api Session Task
#ifndef WIZEAPI_STACK_SIZE
	#define WIZEAPI_STACK_SIZE 400
#endif

#ifndef WIZEAPI_PRIORITY
	#define WIZEAPI_PRIORITY (UBaseType_t)(tskIDLE_PRIORITY+2)
#endif

#define WIZEAPI_TASK_NAME wizeapi
#define WIZEAPI_TASK_FCT _wizeapi_ses_main_

SYS_TASK_CREATE_DEF(wizeapi, WIZEAPI_STACK_SIZE, WIZEAPI_PRIORITY);

SYS_BINSEM_CREATE_DEF(wizeapi_inst);
SYS_BINSEM_CREATE_DEF(wizeapi_adm);
SYS_BINSEM_CREATE_DEF(wizeapi_dwn);

/******************************************************************************/
static wize_net_t sNetCtx;
static struct ses_disp_ctx_s sSesDispCtx;
static struct time_upd_s sTimeUpdCtx;

static struct adm_mgr_ctx_s sAdmCtx;
static struct inst_mgr_ctx_s sInstCtx;
static struct dwn_mgr_ctx_s sDwnCtx;

static void *hTimeMgrTask;
static void *hWizeApiTask;
static void *hWizeApiLock[SES_NB];
static void *hWizeApiCaller[SES_NB];


/******************************************************************************/
static void _time_mgr_main_(void const * argument);
static void _wizeapi_ses_main_(void const * argument);

static wize_api_ret_e _wizeapi_ses_preinit_(uint8_t *pData, uint8_t u8Size, uint8_t u8Type);

inline
static uint32_t _wizeapi_get_frm_duration_(uint16_t u8Len, uint8_t bIsUpLink);


/******************************************************************************/
/******************************************************************************/
/*!
 * @brief This function start a INST (PING/PONG) session
 *
 * @param [in] pData  Pointer on raw ping to send
 * @param [in] u8Size Number of byte to send
 *
 * @retval return wize_api_ret_e::WIZE_API_SUCCESS (0) if everything is fine
 *         return wize_api_ret_e::WIZE_API_ACCESS_TIMEOUT (1) if access is refused
 *         return wize_api_ret_e::WIZE_API_INVALID_PARAM (2) if given parameter(s) is/are invalid
 */
wize_api_ret_e WizeApi_ExecPing(uint8_t *pData, uint8_t u8Size)
{
	wize_api_ret_e eRet = WIZE_API_ACCESS_TIMEOUT;
	// Ensure that only one request at the time
	if ( xSemaphoreTake( hWizeApiLock[SES_INST], WIZEAPI_INST_REQ_TMO ) )
	{
		eRet = _wizeapi_ses_preinit_(pData, u8Size, APP_INSTALL);
		if ( eRet == WIZE_API_SUCCESS)
		{
			hWizeApiCaller[SES_INST] = xTaskGetCurrentTaskHandle( );
			xTaskNotify(hWizeApiTask, SES_EVT_INST_OPEN, eSetBits);
		}
	}
	return eRet;
}

/*!
 * @brief This function send a DATA message.
 *
 * @details : The L6APP has to be at pData[0]
 *
 * @param [in] pData  Pointer on raw data to send
 * @param [in] u8Size Number of byte to send
 * @param [in] bPrio  Type of frame. 0 : DATA; DATA_PRIO for other values
 *
 * @retval return wize_api_ret_e::WIZE_API_SUCCESS (0) if everything is fine
 *         return wize_api_ret_e::WIZE_API_ACCESS_TIMEOUT (1) if access is refused
 *         return wize_api_ret_e::WIZE_API_INVALID_PARAM (2) if given parameter(s) is/are invalid
 */
wize_api_ret_e WizeApi_Send(uint8_t *pData, uint8_t u8Size, uint8_t bPrio)
{
	wize_api_ret_e eRet = WIZE_API_ACCESS_TIMEOUT;
	// Ensure that only one request at the time
	if ( xSemaphoreTake( hWizeApiLock[SES_ADM], WIZEAPI_ADM_REQ_TMO ) )
	{
		eRet = _wizeapi_ses_preinit_(pData, u8Size, ( (bPrio)?(APP_DATA_PRIO):(APP_DATA) ) );
		//hCaller[SES_ADM] = xTaskGetCurrentTaskHandle( );
		if ( eRet == WIZE_API_SUCCESS)
		{
			hWizeApiCaller[SES_ADM] = xTaskGetCurrentTaskHandle( );
			xTaskNotify(hWizeApiTask, SES_EVT_ADM_OPEN, eSetBits);
		}
	}
	return eRet;
}

/*!
 * @brief This function start a download session based on previously received ANN_DOWNLOAD
 *
 * @retval return wize_api_ret_e::WIZE_API_SUCCESS (0) if everything is fine
 *         return wize_api_ret_e::WIZE_API_ACCESS_TIMEOUT (1) if access is refused
 *         return wize_api_ret_e::WIZE_API_INVALID_PARAM (2) if given parameter(s) is/are invalid
 */
wize_api_ret_e WizeApi_Download(void)
{
	wize_api_ret_e eRet = WIZE_API_ACCESS_TIMEOUT;
	// Ensure that only one request at the time
	if ( xSemaphoreTake( hWizeApiLock[SES_DWN], WIZEAPI_DWN_REQ_TMO ) )
	{
		eRet = _wizeapi_ses_preinit_(NULL, 0, APP_DOWNLOAD);
		//hCaller[SES_ADM] = xTaskGetCurrentTaskHandle( );
		if ( eRet == WIZE_API_SUCCESS)
		{
			hWizeApiCaller[SES_DWN] = xTaskGetCurrentTaskHandle( );
			xTaskNotify(hWizeApiTask, SES_EVT_DWN_OPEN, eSetBits);
		}
	}
	return eRet;
}

void WizeApi_Notify(uint32_t evt)
{
	xTaskNotify(hWizeApiTask, evt, eSetBits);
}

/*!
 * @brief This function setup the wize stack
 **
 * @retval None
  */

void WizeApi_Setup(phydev_t *pPhyDev)
{
	// Create the Time Manager task
	hTimeMgrTask = SYS_TASK_CREATE_CALL(timemgr, _time_mgr_main_, &sTimeUpdCtx);
	assert(hTimeMgrTask);

	// Create session locks
	hWizeApiLock[SES_INST] = SYS_BINSEM_CREATE_CALL(wizeapi_inst);
	assert(hWizeApiLock[SES_INST]);
	hWizeApiCaller[SES_INST] = NULL;

	hWizeApiLock[SES_ADM] = SYS_BINSEM_CREATE_CALL(wizeapi_adm);
	assert(hWizeApiLock[SES_ADM]);
	hWizeApiCaller[SES_ADM] = NULL;

	hWizeApiLock[SES_DWN] = SYS_BINSEM_CREATE_CALL(wizeapi_dwn);
	assert(hWizeApiLock[SES_DWN]);
	hWizeApiCaller[SES_DWN] = NULL;

	// Create the Session dispatcher task
	hWizeApiTask = SYS_TASK_CREATE_CALL(wizeapi, WIZEAPI_TASK_FCT, NULL);
	assert(hWizeApiTask);

	// Setup Network Manager
	NetMgr_Setup(pPhyDev, &sNetCtx);

	// Setup the session dispacher
	sSesDispCtx.sSesCtx[SES_INST].pPrivate = &(sInstCtx);
	sSesDispCtx.sSesCtx[SES_ADM].pPrivate = &(sAdmCtx);
	sSesDispCtx.sSesCtx[SES_DWN].pPrivate = &(sDwnCtx);
	SesDisp_Setup(&sSesDispCtx);

	WizeApp_Init(&sAdmCtx.sCmdMsg, &sAdmCtx.sRspMsg, &sInstCtx.sRspMsg, &sDwnCtx.sRecvMsg);

	sSesDispCtx.hTask = hWizeApiTask;
}

/******************************************************************************/

void WizeApi_Enable(uint8_t bFlag)
{

}

/*!
 * @brief This function Clear the Time Update state
 *
 * @return None
 */
void WizeApi_CtxClear(void)
{
	memset((void*)(&sNetCtx), 0, sizeof(sNetCtx));
	BSP_Rtc_Backup_Write(0, (uint32_t)0);
	BSP_Rtc_Backup_Write(1, (uint32_t)0);
}

/*!
 * @brief This function Restore the Time Update state
 *
 * @return None
 */
void WizeApi_CtxRestore(void)
{
	((uint32_t*)&sTimeUpdCtx)[0] = BSP_Rtc_Backup_Read(0);
	((uint32_t*)&sTimeUpdCtx)[1] = BSP_Rtc_Backup_Read(1);
}

/*!
 * @brief This function Save the Time Update state
 *
 * @return None
 */
void WizeApi_CtxSave(void)
{
	BSP_Rtc_Backup_Write(0, ((uint32_t*)&sTimeUpdCtx)[0]);
	BSP_Rtc_Backup_Write(1, ((uint32_t*)&sTimeUpdCtx)[1]);
}


__attribute__((weak))
void WizeApi_OnSesFlag(void *hSesCaller, uint32_t u32Flg)
{
	if(hSesCaller)
	{
		sys_flag_set(hSesCaller, u32Flg);
	}
}

/******************************************************************************/

extern void _time_wakeup_enable(void);
extern void _time_wakeup_reload(void);
extern void _time_wakeup_force(void);
extern void _time_update_set_handler(pfTimeEvt_HandlerCB_t const pfCb);

// TIME_FLG_DAY_PASSED    = 0x10, /*!< A new day is passed */

static void _time_mgr_evtCb_(void);

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
	time_upd_ctx_t sCtx;
	uint32_t ulPeriod = pdMS_TO_TICKS(TIME_MGR_EVT_PERIOD());
	uint32_t bNewDay = 0;
	uint32_t eRet = 0;

	sCtx.pTimeUpd = (struct time_upd_s *)argument;
	sCtx.pCurEpoch  = (uint32_t*)Param_GetAddOf(CLOCK_CURRENT_EPOC);
	sCtx.pCurOffset = (uint16_t*)Param_GetAddOf(CLOCK_OFFSET_CORRECTION);
	sCtx.pCurDrift  = (uint16_t*)Param_GetAddOf(CLOCK_DRIFT_CORRECTION);
	sCtx.u32OffsetToUnix = EPOCH_UNIX_TO_OURS;

	assert(sCtx.pTimeUpd);
	assert(sCtx.pCurEpoch);
	assert(sCtx.pCurOffset);
	assert(sCtx.pCurDrift);

	// register the wakeup cb
	_time_update_set_handler(_time_mgr_evtCb_);
	_time_wakeup_enable();

	while (1)
	{
		eRet = TimeMgr_Main(&sCtx, bNewDay);
		if (bNewDay)
		{
			LOG_INF("TIME day update\n");
			if(eRet & 0x0F )
			{
				LOG_INF("TIME correction proceed\n");
			}
			// reprogram periodic wake-up timer
			_time_wakeup_reload();
		}
		else
		{
			if(eRet & TIME_FLG_CLOCK_CHANGE)
			{
				LOG_INF("TIME EPOCH corr. req.\n");
				if ( sCtx.pTimeUpd->state_.clock_init == 0 )
				{
					// first clock time setup, so set it immediately
					_time_wakeup_force();
					sCtx.pTimeUpd->state_.clock_init = 1;
				}
			}
			if(eRet & TIME_FLG_OFFSET_CHANGE)
			{
				LOG_INF("TIME OFFSET corr. req.\n");
			}
			if(eRet & TIME_FLG_DRIFT_CHANGE)
			{
				LOG_INF("TIME DRIFT corr. changed\n");
			}
		}
		// waiting for event
		bNewDay = ulTaskNotifyTake(pdTRUE, ulPeriod);
	}
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
	vTaskNotifyGiveFromISR(hTimeMgrTask, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
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
static void  _wizeapi_ses_main_(void const * argument)
{
	const uint32_t _ses_filter_msk_[SES_NB] =
	{
		(SES_FLG_INST_MSK | SES_FLG_SENDRECV_INST_MSK),
		(SES_FLG_ADM_MSK | SES_FLG_SENDRECV_ADM_MSK),
		(SES_FLG_DWN_MSK | SES_FLG_SENDRECV_DWN_MSK)
	};
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;
	uint32_t ulEvent;
	uint32_t u32Flag;
	uint32_t ulBckFlg;
	uint8_t i;

	SesDisp_Init(pCtx, 1);

	for (i = 0; i < SES_NB; i++)
	{
		xSemaphoreGive(hWizeApiLock[i]);
	}

	while(1)
	{
		if (sys_flag_wait(&ulEvent, WIZEAPI_SES_DISP_TMO))
		{
			u32Flag = SesDisp_Fsm(pCtx, ulEvent);
			for (i = 0; i < SES_NB; i++)
			{
				ulBckFlg = u32Flag & _ses_filter_msk_[i];
				if (ulBckFlg)
				{
					WizeApi_OnSesFlag(hWizeApiCaller[i], ulBckFlg);
					if ( ulBckFlg & SES_FLG_SES_COMPLETE_MSK )
					{
						hWizeApiCaller[i] = NULL;
						xSemaphoreGive(hWizeApiLock[i]);
					}
				}
			}
		}
	}
}

/******************************************************************************/
/*!
 * @static
 * @brief This function preinit the message and internal variables.
 *
 * @param [in]     pData  Pointer on raw data to send
 * @param [in]     u8Size Number of byte to send
 * @param [in]     u8Type Type of frame DATA, DATA_PRIO, INSTALL or DOWNLOAD
 *
 * @retval return wize_api_ret_e::WIZE_API_SUCCESS (0) if everything is fine
 *         return wize_api_ret_e::WIZE_API_INVALID_PARAM (2) if given parameter(s) is/are invalid
 */
static wize_api_ret_e _wizeapi_ses_preinit_(uint8_t *pData, uint8_t u8Size, uint8_t u8Type)
{
	uint8_t u8TxLenMax, u8RxLenMax;
	net_msg_t *pMsg;

	if (u8Type == APP_INSTALL)
	{
		if ( (pData == NULL) || (u8Size != sizeof(inst_ping_t)))
		{
			return WIZE_API_INVALID_PARAM;
		}
		pMsg = &sInstCtx.sCmdMsg;
		memcpy(pMsg->pData, pData, u8Size);
		pMsg->u8Size = u8Size;
		pMsg->u8Type = u8Type;
		pMsg->u16Id++;
		pMsg->u8KeyId = 0;
		Param_Access(PING_RX_DELAY, (uint8_t*)&(sInstCtx.u8InstRxDelay), 0);
		Param_Access(PING_RX_LENGTH, (uint8_t*)&(sInstCtx.u8InstRxLength), 0);

		sSesDispCtx.u32InstDurationMs = _wizeapi_get_frm_duration_(u8Size, 1);
		sSesDispCtx.u32InstDurationMs += sInstCtx.u8InstRxDelay*1000;
		sSesDispCtx.u32InstDurationMs += sInstCtx.u8InstRxLength*1000;
	}
	else if ( (u8Type == APP_DATA) || (u8Type == APP_DATA_PRIO))
	{
		Param_Access(L7TRANSMIT_LENGTH_MAX, (uint8_t*)&u8TxLenMax, 0);
		if ( (pData == NULL) || (u8Size > u8TxLenMax) )
		{
			return WIZE_API_INVALID_PARAM;
		}
		pMsg = &sAdmCtx.sDataMsg;
		memcpy(pMsg->pData, pData, u8Size);
		pMsg->u8Size = u8Size;
		pMsg->u8Type = u8Type;
		pMsg->u16Id++;
		Param_Access(CIPH_CURRENT_KEY,    (uint8_t*)&(pMsg->u8KeyId), 0);
		Param_Access(EXCH_RX_DELAY,       (uint8_t*)&(sAdmCtx.u8ExchRxDelay), 0);
		Param_Access(EXCH_RESPONSE_DELAY, (uint8_t*)&(sAdmCtx.u8ExchRespDelay), 0);
		Param_Access(EXCH_RX_LENGTH,      (uint8_t*)&(sAdmCtx.u8ExchRxLength), 0);

		Param_Access(L7RECEIVE_LENGTH_MAX, (uint8_t*)&u8RxLenMax, 0);

		sAdmCtx.u8ByPassCmd = (sAdmCtx.u8ExchRxLength)?(0):(1);

		sSesDispCtx.u32DataDurationMs = _wizeapi_get_frm_duration_(u8Size, 1);
		sSesDispCtx.u32CmdDurationMs = _wizeapi_get_frm_duration_(u8RxLenMax, 0);
		sSesDispCtx.u32CmdDurationMs += sAdmCtx.u8ExchRxDelay*1000;
		sSesDispCtx.u32RspDurationMs = _wizeapi_get_frm_duration_(u8TxLenMax, 1);
		sSesDispCtx.u32RspDurationMs += sAdmCtx.u8ExchRespDelay*1000;

		pMsg->Option_b.App = 1;
	}
	else if (u8Type == APP_DOWNLOAD)
	{
		admin_cmd_anndownload_t *pAnnReq = (admin_cmd_anndownload_t*)(sAdmCtx.sCmdMsg.pData);
		admin_rsp_t *pAnnRsp = (admin_rsp_t*)(sAdmCtx.sRspMsg.pData);

		// check that previous anndownload is valid
		int32_t isNotValid;
		isNotValid = pAnnReq->L7CommandId - pAnnRsp->L7ResponseId;
		isNotValid += pAnnReq->L7CommandId - ADM_ANNDOWNLOAD;
		isNotValid += pAnnRsp->L7ErrorCode;

		if ( isNotValid )
		{
			return WIZE_API_INVALID_PARAM;
		}

		// Set the klog
		Crypto_WriteKey( pAnnReq->L7Klog, KEY_LOG_ID);
		sDwnCtx.u8ChannelId    = (pAnnReq->L7ChannelId -100)/10;
		sDwnCtx.u16BlocksCount = __ntohs( *(uint16_t*)(pAnnReq->L7BlocksCount) );
		sDwnCtx.u8ModulationId = pAnnReq->L7ModulationId;
		sDwnCtx.u8DayRepeat    = pAnnReq->L7DayRepeat;
		sDwnCtx.u8DeltaSec     = pAnnReq->L7DeltaSec;
		sDwnCtx.u32DaysProg    = __ntohl( *(uint32_t*)(pAnnReq->L7DaysProg) ) + EPOCH_UNIX_TO_OURS;

		sDwnCtx.u32InitDelayMin = sAdmCtx.u8ExchRespDelay +1;
		sDwnCtx.u8DownRxLength = 1;
	}
	else
	{
		return WIZE_API_INVALID_PARAM;
	}
	return WIZE_API_SUCCESS;
}

/*!
 * @static
 * @brief This function compute (estimate) the frame duration
 *
 * @param [in] u8Len     The frame (L7 only) length
 * @param [in] bIsUpLink Up-link or down-link
 *
 * @return the frame duration in ms
 *
 */
inline static uint32_t _wizeapi_get_frm_duration_(uint16_t u8Len, uint8_t bIsUpLink)
{
	uint32_t frm_len;
	uint32_t rate;
	uint8_t mod;
	if (bIsUpLink)
	{
		Param_Access(RF_UPLINK_MOD, (uint8_t*)&mod, 0);
	}
	else
	{
		Param_Access(RF_DOWNLINK_MOD, (uint8_t*)&mod, 0);
	}
	// L6 len : 13 bytes
	// L2 len : 13 bytes
	// L1 len : 4 bytes (w2400, wm 4800) ; 32 bytes (wm6400)
	frm_len = u8Len + 30;
	rate = 2400;
	if (mod == 2)
	{
		frm_len += 28;
		rate = 6400;
	}
	else if (mod == 1)
	{
		rate = 4800;
	}
	return ( (frm_len *8*1000) / rate );
}

/******************************************************************************/

#ifdef __cplusplus
}
#endif

/*! @} */
