/**
  * @file: wize_api.c
  * @brief: This file expose API to the external.
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
 * @ingroup WizeCore
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

#include "time_mgr.h"
#include "net_api_private.h"
#include "net_mgr.h"
#include "ses_dispatcher.h"

static wize_net_t sNetCtx;
static struct ses_disp_ctx_s sSesDispCtx;
static struct time_upd_s sTimeUpdCtx;
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
#define SES_MGR_INST_REQ_TMO_MSK 0xFFFFFFFFU
#define SES_MGR_INST_FLG_TMO_MSK 0xFFFFFFFFU
#define SES_MGR_INST_FLG_ALL_MSK 0x0000FFFFU | (SES_INST << SES_MGR_FLG_POS)

/*!
 * @brief This function start a INST (PING/PONG) session
 *
 *
 * @retval return wize_api_ret_e::WIZE_API_SUCCESS (0) if everything is fine
 *         return wize_api_ret_e::WIZE_API_FAILED (1) if INST session failed
  *         return wize_api_ret_e::WIZE_API_ACCESS_TIMEOUT (3) if access is refused
  */
wize_api_ret_e WizeApi_ExecPing(void)
{
	struct ses_ctx_s *pCtx = &(sSesDispCtx.sSesCtx[SES_INST]);
	uint32_t u32Ret;

	if ( xSemaphoreTake( pCtx->hMutex, SES_MGR_INST_REQ_TMO_MSK ) )
	{
		((struct inst_mgr_ctx_s*)(pCtx->pPrivate))->u8ParamUpdate = 0;
		// Request for INSTALL session
		xTaskNotify(sSesDispCtx.hTask, SES_MGR_INST_EVT_OPEN, eSetValueWithOverwrite);
		u32Ret = xEventGroupWaitBits(sSesDispCtx.hEvents, SES_MGR_INST_FLG_ALL_MSK, pdTRUE, pdFALSE, SES_MGR_INST_FLG_TMO_MSK);

		xSemaphoreGive(pCtx->hMutex);
		if (u32Ret == SES_MGR_INST_FLG_FAILED)
		{
			return WIZE_API_FAILED;
		}
		return WIZE_API_SUCCESS;
	}
	else
	{
		return WIZE_API_ACCESS_TIMEOUT;
	}
}


/******************************************************************************/
#define SES_MGR_ADM_REQ_TIMEOUT_MSK 0xFFFFFFFFU
#define SES_MGR_ADM_FLG_TIMEOUT_MSK 0xFFFFFFFFU
#define SES_MGR_ADM_FLG_ALL_MSK 0x0000FFFFU | (SES_ADM << SES_MGR_FLG_POS)

/*!
 * @brief This function get the last received ADM command message
 *
 * @param [out] pMsg Pointer on message buffer
 *
 * @retval return wize_api_ret_e::WIZE_API_SUCCESS (0) if everything is fine
 *         return wize_api_ret_e::WIZE_API_ACCESS_TIMEOUT (3) if access is refused
 *         return wize_api_ret_e::WIZE_API_INVALID_PARAM (4) if given parameter(s) is/are invalid
 */
wize_api_ret_e WizeApi_GetAdmCmd(net_msg_t *pMsg)
{
	struct ses_ctx_s *pCtx = &(sSesDispCtx.sSesCtx[SES_ADM]);
	struct adm_mgr_ctx_s *pPrvCtx;
	uint8_t *pKeep = pMsg->pData;
	if(pMsg && pMsg->pData)
	{
		if ( xSemaphoreTake( pCtx->hMutex, SES_MGR_ADM_REQ_TIMEOUT_MSK ) )
		{
			pPrvCtx = (struct adm_mgr_ctx_s *)pCtx->pPrivate;
			memcpy(pMsg, &(pPrvCtx->sCmdMsg), sizeof(net_msg_t));
			pMsg->pData = pKeep;
			if (pPrvCtx->sCmdMsg.u8Size <= RECV_BUFFER_SZ)
			{
				memcpy(pMsg->pData, pPrvCtx->sCmdMsg.pData, pPrvCtx->sCmdMsg.u8Size);
			}
			xSemaphoreGive(pCtx->hMutex);
			return WIZE_API_SUCCESS;
		}
		else
		{
			return WIZE_API_ACCESS_TIMEOUT;
		}
	}
	return WIZE_API_INVALID_PARAM;
}

/*!
 * @brief This function get the last sent ADM response message
 *
 * @param [out] pMsg Pointer on message buffer
 *
 * @retval return wize_api_ret_e::WIZE_API_SUCCESS (0) if everything is fine
 *         return wize_api_ret_e::WIZE_API_ACCESS_TIMEOUT (3) if access is refused
 *         return wize_api_ret_e::WIZE_API_INVALID_PARAM (4) if given parameter(s) is/are invalid
 */
