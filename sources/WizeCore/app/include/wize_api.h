/**
  * @file: wize_api.h
  * @brief This file define the Wize API functions
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
  * @par 1.0.0 : 2020/10/14 [GBI]
  * Initial version
  *
  */

/*!
 * @addtogroup wize_api
 * @{
 *
 */
#ifndef _WIZE_API_H_
#define _WIZE_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "app_layer.h"

#include "net_api.h"
#include "phy_itf.h"

#include "inst_mgr.h"
#include "adm_mgr.h"
#include "dwn_mgr.h"

#include "time_mgr.h"

#include "logger.h"

#include "wize_banner.h"

/******************************************************************************/
/*!
 * @brief This enum define the return code from Wize API
 */
typedef enum
{
	WIZE_API_SUCCESS,
	WIZE_API_FAILED,
	WIZE_API_ACCESS_TIMEOUT,
	WIZE_API_INVALID_PARAM,
} wize_api_ret_e;

/*
typedef enum
{
	WIZE_API_FLG_NONE    = 0x0,
	WIZE_API_FLG_SUCCESS = 0x1,
	WIZE_API_FLG_FAILED  = 0x2,
	WIZE_API_FLG_REQUEST = 0x3,

} wize_api_flg_e;

typedef enum {

	SES_LOG_NONE           = 0,
	SES_LOG_CMD_RECV_READ  = 1,
	SES_LOG_CMD_RECV_WRITE,
	SES_LOG_CMD_RECV_KEY,
	SES_LOG_CMD_RECV_EXEC,
	SES_LOG_CMD_RECV_ANN,

	SES_LOG_PONG_RECV ,
	SES_LOG_BLK_RECV,
	SES_LOG_DATA_SENT,
	SES_LOG_RSP_SENT,
	SES_LOG_PING_SENT,

	SES_LOG_FINE_ADJUST,
	SES_LOG_COARSE_ADJUST,
	SES_LOG_DAY_PASSED,
	SES_LOG_DAYLIGHT_CHG,
} ses_log_e;

static const char * const _ses_log_str_[] =
{
	[SES_LOG_NONE]           = "...",
	[SES_LOG_CMD_RECV_READ]  = "READ",
	[SES_LOG_CMD_RECV_WRITE] = "WRITE",
	[SES_LOG_CMD_RECV_KEY]   = "KEY",
	[SES_LOG_CMD_RECV_EXEC]  = "EXEC",
	[SES_LOG_CMD_RECV_ANN]   = "ANN",

	[SES_LOG_PONG_RECV]      = "PONG",
	[SES_LOG_BLK_RECV]       = "BLK",
	[SES_LOG_DATA_SENT]      = "DATA",
	[SES_LOG_RSP_SENT]       = "RSP",
	[SES_LOG_PING_SENT]      = "PING",

	[SES_LOG_FINE_ADJUST]    = "FINE ADJ",
	[SES_LOG_COARSE_ADJUST]  = "COARSE ADJ",
	[SES_LOG_DAY_PASSED]     = "DAY PASSED",
	[SES_LOG_DAYLIGHT_CHG]   = "DAYLIGHT CHG"
};
*/
/******************************************************************************/
wize_api_ret_e WizeApi_SetDeviceId(device_id_t *pDevId);
wize_api_ret_e WizeApi_GetDeviceId(device_id_t *pDevId);

wize_api_ret_e WizeApi_ExecPing(uint8_t *pData, uint8_t u8Size);
wize_api_ret_e WizeApi_Send(uint8_t *pData, uint8_t u8Size, uint8_t bPrio);
wize_api_ret_e WizeApi_Download(void);
void WizeApi_Notify(uint32_t evt);
void WizeApi_Enable(uint8_t bFlag);
uint32_t WizeApi_GetState(uint8_t eSesId);
void WizeApi_SesMgr_Setup(
	phydev_t *pPhyDev,
	struct inst_mgr_ctx_s *pInstCtx,
	struct adm_mgr_ctx_s *pAdmCtx,
	struct dwn_mgr_ctx_s *pDwnCtx
	);
void WizeApi_OnSesFlag(void *hSesCaller, uint32_t u32Flg);

void WizeApi_Cancel(uint8_t eSesId);
void WizeApi_ExecPing_Cancel(void);
void WizeApi_Send_Cancel(void);
void WizeApi_Download_Cancel(void);

/******************************************************************************/
void WizeApi_TimeMgr_Register(void *hTask);
void WizeApi_TimeMgr_Update(uint32_t wakup_cycles);
void WizeApi_TimeMgr_Setup(struct time_upd_s *pTimeUpdCtx);
void WizeApi_OnTimeFlag(uint32_t u32Flg);

/******************************************************************************/
void WizeApi_CtxClear(void);
void WizeApi_CtxRestore(void);
void WizeApi_CtxSave(void);

/******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* _WIZE_API_H_ */

/*! @} */
