/**
  * @file app_template.h
  * @brief // TODO This file ...
  * 
  * @details
  *
  * @copyright 2023, GRDF, Inc.  All rights reserved.
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
  * @par 1.0.0 : 2023/03/01 [GBI]
  * Initial version
  *
  */
#ifndef _APP_TEMPLATE_H_
#define _APP_TEMPLATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "app_layer.h"

/******************************************************************************/
typedef enum
{
	UPD_PEND_NONE      = 0x00,
	UPD_PEND_INTERNAL  = 0x01,
	UPD_PEND_EXTERNAL  = 0x02,
	UPD_PEND_LOCAL     = 0x03,
	// ---
} pend_update_e;

typedef enum
{
	UPD_STATUS_UNK          = 0x00,
	UPD_STATUS_SES_FAILED   = 0x01,
	UPD_STATUS_STORE_FAILED = 0x02,
	// ---
	UPD_STATUS_INPROGRESS   = 0x04,
	// ---
	UPD_STATUS_INCOMPLETE   = 0x05,
	UPD_STATUS_CORRUPTED    = 0x06,
	UPD_STATUS_VALID        = 0x07,
	// ---
} update_status_e;

struct update_ctx_s
{
	pend_update_e   ePendUpdate;
	update_status_e eUpdateStatus;
	uint8_t         eErrCode;
	uint8_t         eErrParam;
} ;

/******************************************************************************/

void App_Process(uint32_t u32Evt);
void Time_Process(uint32_t u32Evt);
void WizeApp_AnnCheckFwInfo(admin_cmd_anndownload_t *pAnn);
uint8_t WizeApp_OnDwnBlkRecv(uint16_t u16Id, const uint8_t *pData);
int32_t AdmInt_AnnIsLocalUpdate(void);

/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* _APP_TEMPLATE_H_ */