wize_api_ret_e WizeApi_GetAdmRsp(net_msg_t *pMsg)
{
	struct ses_ctx_s *pCtx = &(sSesDispCtx.sSesCtx[SES_ADM]);;
	struct adm_mgr_ctx_s *pPrvCtx;
	uint8_t *pKeep = pMsg->pData;
	if(pMsg && pMsg->pData)
	{
		if ( xSemaphoreTake(  pCtx->hMutex, SES_MGR_ADM_REQ_TIMEOUT_MSK ) )
		{
			pPrvCtx = (struct adm_mgr_ctx_s *)pCtx->pPrivate;
			memcpy(pMsg, &( pPrvCtx->sRspMsg), sizeof(net_msg_t));
			pMsg->pData = pKeep;
			if ( pPrvCtx->sCmdMsg.u8Size <= SEND_BUFFER_SZ)
			{
				memcpy(pMsg->pData,  pPrvCtx->sRspMsg.pData,  pPrvCtx->sRspMsg.u8Size);
			}
			xSemaphoreGive(pCtx->hMutex);
			return WIZE_API_SUCCESS;
		}
		else
		{
			return WIZE_API_ACCESS_TIMEOUT;
		}
	}
	return WIZE_API_INVALID_PARAM;
}

/*!
 * @brief This function send a DATA message
 *
 * @param [in] pData  Pointer on raw data to send
 * @param [in] u8Size Number of byte to send
 * @param [in] u8Type Type of frame DATA or DATA_PRIO
 *
 * @retval return wize_api_ret_e::WIZE_API_SUCCESS (0) if everything is fine
 *         return wize_api_ret_e::WIZE_API_FAILED (1) if ADN session failed
 *         return wize_api_ret_e::WIZE_API_ADM_SUCCESS (2) if ADM CMD has been received
 *         return wize_api_ret_e::WIZE_API_ACCESS_TIMEOUT (3) if access is refused
 *         return wize_api_ret_e::WIZE_API_INVALID_PARAM (4) if given parameter(s) is/are invalid
 */
wize_api_ret_e WizeApi_SendEx(uint8_t *pData, uint8_t u8Size, uint8_t u8Type)
{
	//struct dat_mgr_ctx_s *pCtx = sSesDispCtx.pDatMgrCtx;
	struct ses_ctx_s *pCtx = &(sSesDispCtx.sSesCtx[SES_ADM]);
	uint32_t u32Ret;
	uint8_t u8L7MaxLen;
	if ( pData )
	{
		Param_Access(L7TRANSMIT_LENGTH_MAX, (uint8_t*)&u8L7MaxLen, 0);
		if ( u8Size > u8L7MaxLen)
		{
			return WIZE_API_INVALID_PARAM;
		}
		if ( (u8Type != APP_DATA) && (u8Type != APP_DATA_PRIO))
		{
			return WIZE_API_INVALID_PARAM;
		}

		// Ensure that only one request at the time
		if ( xSemaphoreTake( pCtx->hMutex, SES_MGR_ADM_REQ_TIMEOUT_MSK ) )
		{
			((struct adm_mgr_ctx_s*)(pCtx->pPrivate))->u8ParamUpdate = 0;
			// Request for DATA, ADMIN session
			net_msg_t *pMsg = &((struct adm_mgr_ctx_s*)(pCtx->pPrivate))->sDataMsg;
			pMsg->pData = pData;
			pMsg->Option_b.App = 1;
			pMsg->u8Size = u8Size;
			pMsg->u8Type = u8Type;
			pMsg->u16Id++;
			xTaskNotify(sSesDispCtx.hTask, SES_MGR_ADM_EVT_OPEN, eSetValueWithOverwrite);
			u32Ret = xEventGroupWaitBits(sSesDispCtx.hEvents, SES_MGR_ADM_FLG_ALL_MSK, pdTRUE, pdFALSE, SES_MGR_ADM_FLG_TIMEOUT_MSK);
			if (u32Ret == SES_MGR_ADM_FLG_REQUEST)
			{
				if (((struct adm_mgr_ctx_s*)(pCtx->pPrivate))->sCmdMsg.pData[0] == ADM_WRITE_PARAM)
				{
					u32Ret = WIZE_API_ADM_SUCCESS;
				}
				else
				{
					u32Ret = WIZE_API_SUCCESS;
				}
			}
			else if (u32Ret == SES_MGR_ADM_FLG_SUCCES)
			{
				u32Ret = WIZE_API_SUCCESS;
			}
			else {
				u32Ret = WIZE_API_FAILED;
			}
			xSemaphoreGive(pCtx->hMutex);
			return u32Ret;
		}
		else
		{
			return WIZE_API_ACCESS_TIMEOUT;
		}
	}
	return WIZE_API_INVALID_PARAM;
}

