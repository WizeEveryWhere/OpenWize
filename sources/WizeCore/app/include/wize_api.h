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

#include "wize_banner.h"
#include "net_api.h"
#include "phy_itf.h"
#include "app_layer.h"

#include "logger.h"

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


/*!
 * @brief This enumeration define
 */
typedef enum
{
	WIZE_API_FLG_NONE    = 0x0,
	WIZE_API_FLG_SUCCESS = 0x1,
	WIZE_API_FLG_FAILED  = 0x2,
	WIZE_API_FLG_REQUEST = 0x3,

} wize_api_flg_e;

/******************************************************************************/

wize_api_ret_e WizeApi_ExecPing(uint8_t *pData, uint8_t u8Size);
wize_api_ret_e WizeApi_Send(uint8_t *pData, uint8_t u8Size, uint8_t bPrio);
wize_api_ret_e WizeApi_Download(void);
void WizeApi_Notify(uint32_t evt);
void WizeApi_Setup(phydev_t *pPhyDev);

void WizeApi_Enable(uint8_t bFlag);
void WizeApi_CtxClear(void);
void WizeApi_CtxRestore(void);
void WizeApi_CtxSave(void);

/******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* _WIZE_API_H_ */

/*! @} */
