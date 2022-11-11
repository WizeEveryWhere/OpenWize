/**
  * @file wize_app.h
  * @brief // TODO This file ...
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
  * @par 1.0.0 : 2022/11/05 [TODO: your name]
  * Initial version
  *
  */
#ifndef WIZE_APP_H_
#define WIZE_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "net_api.h"
#include "app_layer.h"

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
	WIZEAPP_INFO_DAY_PASSED = 0x0400,
	WIZEAPP_INFO_DAYLI_CHG  = 0x0800,
	WIZEAPP_INFO_FULL_POWER = 0x1000,
	WIZEAPP_INFO_PERIO_INST = 0x2000,
} wizeapp_info_e;

void WizeApp_Init(net_msg_t *pCmdMsg, net_msg_t *pRspMsg, net_msg_t *pPongMsg, net_msg_t *pBlkMsg);

uint8_t WizeApp_OnDwnBlkRecv(uint16_t u16Id, const uint8_t *pData);
uint8_t WizeApp_OnDwnComplete(void);
uint8_t WizeApp_AnnCheckFwInfo(admin_cmd_anndownload_t *pAnn);
void WizeApp_AnnReady(uint8_t eErrCode, uint8_t u8ErrorParam);

uint32_t WizeApp_Common(uint32_t ulEvent);

#ifdef __cplusplus
}
#endif

#endif /* WIZE_APP_H_ */
