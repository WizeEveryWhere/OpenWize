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
#include "crypto.h"

#include "net_api_private.h"
#include "net_mgr.h"

#include "ses_dispatcher.h"

/******************************************************************************/
/*!
 * @cond INTERNAL
 * @{
 */

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
#ifndef TIME_MGR_TASK_STACK_SIZE
	#define TIME_MGR_TASK_STACK_SIZE 300
#endif

#ifndef TIME_MGR_TASK_PRIORITY
#if defined(configMAX_PRIORITIES)
	#define TIME_MGR_TASK_PRIORITY (UBaseType_t)(configMAX_PRIORITIES - 3)
#else
	#define TIME_MGR_TASK_PRIORITY (UBaseType_t)(tskIDLE_PRIORITY + 4)
#endif
#endif

#ifndef TIME_MGR_EVT_PERIOD
	#define TIME_MGR_EVT_PERIOD() 1000 // period in ms
#endif

#define TIME_MGR_TASK_NAME timemgr
#define TIME_MGR_TASK_FCT _time_mgr_main_

SYS_TASK_CREATE_DEF(timemgr, TIME_MGR_TASK_STACK_SIZE, TIME_MGR_TASK_PRIORITY);
// -------------------------------

// Wize Api Session Task
#ifndef SES_MGR_STACK_SIZE
	#define SES_MGR_STACK_SIZE 400
#endif

#ifndef SES_MGR_PRIORITY
#if defined(configMAX_PRIORITIES)
	#define SES_MGR_PRIORITY (UBaseType_t)(configMAX_PRIORITIES - 3)
#else
	#define SES_MGR_PRIORITY (UBaseType_t)(tskIDLE_PRIORITY + 4)
#endif
#endif

#define SES_MGR_TASK_NAME sesmgr
#define SES_MGR_TASK_FCT _ses_mgr_main_

SYS_TASK_CREATE_DEF(wizeapi, SES_MGR_STACK_SIZE, SES_MGR_PRIORITY);

SYS_BINSEM_CREATE_DEF(wizeapi_inst);
SYS_BINSEM_CREATE_DEF(wizeapi_adm);
SYS_BINSEM_CREATE_DEF(wizeapi_dwn);

/*!
 * @}
 * @endcond
 */

/******************************************************************************/
/*!
 * @cond INTERNAL
 * @{
 */
static wize_net_t sNetCtx;
static struct ses_disp_ctx_s sSesDispCtx;
static struct time_upd_ctx_s sTimeCtx;

static struct adm_mgr_ctx_s *_pAdmCtx_;
static struct inst_mgr_ctx_s *_pInstCtx_;
static struct dwn_mgr_ctx_s *_pDwnCtx_;

static void *hTimeMgrTask;
static void *hTimeMgrCaller;

static void *hSesMgrTask;
static void *hSesMgrLock[SES_NB];
static void *hSesMgrCaller[SES_NB];
/*!
 * @}
 * @endcond
 */

/******************************************************************************/
/*!
 * @cond INTERNAL
 * @{
 */

static void _time_mgr_main_(void const * argument);
static void _ses_mgr_main_(void const * argument);

static
wize_api_ret_e _wizeapi_ses_preinit_(uint8_t *pData, uint8_t u8Size, uint8_t u8Type);

inline
static uint32_t _wizeapi_get_frm_duration_(uint16_t u8Len, uint8_t bIsUpLink);

/*!
 * @}
 * @endcond
 */

/******************************************************************************/
/*!
 * @brief This function set the device identification
 *
 * @param[in] sDevId The device identification to set
 *
 * @retval return wize_api_ret_e::WIZE_API_SUCCESS (0)
 */
wize_api_ret_e WizeApi_SetDeviceId(device_id_t *pDevId)
{
	struct proto_ctx_s *pProtoCtx = NULL;
	pProtoCtx = &(sNetCtx.sProtoCtx);
	memcpy(pProtoCtx->aDeviceManufID, pDevId->aDevInfo, MFIELD_SZ);
	memcpy(pProtoCtx->aDeviceAddr, pDevId->aAddr, AFIELD_SZ);
	return WIZE_API_SUCCESS;
}

/*!
 * @brief This function get the device identification
 *
 * @param[in] pDevId Pointer on the device identification holder
 *
 * @retval return wize_api_ret_e::WIZE_API_SUCCESS (0)
  */