/*!
 * @brief This function send a DATA message
 *
 * @param [in] pData  Pointer on raw data to send
 * @param [in] u8Size Number of byte to send
 * @param [in] u8Type Type of frame DATA or DATA_PRIO
 *
 * @retval return wize_api_ret_e::WIZE_API_SUCCESS (0) if everything is fine
 *         return wize_api_ret_e::WIZE_API_FAILED (1) if ADN session failed
 *         return wize_api_ret_e::WIZE_API_ADM_SUCCESS (2) if ADM CMD has been received
 *         return wize_api_ret_e::WIZE_API_ACCESS_TIMEOUT (3) if access is refused
 *         return wize_api_ret_e::WIZE_API_INVALID_PARAM (4) if given parameter(s) is/are invalid
 */
wize_api_ret_e WizeApi_Send(uint8_t *pData, uint8_t u8Size, uint8_t u8Type)
{
	//struct dat_mgr_ctx_s *pCtx = sSesDispCtx.pDatMgrCtx;
	struct ses_ctx_s *pCtx = &(sSesDispCtx.sSesCtx[SES_ADM]);
	uint32_t u32Ret;
	uint8_t u8L7MaxLen;
	if ( pData )
	{
		Param_Access(L7TRANSMIT_LENGTH_MAX, (uint8_t*)&u8L7MaxLen, 0);
		if ( u8Size > u8L7MaxLen)
		{
			return WIZE_API_INVALID_PARAM;
		}
		if ( (u8Type != APP_DATA) && (u8Type != APP_DATA_PRIO))
		{
			return WIZE_API_INVALID_PARAM;
		}
		if ( xSemaphoreTake( pCtx->hMutex, SES_MGR_ADM_REQ_TIMEOUT_MSK ) )
		{
			((struct adm_mgr_ctx_s*)(pCtx->pPrivate))->u8ParamUpdate = 0;
			// Request for DATA, ADMIN session
			net_msg_t *pMsg = &((struct adm_mgr_ctx_s*)(pCtx->pPrivate))->sDataMsg;
			pMsg->pData = pData;
			pMsg->u8Size = u8Size;
			pMsg->u8Type = u8Type;
			pMsg->u16Id++;
			xTaskNotify(sSesDispCtx.hTask, SES_MGR_ADM_EVT_OPEN, eSetValueWithOverwrite);
			u32Ret = xEventGroupWaitBits(sSesDispCtx.hEvents, SES_MGR_ADM_FLG_ALL_MSK, pdTRUE, pdFALSE, SES_MGR_ADM_FLG_TIMEOUT_MSK);
			if (u32Ret & SES_MGR_FLG_REQUEST)
			{
				if (((struct adm_mgr_ctx_s*)(pCtx->pPrivate))->sCmdMsg.pData[0] == ADM_WRITE_PARAM)
				{
					u32Ret = WIZE_API_ADM_SUCCESS;
				}
				else
				{
					u32Ret = WIZE_API_SUCCESS;
				}
			}
			else if (u32Ret & SES_MGR_FLG_SUCCESS)
			{
				u32Ret = WIZE_API_SUCCESS;
			}
			else {
				u32Ret = WIZE_API_FAILED;
			}
			xSemaphoreGive(pCtx->hMutex);
			return u32Ret;
		}
		else
		{
			return WIZE_API_ACCESS_TIMEOUT;
		}
	}
	return WIZE_API_INVALID_PARAM;
}

/******************************************************************************/

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

/*!
 * @brief This function setup the wize stack
 **
 * @retval None
  */

void WizeApi_Setup(phydev_t *pPhyDev)
{
	// Setup the Time Manager
	TimeMgr_Setup(&sTimeUpdCtx);
	// Setup Network Manager
	NetMgr_Setup(pPhyDev, &sNetCtx);
	// Setup the session dispacher
	SesDisp_Setup(&sSesDispCtx);
}

/*!
 * @brief This function initialize the wize stack
 *
 * @param[in] pPhyDev Pointer on the phy device
 *
 * @retval None
  */

void WizeApi_Init(void)
{
}

void WizeApi_Enable(uint8_t bFlag)
{
	//SesDisp_Init(&sSesDispCtx, bFlag);
}

#ifdef __cplusplus
}
#endif

/*! @} */
