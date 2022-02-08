/**
  * @file: wize_api.h
  * @brief: // TODO This file ...
  * 
  *****************************************************************************
  * @Copyright 2019, GRDF, Inc.  All rights reserved.
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
  *****************************************************************************
  *
  * Revision history
  * ----------------
  * 1.0.0 : 2020/10/14[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Sources
 * @{
 * @ingroup Wize
 * @{
 * @ingroup API
 * @{
 *
 */

#ifndef _WIZE_API_H_
#define _WIZE_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "wize_banner.h"
#include "net_api.h"
#include "phy_itf.h"
#include "app_layer.h"

#include "logger.h"
/******************************************************************************/
/******************************************************************************/

uint16_t WizeApi_Img_GetMaxNumBlk(void);
int8_t WizeApi_Img_Init(uint32_t u32ImgAdd, uint16_t u16NbExpectedBlk);
void WizeApi_Img_AddBlk(uint16_t u16_BlkId, uint8_t *p_Blk);
uint8_t WizeApi_Img_IsComplete(void);
uint8_t WizeApi_Img_GetSHA256(uint8_t *pImgHash, uint8_t u8ReqSize);

/******************************************************************************/
/*!
 * @brief This enum define the return code from Wize API
 */
typedef enum
{
	WIZE_API_SUCCESS,
	WIZE_API_FAILED,

	WIZE_API_ADM_SUCCESS,

	WIZE_API_ACCESS_TIMEOUT,
	WIZE_API_INVALID_PARAM,
} wize_api_ret_e;

wize_api_ret_e WizeApi_GetAdmCmd(net_msg_t *pAdmMsg);
wize_api_ret_e WizeApi_GetAdmRsp(net_msg_t *pAdmMsg);
wize_api_ret_e WizeApi_GetStats(net_stats_t *pStats);
wize_api_ret_e WizeApi_SetDeviceId(device_id_t *pDevId);
wize_api_ret_e WizeApi_GetDeviceId(device_id_t *pDevId);

wize_api_ret_e WizeApi_ExecPing(void);
wize_api_ret_e WizeApi_Send(uint8_t *pData, uint8_t u8Size, uint8_t u8Type);
wize_api_ret_e WizeApi_SendEx(uint8_t *pData, uint8_t u8Size, uint8_t u8Type);

void WizeApi_Setup(phydev_t *pPhyDev);
void WizeApi_Enable(uint8_t bFlag);

void WizeApi_CtxClear(void);
void WizeApi_CtxSave(void);
void WizeApi_CtxRestore(void);

/******************************************************************************/

/*! @} */
/*! @} */
/*! @} */

#ifdef __cplusplus
}
#endif
#endif /* _WIZE_API_H_ */
