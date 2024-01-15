/**
  * @file wize_app.h
  * @brief This file implement some convenient functions to deal with the Wize
  *        application layer.
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
  * @par 1.0.0 : 2022/11/05 [GBI]
  * Initial version
  *
  */
#ifndef WIZE_APP_H_
#define WIZE_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "wize_api.h"
#include "inst_internal.h"
#include "adm_internal.h"

/******************************************************************************/
extern uint16_t gu16PeriodInstCnt;
extern uint16_t gu16FullPowerCnt;

extern struct inst_mgr_ctx_s sInstCtx;
extern struct adm_mgr_ctx_s sAdmCtx;
extern struct dwn_mgr_ctx_s sDwnCtx;
extern struct time_upd_s sTimeUpdCtx;
extern struct ping_reply_ctx_s sPingReply;
extern struct adm_config_s sAdmConfig;

/******************************************************************************/
/*!
 * @brief This enumeration define
 */
typedef enum
{
	WIZEAPP_INFO_NONE       = 0x0000,
	// ----
	WIZEAPP_INFO_CMD_READ   = ADM_READ_PARAM,
	WIZEAPP_INFO_CMD_WRITE  = ADM_WRITE_PARAM,
	WIZEAPP_INFO_CMD_KEY    = ADM_WRITE_KEY,
	WIZEAPP_INFO_CMD_EXEC   = ADM_EXECINSTPING,
	WIZEAPP_INFO_CMD_ANN    = ADM_ANNDOWNLOAD,

	WIZEAPP_INFO_FINE_ADJ   = 0x0100,
	WIZEAPP_INFO_COARSE_ADJ = 0x0200,

	WIZEAPP_INFO_FULL_POWER = 0x1000,
	WIZEAPP_INFO_PERIO_INST = 0x2000,
	WIZEAPP_INFO_DAYLI_CHG  = 0x4000,

	WIZEAPP_INFO_DAY_PASSED = 0x8000,
	// ---
	WIZEAPP_INFO_CMD_MSK    = 0x00FF,
	WIZEAPP_INFO_CLOCK_MSK  = 0x0300,
	WIZEAPP_INFO_OTHER_MSK  = 0x7000,
	WIZEAPP_INFO_RSP_MSK    = 0x80000000,
} wizeapp_info_e;

typedef enum
{
	WIZEAPP_ADM_CMD_PEND  = 0x01,
	WIZEAPP_ADM_RSP_PEND  = 0x02,
} wizeapp_adm_pending_e;

/******************************************************************************/
// ---
// weak functions
uint8_t WizeApp_GetAdmReq(uint8_t *pData, uint8_t *pOther);
uint8_t WizeApp_GetInstReq(uint8_t *pData, uint8_t *pOther);
uint8_t WizeApp_GetFwInfo(admin_ann_fw_info_t *pFwAnnInfo, uint8_t *pOther);
int32_t WizeApp_GetFwInfoType(void);
uint8_t WizeApp_AnnCheckFwInfo(admin_cmd_anndownload_t *pAnn);
uint8_t WizeApp_OnDwnBlkRecv(uint16_t u16Id, const uint8_t *pData);

// ---
void WizeApp_Init(void);

//wize_api_ret_e WizeApp_Install(void);
//wize_api_ret_e WizeApp_Send(uint8_t *pData, uint8_t u8Size);
//wize_api_ret_e WizeApp_Alarm(uint8_t *pData, uint8_t u8Size);

wize_api_ret_e WizeApp_DownOpen(void);
void WizeApp_DownCancel(void);

void WizeApp_AnnReady(uint8_t eErrCode, uint8_t u8ErrorParam);
uint32_t WizeApp_Common(uint32_t ulEvent);
//uint8_t WizeApp_GetAdmCmd(uint8_t *pData, uint8_t *rssi);
uint32_t WizeApp_Time(void);

#ifdef __cplusplus
}
#endif

#endif /* WIZE_APP_H_ */