wize_api_ret_e WizeApi_GetDeviceId(device_id_t *pDevId)
{
	struct proto_ctx_s *pProtoCtx = NULL;
	pProtoCtx = &(sNetCtx.sProtoCtx);
	memcpy(pDevId->aManuf, pProtoCtx->aDeviceManufID, MFIELD_SZ);
	memcpy(pDevId->aAddr, pProtoCtx->aDeviceAddr, AFIELD_SZ);
	return WIZE_API_SUCCESS;
}

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
	if ( sys_binsen_acquire( hSesMgrLock[SES_INST], WIZEAPI_INST_REQ_TMO ) )
	{
		eRet = _wizeapi_ses_preinit_(pData, u8Size, APP_INSTALL);
		if ( eRet == WIZE_API_SUCCESS)
		{
			hSesMgrCaller[SES_INST] = sys_get_pid( );
			sys_flag_set(hSesMgrTask, SES_EVT_INST_OPEN);
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
	if ( sys_binsen_acquire( hSesMgrLock[SES_ADM], WIZEAPI_ADM_REQ_TMO ) )
	{
		eRet = _wizeapi_ses_preinit_(pData, u8Size, ( (bPrio)?(APP_DATA_PRIO):(APP_DATA) ) );
		if ( eRet == WIZE_API_SUCCESS)
		{
			hSesMgrCaller[SES_ADM] = sys_get_pid( );
			sys_flag_set(hSesMgrTask, SES_EVT_ADM_OPEN);
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
	if ( sys_binsen_acquire( hSesMgrLock[SES_DWN], WIZEAPI_DWN_REQ_TMO ) )
	{
		eRet = _wizeapi_ses_preinit_(NULL, 0, APP_DOWNLOAD);
		if ( eRet == WIZE_API_SUCCESS)
		{
			hSesMgrCaller[SES_DWN] = sys_get_pid( );
			sys_flag_set(hSesMgrTask, SES_EVT_DWN_OPEN);
		}
	}
	return eRet;
}

/*!
 * @brief Notify an event to the Session Manager
 *
 * @param [in] evt The event to notify
 *
 * @retval None
 */
void WizeApi_Notify(uint32_t evt)
{
	sys_flag_set(hSesMgrTask, evt);
}

/*!
 * @brief This function enable/disable the Wize Stack
 *
 * @retval None
 */
void WizeApi_Enable(uint8_t bFlag)
{
	SesDisp_Init(&sSesDispCtx, bFlag);
}

/*!
 * @brief This function get the current state of the given session
 *
 * @param [in] eSesId The session Id (see ses_type_t)
 *
 * @retval the session current state (see ses_state_e)
 */
uint32_t WizeApi_GetState(uint8_t eSesId)
{
	return (uint32_t)( (eSesId < SES_NB)?(sSesDispCtx.sSesCtx[eSesId].eState):(-1) );
}

/*!
 * @brief This function setup the Wize stack
 *
 * @param [in,out] pPhyDev  Pointer on the phydev context
 * @param [in,out] pInstCtx Pointer on Install session context
 * @param [in,out] pAdmCtx  Pointer on Admin session context
 * @param [in,out] pDwnCtx  Pointer on Download session context
 *
 * @retval None
 */
void WizeApi_SesMgr_Setup(
	phydev_t *pPhyDev,
	struct inst_mgr_ctx_s *pInstCtx,
	struct adm_mgr_ctx_s *pAdmCtx,
	struct dwn_mgr_ctx_s *pDwnCtx
	)
{
	assert(pInstCtx);
	assert(pAdmCtx);
	assert(pDwnCtx);

	_pInstCtx_ = pInstCtx;
	_pAdmCtx_  = pAdmCtx;
	_pDwnCtx_  = pDwnCtx;

	// Setup the session dispacher
	sSesDispCtx.sSesCtx[SES_INST].pPrivate = _pInstCtx_;
	sSesDispCtx.sSesCtx[SES_ADM].pPrivate = _pAdmCtx_;
	sSesDispCtx.sSesCtx[SES_DWN].pPrivate = _pDwnCtx_;
	SesDisp_Setup(&sSesDispCtx);

	// Setup Network Manager
	NetMgr_Setup(pPhyDev, &sNetCtx);

	// Create session locks
	hSesMgrLock[SES_INST] = SYS_BINSEM_CREATE_CALL(wizeapi_inst);
	assert(hSesMgrLock[SES_INST]);
	hSesMgrCaller[SES_INST] = NULL;

	hSesMgrLock[SES_ADM] = SYS_BINSEM_CREATE_CALL(wizeapi_adm);
	assert(hSesMgrLock[SES_ADM]);
	hSesMgrCaller[SES_ADM] = NULL;

	hSesMgrLock[SES_DWN] = SYS_BINSEM_CREATE_CALL(wizeapi_dwn);
	assert(hSesMgrLock[SES_DWN]);
	hSesMgrCaller[SES_DWN] = NULL;

	// Create the Session dispatcher task
	hSesMgrTask = SYS_TASK_CREATE_CALL(wizeapi, SES_MGR_TASK_FCT, NULL);
	assert(hSesMgrTask);
}

/*!
 * @brief This function notify the given task of the give event
 *
 * @details This function  is called by the Session manager task to notify caller
 * of session back event. This is weak function.
 *
 * @param [in] hSesCaller Task handler to notify
 * @param [in] u32Flg     Event to notify
 *
 * @retval None
 */
__attribute__((weak))
void WizeApi_OnSesFlag(void *hSesCaller, uint32_t u32Flg)
{
	if(hSesCaller)
	{
		sys_flag_set(hSesCaller, u32Flg);
	}
}

/******************************************************************************/
// Some convenient function

/*!
 * @brief This function cancel the given session
 *
 * @param [in] eSesId The session Id (see ses_type_t)
 *
 * @retval None
 */
inline
void WizeApi_Cancel(uint8_t eSesId)
{
	if(eSesId < SES_NB)
	{
		WizeApi_Notify( (SES_EVT_INST_CANCEL << (4 *eSesId)) );
	}
}

/*!
 * @brief This function cancel the Install session
 *
 * @retval None
 */
inline
void WizeApi_ExecPing_Cancel(void)
{
	WizeApi_Notify(SES_EVT_INST_CANCEL);
}

/*!
 * @brief This function cancel the Admin session
 *
 * @retval None
 */
inline
void WizeApi_Send_Cancel(void)
{
	WizeApi_Notify(SES_EVT_ADM_CANCEL);
}

/*!
 * @brief This function cancel the Download session
 *
 * @retval None
 */
inline
void WizeApi_Download_Cancel(void)
{
	WizeApi_Notify(SES_EVT_DWN_CANCEL);
}
/******************************************************************************/

/*!
 * @brief This function Clear the WizeApi context
 *
 * @return None
 */
__attribute__((weak))
void WizeApi_CtxClear(void)
{
}

/*!
 * @brief This function Restore the WizeApi context
 *
 * @return None
 */
__attribute__((weak))
void WizeApi_CtxRestore(void)
{
}

/*!
 * @brief This function Save the WizeApi context
 *
 * @return None
 */
__attribute__((weak))
void WizeApi_CtxSave(void)
{
}

/******************************************************************************/
/******************************************************************************/
/*!
 * @brief This function notify the previously registered task of the give event
 *
 * @details This function is called by the Time manager task to notify the
 * registered task of time back event. This is weak function.
 *
 * @param [in] u32Flg  Event to notify
 *
 * @retval None
 */
__attribute__((weak))
void WizeApi_OnTimeFlag(uint32_t u32Flg)
{
	if(hTimeMgrCaller)
	{
		sys_flag_set(hTimeMgrCaller, u32Flg);
	}
}

/*!
 * @brief This function registered a task to get back time event
 *
 * @param [in] hTask  Task handler to register
 *
 * @retval None
 */
__attribute__((weak))
void WizeApi_TimeMgr_Register(void *hTask)
{
	hTimeMgrCaller = hTask;
}

/*!
 * @brief Request for Time Update
 *
 * @retval None
 */
void WizeApi_TimeMgr_Update(uint32_t u32usDelay)
{
	// Force immediate
	sTimeCtx.pTimeUpd->state_.clock_init = 0;
	sys_flag_set(hTimeMgrTask, u32usDelay);
}

/*!
 * @brief This function setup the Wize stack
 *
 * @param [in,out] pTimeUpdCtx Pointer on the tume_upd context
 *
 * @retval None
 */
void WizeApi_TimeMgr_Setup(struct time_upd_s *pTimeUpdCtx)
{
	sTimeCtx.pTimeUpd = pTimeUpdCtx;
	assert(sTimeCtx.pTimeUpd);

	TimeEvt_Setup();
	// Create the Time Manager task
	hTimeMgrTask = SYS_TASK_CREATE_CALL(timemgr, TIME_MGR_TASK_FCT, NULL);
	assert(hTimeMgrTask);
	hTimeMgrCaller = NULL;
}

/******************************************************************************/
/******************************************************************************/

extern void _time_wakeup_enable(void);
extern void _time_wakeup_reload(void);
extern void _time_wakeup_force(uint32_t wakup_cycles);
extern void _time_update_set_handler(pfTimeEvt_HandlerCB_t const pfCb);

// TIME_FLG_DAY_PASSED    = 0x10, /*!< A new day is passed */

static void _time_mgr_evtCb_(void);

// FIXME
#define LSE_CLK_DIV2 16384 //UL

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
	uint32_t ulPeriod = pdMS_TO_TICKS(TIME_MGR_EVT_PERIOD());
	uint32_t bNewDay = 0;
	uint32_t eRet = 0;
	uint32_t usDelay = 0;

	assert(sTimeCtx.pTimeUpd);

	sTimeCtx.u32OffsetToUnix = EPOCH_UNIX_TO_OURS;
	sTimeCtx.pCurEpoch  = (uint32_t*)Param_GetAddOf(CLOCK_CURRENT_EPOC);
	sTimeCtx.pCurOffset = (uint16_t*)Param_GetAddOf(CLOCK_OFFSET_CORRECTION);
	sTimeCtx.pCurDrift  = (uint16_t*)Param_GetAddOf(CLOCK_DRIFT_CORRECTION);
	assert(sTimeCtx.pCurEpoch);
	assert(sTimeCtx.pCurOffset);
	assert(sTimeCtx.pCurDrift);

	sTimeCtx.hTask = sys_get_pid();
	// register the wakeup cb
	_time_update_set_handler(_time_mgr_evtCb_);
	_time_wakeup_enable();

	while (1)
	{
		eRet = TimeMgr_Main(&sTimeCtx, bNewDay);
		if (bNewDay)
		{
			LOG_DBG("TIME day update\n");
			if(eRet & 0x0F )
			{
				LOG_INF("TIME correction proceed\n");
			}
			// reprogram periodic wake-up timer
			_time_wakeup_reload();
			eRet |= TIME_FLG_DAY_PASSED;
		}
		else
		{
			if(eRet & TIME_FLG_CLOCK_CHANGE)
			{
				//LOG_DBG("TIME EPOCH corr. req.\n");
				LOG_DBG( "TIME EPOCH corr. req. to %d (local)\n",
						//__ntohl(*(sTimeCtx.pCurEpoch))
						sTimeCtx.pTimeUpd->value
						+ sTimeCtx.u32OffsetToUnix
					);

				if ( sTimeCtx.pTimeUpd->state_.clock_init == 0 )
				{
					// First clock time setup, so set it immediately
					/*
					 * Note :
					 * The following function force an RTC Wake-Up, which will
					 * have to effect to set the local "bNewDay" variable on
					 * the "next turn". Then, the function "TimeMgr_Main" will
					 * effectively correct the RTC clock with the new value.
					 *
					 */
					uint64_t cycDelay;
					cycDelay = (LSE_CLK_DIV2 * (uint64_t)usDelay) / 1000000 ;
					LOG_DBG("...in %d us (%d cyc)\n", usDelay, (uint32_t)cycDelay);
					_time_wakeup_force((uint32_t)cycDelay);
					sTimeCtx.pTimeUpd->state_.clock_init = 1;
				}
			}
			if(eRet & TIME_FLG_OFFSET_CHANGE)
			{
				LOG_DBG("TIME OFFSET corr. req.\n");
			}
			if(eRet & TIME_FLG_DRIFT_CHANGE)
			{
				LOG_DBG("TIME DRIFT corr. changed\n");
			}
		}

		if(eRet != TIME_FLG_NONE)
		{
			WizeApi_OnTimeFlag(eRet);
		}

		// waiting for event
		if ( sys_flag_wait(&usDelay, ulPeriod) )
		{
			bNewDay = (usDelay)?(0):(1);
		}
		else
		{
			bNewDay = 0;
			usDelay = 120;
		}
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
	//sys_flag_give_isr(hTimeMgrTask);
	sys_flag_set_isr(hTimeMgrTask, 0);
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
static void  _ses_mgr_main_(void const * argument)
{
	const uint32_t _ses_filter_msk_[SES_NB] =
	{
		(SES_FLG_INST_MSK | SES_FLG_SENDRECV_INST_MSK),
		(SES_FLG_ADM_MSK | SES_FLG_SENDRECV_ADM_MSK),
		(SES_FLG_DWN_MSK | SES_FLG_SENDRECV_DWN_MSK)
	};

	uint32_t ulEvent;
	uint32_t u32Flag;
	uint32_t ulBckFlg;
	uint8_t i;

	SesDisp_Init(&sSesDispCtx, 1);

	sSesDispCtx.hTask = sys_get_pid();
	for (i = 0; i < SES_NB; i++)
	{
		sys_binsen_release(hSesMgrLock[i]);
		sSesDispCtx.sSesCtx[i].hTask = sSesDispCtx.hTask;
	}

	while(1)
	{
		if (sys_flag_wait(&ulEvent, WIZEAPI_SES_DISP_TMO))
		{
			u32Flag = SesDisp_Fsm(&sSesDispCtx, ulEvent);
			for (i = 0; i < SES_NB; i++)
			{
				ulBckFlg = u32Flag & _ses_filter_msk_[i];
				if (ulBckFlg)
				{
					WizeApi_OnSesFlag(hSesMgrCaller[i], ulBckFlg);
					if ( ulBckFlg & SES_FLG_SES_COMPLETE_MSK )
					{
						hSesMgrCaller[i] = NULL;
						sys_binsen_release(hSesMgrLock[i]);
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
		pMsg = &_pInstCtx_->sCmdMsg;
		memcpy(pMsg->pData, pData, u8Size);
		pMsg->u8Size = u8Size;
		pMsg->u8Type = u8Type;
		pMsg->u16Id++;
		pMsg->u8KeyId = 0;
		Param_Access(PING_RX_DELAY, (uint8_t*)&(_pInstCtx_->u8InstRxDelay), 0);
		Param_Access(PING_RX_LENGTH, (uint8_t*)&(_pInstCtx_->u8InstRxLength), 0);

		sSesDispCtx.u32InstDurationMs = _wizeapi_get_frm_duration_(u8Size, 1);
		sSesDispCtx.u32InstDurationMs += _pInstCtx_->u8InstRxDelay*1000;
		sSesDispCtx.u32InstDurationMs += _pInstCtx_->u8InstRxLength*1000;
	}
	else if ( (u8Type == APP_DATA) || (u8Type == APP_DATA_PRIO))
	{
		/*
		 * TODO : case DATA_PRIO
		 * - if INST session is running, cancel then restart the session
		 * - if DWN session is running and required time is too short then standby the session
		 */
		Param_Access(L7TRANSMIT_LENGTH_MAX, (uint8_t*)&u8TxLenMax, 0);
		if ( (pData == NULL) || (u8Size > u8TxLenMax) )
		{
			return WIZE_API_INVALID_PARAM;
		}
		pMsg = &_pAdmCtx_->sDataMsg;
		memcpy(pMsg->pData, pData, u8Size);
		pMsg->u8Size = u8Size;
		pMsg->u8Type = u8Type;
		pMsg->u16Id++;
		Param_Access(CIPH_CURRENT_KEY,    (uint8_t*)&(pMsg->u8KeyId), 0);
		Param_Access(EXCH_RX_DELAY,       (uint8_t*)&(_pAdmCtx_->u8ExchRxDelay), 0);
		Param_Access(EXCH_RESPONSE_DELAY, (uint8_t*)&(_pAdmCtx_->u8ExchRespDelay), 0);
		Param_Access(EXCH_RX_LENGTH,      (uint8_t*)&(_pAdmCtx_->u8ExchRxLength), 0);

		Param_Access(L7RECEIVE_LENGTH_MAX, (uint8_t*)&u8RxLenMax, 0);

#ifdef HAS_WIZE_CORE_EXTEND_PARAMETER
		int16_t tmp;
		Param_Access(ADM_RECEPTION_OFFSET, (uint8_t*)&(tmp), 0);
		_pAdmCtx_->i16DeltaRxMs = __ntohs(tmp);
		// FIXME
		//_pAdmCtx_->i16DeltaRxMs &= (_pAdmCtx_->i16DeltaRxMs < 0)?(0xFFC1):(0x003F);
#else
		_pAdmCtx_->i16DeltaRxMs = 0;
#endif
		_pAdmCtx_->u8ByPassCmd = (_pAdmCtx_->u8ExchRxLength)?(0):(1);

		sSesDispCtx.u32DataDurationMs = _wizeapi_get_frm_duration_(u8Size, 1);
		sSesDispCtx.u32CmdDurationMs = _wizeapi_get_frm_duration_(u8RxLenMax, 0);
		sSesDispCtx.u32CmdDurationMs += _pAdmCtx_->u8ExchRxDelay*1000;
		sSesDispCtx.u32RspDurationMs = _wizeapi_get_frm_duration_(u8TxLenMax, 1);
		sSesDispCtx.u32RspDurationMs += _pAdmCtx_->u8ExchRespDelay*1000;

		pMsg->Option_b.App = 1;
	}
	else if (u8Type == APP_DOWNLOAD)
	{
		admin_cmd_anndownload_t *pAnnReq = (admin_cmd_anndownload_t*)(_pAdmCtx_->sCmdMsg.pData);
		admin_rsp_t *pAnnRsp = (admin_rsp_t*)(_pAdmCtx_->sRspMsg.pData);

		// check that previous anndownload is valid
		int32_t isNotValid;
		isNotValid = pAnnReq->L7CommandId - pAnnRsp->L7ResponseId;
		isNotValid += pAnnReq->L7CommandId - ADM_ANNDOWNLOAD;
		isNotValid += pAnnRsp->L7ErrorCode;

		if ( isNotValid )
		{
			return WIZE_API_INVALID_PARAM;
		}

#ifdef HAS_WIZE_CORE_EXTEND_PARAMETER
		int16_t tmp;
		Param_Access(DWN_RECEPTION_OFFSET, (uint8_t*)&(tmp), 0);
		_pDwnCtx_->i16DeltaRxMs = __ntohs(tmp);
		// FIXME
		//_pDwnCtx_->i16DeltaRxMs &= (_pDwnCtx_->i16DeltaRxMs < 0)?(0xFC01):(0x03FF);
#else
		_pDwnCtx_->i16DeltaRxMs = 0;
#endif
		// FIXME : not very clean
		sNetCtx.sProtoCtx.sProtoConfig.DwnId[0] = pAnnReq->L7DwnId[0];
		sNetCtx.sProtoCtx.sProtoConfig.DwnId[1] = pAnnReq->L7DwnId[1];
		sNetCtx.sProtoCtx.sProtoConfig.DwnId[2] = pAnnReq->L7DwnId[2];
		/*
		uint32_t args = pAnnReq->L7DwnId[0] << 16 | pAnnReq->L7DwnId[1] << 8 | pAnnReq->L7DwnId[2];
		NetMgr_Open(NULL);
		if ( NetMgr_Ioctl(NETDEV_CTL_SET_DWNID, args) != NET_STATUS_OK )
		{
			return WIZE_API_FAILED;
		}
		NetMgr_Close();
		*/

		// Set the klog
		Crypto_WriteKey( pAnnReq->L7Klog, KEY_LOG_ID);
		_pDwnCtx_->u8ChannelId    = (pAnnReq->L7ChannelId -100)/10;
		_pDwnCtx_->u16BlocksCount = __ntohs( *(uint16_t*)(pAnnReq->L7BlocksCount) );
		_pDwnCtx_->u8ModulationId = pAnnReq->L7ModulationId;
		_pDwnCtx_->u8DayRepeat    = pAnnReq->L7DayRepeat & 0x0F;
		_pDwnCtx_->u8DeltaSec     = pAnnReq->L7DeltaSec;
		_pDwnCtx_->u32DaysProg    = __ntohl( *(uint32_t*)(pAnnReq->L7DaysProg) ) + EPOCH_UNIX_TO_OURS;

		_pDwnCtx_->u16InitDelayMinMs = _pAdmCtx_->u8ExchRespDelay*1000 +1;
		// FIXME :
		//_pDwnCtx_->u8RxLength = 1;
		//_pDwnCtx_->i16DeltaRxMs = 0;

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
