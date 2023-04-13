/**
  * @file adm_internal.c
  * @brief This file implement the functions to treat the administration L7
  * content.
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
  * @par 1.0.0 : 2020/10/11[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup wize_admin_layer
 * @{
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "adm_internal.h"

#include "parameters.h"
#include "parameters_lan_ids.h"

#include "crypto.h"
#include "phy_layer.h"

#include <time.h>
#include <string.h>
#include <machine/endian.h>

/******************************************************************************/
/*!
 * @cond INTERNAL
 * @{
 */
__attribute__((weak)) struct adm_config_s sAdmConfig;

#define LAST_WRITE_PARAM_NB_MAX 128
#define LAST_READ_PARAM_NB_MAX 224

static uint8_t aLastWriteParamIds[LAST_WRITE_PARAM_NB_MAX];
static uint8_t aLastReadParamIds[LAST_READ_PARAM_NB_MAX];

const struct adm_config_s sAdmConfigDefault =
{
	.state = 1,
	.ClkFreqAutoAdj = 0,
	.ClkFreqAutoAdjRssi = 0,
	.filterDisAnn = 0,
	.filterDisParam = 0,
	.filterDisKey = 0,

	// .MField = pProtoCtx->aDeviceManufID
	.u32DwnDayProgWinMin = DWN_DAY_PROG_WIN_MIN,
	.u32DwnDayProgWinMax = DWN_DAY_PROG_WIN_MAX,
	.u32MntWinDuration   = MNT_WIN_DURATION,
	.u32DwnDeltaSecMin   = DWN_DELTA_SEC_MIN_WM2400,
	.u32DwnBlkDurMod     = DWN_BLK_DURATION_WM2400,
	.u32DwnBlkNbMax      = DWN_NB_BLK_MAX,

	.pLastWriteParamIds  = aLastWriteParamIds,
	.pLastReadParamIds   = aLastReadParamIds,
};

/*!
 * @}
 * @endcond
 */
/******************************************************************************/
/*!
  * @brief This function initialize the default Admin. config. structure.
  * is not executed here.
  */
void AdmInt_SetupDefaultConfig(void)
{
	memcpy(&sAdmConfig, &sAdmConfigDefault, sizeof(struct adm_config_s));
}

/*!
  * @brief This function initialize the default Admin. config. structure.
  * is not executed here.
  */
void AdmInt_SetupConfig(void)
{
	AdmInt_SetupDefaultConfig();
#ifdef HAS_WIZE_CORE_EXTEND_PARAMETER
	uint8_t tmp[4];
	/* Set the min RSSI to Auto-Adjust Clock and Frequency offset
	 * step : 0.5 dBm
	 * 255 :  -20.0 dBm
	 *   0 : -147.5 dBm
	 * dBm     = (hex_val * 0.5) - 147.5
	 * hex_val = (dBm + 147.5)*2
	 *
	 * -85 dBm : 125
	*/
	Param_Access(AUTO_ADJ_CLK_FREQ, tmp, 0);
	sAdmConfig.ClkFreqAutoAdj     = (uint8_t)tmp[0];
	sAdmConfig.ClkFreqAutoAdjRssi = (uint8_t)tmp[1];

	Param_Access(ADM_ANN_DIS_FLT,   (uint8_t*)&(sAdmConfig.filterDisAnn), 0);
	Param_Access(ADM_PARAM_DIS_FLT, (uint8_t*)&(sAdmConfig.filterDisParam), 0);
	Param_Access(ADM_KEY_DIS_FLT,   (uint8_t*)&(sAdmConfig.filterDisKey), 0);

	Param_Access(DWN_DAY_PRG_WIN_MIN, tmp, 0);
	sAdmConfig.u32DwnDayProgWinMin = __ntohl(*(uint32_t*)tmp);
	Param_Access(DWN_DAY_PRG_WIN_MAX, tmp, 0);
	sAdmConfig.u32DwnDayProgWinMax = __ntohl(*(uint32_t*)tmp);
	Param_Access(MNT_WINDOW_DURATION, tmp, 0);
	sAdmConfig.u32MntWinDuration   = __ntohl(*(uint32_t*)tmp);
	Param_Access(DWN_DELTA_SEC_MIN, tmp, 0);
	sAdmConfig.u32DwnDeltaSecMin   = __ntohl(*(uint32_t*)tmp);
	Param_Access(DWN_BLK_DURATION_MOD, tmp, 0);
	sAdmConfig.u32DwnBlkDurMod     = __ntohl(*(uint32_t*)tmp);
	Param_Access(DWN_BLK_NB_MAX, tmp, 0);
	sAdmConfig.u32DwnBlkNbMax      = __ntohl(*(uint32_t*)tmp);
#endif
}

/******************************************************************************/
/*!
  * @brief This function treat the command and build the response. The command
  * is not executed here.
  *
  * @param [in,out] pReqMsg Pointer on request message
  * @param [in,out] pRspMsg Pointer on response message
  *
  * @retval RSP_NOT_READY (see @link admin_rsp_status_e::RSP_NOT_READY @endlink)
  * @retval RSP_READY (see @link admin_rsp_status_e::RSP_READY @endlink)
  * @retval RSP_ALREADY_DONE (see @link admin_rsp_status_e::RSP_ALREADY_DONE @endlink)
  */
uint8_t AdmInt_PreCmd(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	uint8_t ret = RSP_READY;
	// check if CMD Id has not previously been received
	if (pReqMsg->u16Id != pRspMsg->u16Id)
	{
		pRspMsg->u8Type = APP_ADMIN;
		pRspMsg->u16Id = pReqMsg->u16Id;

		// prepare the header
		((admin_rsp_t*)(pRspMsg->pData))->L7ResponseId = pReqMsg->pData[0];
		((admin_rsp_t*)(pRspMsg->pData))->L7ErrorCode  = ADM_NONE;
		((admin_rsp_t*)(pRspMsg->pData))->L7Rssi       = pReqMsg->u8Rssi;
		Param_Access(VERS_FW_TRX, (uint8_t*)(((admin_rsp_t*)(pRspMsg->pData))->L7SwVersion), 0);
		pRspMsg->u8Size = sizeof(admin_rsp_t);

		switch (pReqMsg->pData[0]) //L7CommandId
		{
			case ADM_READ_PARAM :
				AdmInt_ReadParam(pReqMsg, pRspMsg);
				break;
			case ADM_WRITE_PARAM :
				AdmInt_WriteParam(pReqMsg, pRspMsg);
				break;
			case ADM_WRITE_KEY :
				if(!sAdmConfig.state)
				{
					AdmInt_SetupDefaultConfig();
				}
				AdmInt_WriteKey(pReqMsg, pRspMsg);
				break;
			case ADM_ANNDOWNLOAD :
				if(!sAdmConfig.state)
				{
					AdmInt_SetupDefaultConfig();
				}
				AdmInt_Anndownload(pReqMsg, pRspMsg);
				// Check if error
				if (((admin_rsp_t*)(pRspMsg->pData))->L7ErrorCode == ADM_NONE)
				{
					ret = RSP_NOT_READY;
				}
				break;
			case ADM_EXECINSTPING :
				// response not yet available
				//pRspMsg->pData[0] = ~(pReqMsg->pData[0]);
				ret = RSP_NOT_READY;
				break;
			default :
				AdmInt_Unknown(pReqMsg, pRspMsg);
				break;
		}
	}
	else // CMD was previously processed
	{
		// if RSP is available, send it (previously build RSP)
		// else, build RSP => EXEC_PING case only
		if (pReqMsg->pData[0] == ADM_EXECINSTPING)
		{
			// build rsp
			AdmInt_Execping(pReqMsg, pRspMsg);
		}
		ret = RSP_ALREADY_DONE;
	}
	return ret;
}

/******************************************************************************/

/*!
  * @brief This function treat the L7 unknown command
  *
  * @param [in,out] pReqMsg Pointer on request message
  * @param [in,out] pRspMsg Pointer on response message
  *
  * @return None
  */
__attribute__((weak))
void AdmInt_Unknown(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	((admin_rsp_cmderr_t*)(pRspMsg->pData))->L7ErrorCode = ADM_UNK_CMD;
	pRspMsg->u8Size = sizeof(admin_rsp_cmderr_t);
}

/*!
  * @brief This function treat the L7 read parameter
  *
  * @param [in,out] pReqMsg Pointer on request message
  * @param [in,out] pRspMsg Pointer on response message
  * @return None
  */
void AdmInt_ReadParam(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	uint8_t idx;
	uint8_t u8L7TransLenMax;

	uint8_t u8ParamId;
	uint8_t u8ParamSz;
	uint8_t u8NbParam;

	uint8_t *pIn = &(pReqMsg->pData[1]);
	uint8_t *pOut = pRspMsg->pData;
	uint8_t *pLastReadIds = sAdmConfig.pLastReadParamIds;

	// Warning : the response buffer size must equal the L7TransLenMax
	Param_Access(L7TRANSMIT_LENGTH_MAX, (uint8_t*)(&u8L7TransLenMax), 0);

	u8NbParam = pReqMsg->u8Size - 1; // -1 for L7CommandId

	// update the position with header
	pOut += sizeof(admin_rsp_t);
	for (idx = 0; idx < u8NbParam; idx++)
	{
		u8ParamId = pIn[idx];

		// Keep trace of the last written parameters ids
		*pLastReadIds = u8ParamId;
		pLastReadIds++;

		// check if parameter exist
		if (Param_IsValidId(u8ParamId))
		{
			// check that parameter size is not exceed the end of buffer (don(t forget the paramId)
			u8ParamSz = Param_GetSize(u8ParamId);
			if ( (pOut + u8ParamSz +1) > (pRspMsg->pData + u8L7TransLenMax -1) )
			{
				// parameter size doesn't fit into the remaining buffer size
				((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = READ_LENGTH_EXCEED;
				((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorParam = pOut + u8ParamSz +1 - pRspMsg->pData;
				pRspMsg->u8Size = sizeof(admin_rsp_err_t);
				break;
			}

			// set the parameter id into the response
			*pOut = u8ParamId;
			// update the position
			pOut += 1;
			pRspMsg->u8Size += 1;

			// fill the parameter value
			if ( !(Param_RemoteAccess(u8ParamId, pOut, 0)) )
			{
				// check the access
				if ( NA == Param_GetRemAccess(u8ParamId) )
				{
					// parameter doesn't exist
					((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = READ_UNK_PARAM;
				}
				else
				{
					// unauthorized access
					((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = READ_ACCES_DENIED;
				}
				((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorParam = u8ParamId;
				pRspMsg->u8Size = sizeof(admin_rsp_err_t);
				break;
			}
			else {
				pRspMsg->u8Size += u8ParamSz;
			}
			// update the position
			pOut += u8ParamSz;
		}
		else
		{
			// parameter doesn't exist
			((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = READ_UNK_PARAM;
			((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorParam = u8ParamId;
			pRspMsg->u8Size = sizeof(admin_rsp_err_t);
			break;
		}
	}
	sAdmConfig.u8LastReadParamNb = pLastReadIds - sAdmConfig.pLastReadParamIds;
}

/*!
  * @brief This function treat the L7 write parameter
  *
  * @param [in,out] pReqMsg Pointer on request message
  * @param [in,out] pRspMsg Pointer on response message
  * @return None
  */
void AdmInt_WriteParam(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	uint8_t u8ParamId;
	uint8_t u8ParamSz;
	param_access_e eAccess;

	uint8_t *pIn = &(pReqMsg->pData[1]);
	uint8_t *pOut = pRspMsg->pData;

	// update the position with header
	pOut += sizeof(admin_rsp_t);
	while (pIn < &(pReqMsg->pData[pReqMsg->u8Size]))
	{
		// get paramId
		u8ParamId = *pIn;
		if (Param_IsValidId(u8ParamId))
		{
			// update the position
			pIn++;
			if ( ! (Param_CheckConformity(u8ParamId, pIn)) )
			{
				// the parameter value doesn't conform
				((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = WRITE_ILLEGAL_VALUE;
				((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorParam = u8ParamId;
				pRspMsg->u8Size = sizeof(admin_rsp_err_t);
				break;
			}
			else
			{
				eAccess = Param_GetRemAccess(u8ParamId);
				if ( !(eAccess & WO) )
				{
					if (eAccess == NA)
					{
						// parameter doesn't exist
						((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = WRITE_UNK_PARAM;
					}
					else
					{
						// unauthorized access
						((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = WRITE_ACCES_DENIED;
					}
					((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorParam = u8ParamId;
					pRspMsg->u8Size = sizeof(admin_rsp_err_t);
					break;
				}
			}
			// get the parameter
			u8ParamSz = Param_GetSize(u8ParamId);
			// update the position
			pIn += u8ParamSz;
		}
		else
		{
			// parameter doesn't exist
			((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = WRITE_UNK_PARAM;
			((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorParam = u8ParamId;
			pRspMsg->u8Size = sizeof(admin_rsp_err_t);
			break;
		}
	}
}

/*!
  * @brief This function treat the L7 write key
  *
  * @param [in,out] pReqMsg Pointer on request message
  * @param [in,out] pRspMsg Pointer on response message
  * @return None
  */
void AdmInt_WriteKey(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	admin_cmd_writekey_t *pIn = (admin_cmd_writekey_t*)(pReqMsg->pData);

	do
	{
		if ( pReqMsg->u8Size != sizeof(admin_cmd_writekey_t) )
		{
			// bad frame size
			((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = KEY_INCORRECT_FRM_LEN;
			((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorParam = pReqMsg->u8Size;
			pRspMsg->u8Size = sizeof(admin_rsp_err_t);
			break;
		}

		if (!sAdmConfig.KeyChgBypassUse)
		{
			if (pReqMsg->u8KeyId != KEY_CHG_ID)
			{
				// key change is not used
				((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = KEY_KCHG_NOT_USED;
				((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorParam = pReqMsg->u8KeyId;
				pRspMsg->u8Size = sizeof(admin_rsp_err_t);
				break;
			}
		}

		if( !sAdmConfig.WriteKeyBypassId )
		{
			if ( (pIn->L7KeyId != 0x01) && (pIn->L7KeyId != 0x02) )
			{
				// other keys are forbidden
				((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = KEY_ILLEGAL_VALUE;
				((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorParam = 0;
				pRspMsg->u8Size = sizeof(admin_rsp_err_t);
			}
		}
		else
		{
			if ( (pIn->L7KeyId == 0) || (pIn->L7KeyId >= KEY_MAX_NB) )
			{
				((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = KEY_ILLEGAL_VALUE;
				((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorParam = 0;
				pRspMsg->u8Size = sizeof(admin_rsp_err_t);
			}
		}
	} while(0);
}

/*!
  * @brief This function treat the L7 exec_ping
  *
  * @param [in,out] pReqMsg Pointer on request message
  * @param [in,out] pRspMsg Pointer on response message
  * @return None
  */
void AdmInt_Execping(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	uint8_t *pOut = pRspMsg->pData;

	// Fill rsp
	Param_Access(PING_NBFOUND, &(((admin_rsp_execinstping_t*)pOut)->L7NbPong), 0);
	Param_Access(PING_REPLY1, &(((admin_rsp_execinstping_t*)pOut)->L7Pong[0][0]), 0);
	Param_Access(PING_REPLY2, &(((admin_rsp_execinstping_t*)pOut)->L7Pong[1][0]), 0);
	Param_Access(PING_REPLY3, &(((admin_rsp_execinstping_t*)pOut)->L7Pong[2][0]), 0);
	pRspMsg->u8Size = sizeof(admin_rsp_execinstping_t);
}

/*!
  * @brief This function treat the L7 ann_download
  *
  * @param [in,out] pReqMsg Pointer on request message
  * @param [in,out] pRspMsg Pointer on response message
  * @return None
  */
void AdmInt_Anndownload(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	/**********************************************************************/
	AdmInt_AnnCheckSession(pReqMsg, pRspMsg);

	// Check if error
	if (((admin_rsp_t*)(pRspMsg->pData))->L7ErrorCode != ADM_NONE)
	{
		// error
		pRspMsg->u8Size = sizeof(admin_rsp_err_t);
	}
	else
	{
		// no error
		pRspMsg->u8Size = sizeof(admin_rsp_t);
	}
}

/*!
  * @brief This function check and validate the download session parameters
  *
  * @param [in,out] pReqMsg Pointer on request message
  * @param [in,out] pRspMsg Pointer on response message
  * @return None
  */
void AdmInt_AnnCheckSession(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	admin_cmd_anndownload_t *pIn = (admin_cmd_anndownload_t*)(pReqMsg->pData);
	uint8_t *pOut = pRspMsg->pData;
	time_t currentEpoch;

	uint32_t u32DaysProg;
	uint32_t u32DwnDuration;

	uint16_t u16BlksCnt;

	uint8_t u8DeltaSecMin;
	uint8_t u8ChId;

	// prepare the header
	do
	{
		if (((admin_rsp_t*)pOut)->L7ErrorCode != ADM_NONE)
		{
			break;
		}

		if ( pReqMsg->u8Size != sizeof(admin_cmd_anndownload_t) )
		{
			// bad frame size
			((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_INCORRECT_FRM_LEN;
			((admin_rsp_err_t*)pOut)->L7ErrorParam = pReqMsg->u8Size;
			break;
		}

		/**********************************************************************/
		// Check local update in progress
		int32_t LocalUpdate = AdmInt_AnnIsLocalUpdate();
		if (LocalUpdate != -1)
		{
			// error : local update in progress
			((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_UPD_IN_PROGRESS;
			((admin_rsp_err_t*)pOut)->L7ErrorParam = (uint8_t)(LocalUpdate & 0xFF);
			break;
		}
		/**********************************************************************/
		// Check MField
		if (sAdmConfig.MField)
		{
			if ( memcmp(pIn->L7MField, (void*)(&sAdmConfig.MField), sizeof(pIn->L7MField)) != 0 )
			{
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_ILLEGAL_VALUE;
				((admin_rsp_err_t*)pOut)->L7ErrorParam = ANN_FIELD_ID_L7MField;
				break;
			}
		}
		/**********************************************************************/
		// Check Key used for this message
		if (!sAdmConfig.KeyChgBypassUse)
		{
			if (pReqMsg->u8KeyId != KEY_CHG_ID)
			{
				// error on Key used for this message
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_KCHG_NOT_USED;
				((admin_rsp_err_t*)pOut)->L7ErrorParam = pReqMsg->u8KeyId;
				pRspMsg->u8KeyId = KEY_CHG_ID;
				break;
			}
		}

		/**********************************************************************/
		// Check channel
		u8ChId = (pIn->L7ChannelId -100)/10;
		if (u8ChId > PHY_CH150 )
		{
			//error
			((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_ILLEGAL_VALUE;
			((admin_rsp_err_t*)pOut)->L7ErrorParam = ANN_FIELD_ID_L7ChannelId;
			break;
		}

		// Check modulation
		if (pIn->L7ModulationId == PHY_WM2400)
		{
			u32DwnDuration = sAdmConfig.u32DwnBlkDurMod;
			u8DeltaSecMin = sAdmConfig.u32DwnDeltaSecMin;

		}
		else if (pIn->L7ModulationId == PHY_WM4800)
		{
			u32DwnDuration = sAdmConfig.u32DwnBlkDurMod / 2;
			u8DeltaSecMin = sAdmConfig.u32DwnDeltaSecMin / 2;
		}
		else
		{
			//error
			((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_ILLEGAL_VALUE;
			((admin_rsp_err_t*)pOut)->L7ErrorParam = ANN_FIELD_ID_L7ModulationId;
			break;
		}

		/**********************************************************************/
		// Check Day repeat
		if ( !(pIn->L7DayRepeat & 0x0F))
		{
			((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_ILLEGAL_VALUE;
			((admin_rsp_err_t*)pOut)->L7ErrorParam = ANN_FIELD_ID_L7DayRepeat;
			break;
		}

		// Check delta second
		if (!sAdmConfig.AnnDeltaSec)
		{
			if ( pIn->L7DeltaSec < u8DeltaSecMin)
			{
				//error
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_ILLEGAL_VALUE;
				((admin_rsp_err_t*)pOut)->L7ErrorParam = ANN_FIELD_ID_L7DeltaSec;
				break;
			}
		}

		// Check Broadcast day
		u32DaysProg = __ntohl( *(uint32_t*)(pIn->L7DaysProg) );
		// 1) a) Broadcast day must be >= current_epoch - (10*24*3600)
		// 1) b) Broadcast day must be <= current_epoch + (60*24*3600)
		if(!sAdmConfig.AnnDaysProg)
		{

			time(&currentEpoch);
			currentEpoch -= EPOCH_UNIX_TO_OURS;
			// TODO :
			//currentEpoch = pReqMsg->u32Epoch - EPOCH_UNIX_TO_OURS;

			if ( ( u32DaysProg < ( currentEpoch - (sAdmConfig.u32DwnDayProgWinMin) ) )  ||
				 ( u32DaysProg > ( currentEpoch + (sAdmConfig.u32DwnDayProgWinMax) ) )
			   )
			{
				// error on L7DaysProg
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_INCORRECT_BCAST_START_DAY;
				((admin_rsp_err_t*)pOut)->L7ErrorParam = (int8_t)(currentEpoch - u32DaysProg);
				break;
			}
		}

		// Check diffusion window
		u16BlksCnt = __ntohs(*(uint16_t*)(pIn->L7BlocksCount));
		u32DwnDuration += pIn->L7DeltaSec*1000;
		u32DwnDuration = (u32DwnDuration/1000) * u16BlksCnt - pIn->L7DeltaSec;
		if(sAdmConfig.u32MntWinDuration)
		{
			// Broadcast day is in maintenance window
			if (u32DaysProg % 86400 > sAdmConfig.u32MntWinDuration)
			{
				// error
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_DIFF_TIME_OUT_OF_WINDOWS;
				((admin_rsp_err_t*)pOut)->L7ErrorParam = 0;
				break;
			}

			// Broadcast duration not exceed the maintenance window
			if ( (u32DaysProg % 86400 + u32DwnDuration) > sAdmConfig.u32MntWinDuration)
			{
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_DIFF_TIME_OUT_OF_WINDOWS;

				if(u32DwnDuration > sAdmConfig.u32MntWinDuration)
				{
					// Error the broadcast duration is out of maintenance windows
					((admin_rsp_err_t*)pOut)->L7ErrorParam = 0xFF;
				}
				else
				{
					// Suggest to start the broadcast at 00:00 UTC
					((admin_rsp_err_t*)pOut)->L7ErrorParam = 0;
				}
				break;
			}
		}
		else
		{
			if (u32DwnDuration > 86400)
			{
				// error
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_DIFF_TIME_OUT_OF_WINDOWS;
				((admin_rsp_err_t*)pOut)->L7ErrorParam = 0xFF;
				break;
			}
		}

	} while(0);
}

/*!
  * @brief This function check and validate the internal firmware parameters to be download
  *
  * @param [in] pAnn Pointer on ann_download request message
  * @return The error code
  */
__attribute__((weak))
uint8_t AdmInt_AnnCheckIntFW(admin_ann_fw_info_t *pFwInfo, uint8_t *u8ErrorParam)
{
	uint8_t aTemp[4];
	uint16_t u16Tmp;
	uint8_t eErrCode;

	eErrCode = ADM_NONE;
	*u8ErrorParam = 0;
	do
	{
		/**********************************************************************/
		// Check Blocks Count
		if(!sAdmConfig.AnnBlocksNB)
		{
			if (pFwInfo->u16BlkCnt > sAdmConfig.u32DwnBlkNbMax )
			{
				eErrCode = ANN_ILLEGAL_VALUE;
				*u8ErrorParam = ANN_FIELD_ID_L7BlocksCount;
				break;
			}
		}

		/**********************************************************************/
		// Check HW version matching
		if (!sAdmConfig.AnnHwId)
		{
			Param_Access(VERS_HW_TRX, aTemp, 0);
			u16Tmp = __ntohs( *(uint16_t*)(aTemp) );
			if ( pFwInfo->u16DcHwId != u16Tmp )
			{
				eErrCode = ANN_INCORRECT_HW_VER;
				*u8ErrorParam = (uint8_t)(u16Tmp);
				break;
			}
		}

		/**********************************************************************/
		// Check Initial SW version
		if(!sAdmConfig.AnnSwVerIni)
		{
			Param_Access(VERS_FW_TRX, aTemp, 0);
			u16Tmp = __ntohs( *(uint16_t*)(aTemp) );
			if ( pFwInfo->u16SwVerIni != u16Tmp )
			{
				eErrCode = ANN_INCORRECT_INI_SW_VER;
				*u8ErrorParam = (uint8_t)(u16Tmp);
				break;
			}
		}

		/**********************************************************************/
		// Check Target SW version
		if(!sAdmConfig.AnnSwVerTgt)
		{
			Param_Access(VERS_FW_TRX, aTemp, 0);
			u16Tmp = __ntohs( *(uint16_t*)(aTemp) );
			if ( pFwInfo->u16SwVerTgt == u16Tmp ) // target version already download
			{
				eErrCode = ANN_TGT_VER_DWL;
				*u8ErrorParam = (uint8_t)(u16Tmp);
				break;
			}
			else if (  pFwInfo->u16SwVerTgt < u16Tmp ) // target version is lower than current one
			{
				eErrCode = ANN_TGT_SW_VER;
				*u8ErrorParam = (uint8_t)(u16Tmp);
				break;
			}
		}

	} while(0);
	return eErrCode;
}

/*!
  * @brief This function check if a local update is currently in progress
  *
  * @return The FW version currently being in progress, -1 : otherwise
  */
__attribute__((weak))
int32_t AdmInt_AnnIsLocalUpdate(void)
{
	return -1;
}

/******************************************************************************/
/*!
  * @brief This function executed the pending CMD action if any

  * @param [in,out] pReqMsg Pointer on request message
  * @param [in,out] pRspMsg Pointer on response message
  *
  * @return the executed action id (CMD id)
  */
uint8_t AdmInt_PostCmd(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	uint8_t ret = 0;
	if (pRspMsg->pData[0] == pReqMsg->pData[0])
	{
		// Check if no error, then do action
		if (((admin_rsp_t*)(pRspMsg->pData))->L7ErrorCode == ADM_NONE)
		{
			switch (pRspMsg->pData[0]) //L7CommandId
			{
				case ADM_WRITE_PARAM :
					AdmInt_PostWriteParam(pReqMsg);
					break;
				case ADM_WRITE_KEY :
					AdmInt_PostWriteKey(pReqMsg);
					break;
				case ADM_ANNDOWNLOAD :
					AdmInt_PostAnndownload(pReqMsg);
					break;
				case ADM_READ_PARAM :
					AdmInt_PostReadParam(pReqMsg);
					break;
				case ADM_EXECINSTPING :
					AdmInt_PostExecping(pReqMsg);
					break;
				default :
					break;
			}
			ret = pRspMsg->pData[0];
		}
		/*
		// TODO : finish it
		// Auto adjust clk
		if (sAdmConfig.ClkAutoAdm)
		{
			// check that RSSi is best enough
			if (pReqMsg->u8Rssi >= sAdmConfig.ClkFreqAutoAdjRssi)
			{
				int32_t tmp = (int32_t)(pReqMsg->u32Epoch & 0xFFFF);
				if (tmp > pReqMsg->u16Tstamp)
				{
					tmp -= pReqMsg->u16Tstamp;
				}
				else
				{
					tmp = pReqMsg->u16Tstamp - temp ;
				}


				Param_Access(CLOCK_CURRENT_EPOC, (uint8_t*)&( tmp ), 1);


			}
		}
		*/
	}
	return ret;
}

/******************************************************************************/

/*!
  * @brief This function execute the Read Parameter action
  *
  * @param [in,out] pReqMsg Pointer on request message
  *
  * @return None
  */
__attribute__((weak))
void AdmInt_PostReadParam(net_msg_t *pReqMsg)
{
	(void)pReqMsg;
}

/*!
  * @brief This function execute the Write Parameter action
  *
  * @param [in,out] pReqMsg Pointer on request message
  *
  * @return None
  */
void AdmInt_PostWriteParam(net_msg_t *pReqMsg)
{
	uint8_t u8ParamId;
	// update the position with header
	uint8_t *pIn = &(pReqMsg->pData[1]);
	uint8_t *pLastWriteIds = sAdmConfig.pLastWriteParamIds;

	while (pIn < &(pReqMsg->pData[pReqMsg->u8Size]))
	{
		// get paramId
		u8ParamId = *pIn;
		// update the position
		pIn++;
		// fill the parameter value
		Param_RemoteAccess(u8ParamId, pIn, 1);
		// get the parameter and update the position
		pIn += Param_GetSize(u8ParamId);

		// Keep trace of the last written parameters ids
		*pLastWriteIds = u8ParamId;
		pLastWriteIds++;
	}
	sAdmConfig.u8LastWriteParamNb = pLastWriteIds - sAdmConfig.pLastWriteParamIds;
}

/*!
  * @brief This function execute the Write Key action
  *
  * @param [in,out] pReqMsg Pointer on request message
  *
  * @return None
  */
void AdmInt_PostWriteKey(net_msg_t *pReqMsg)
{
	uint8_t u8KeyId;

	u8KeyId = ((admin_cmd_writekey_t*)(pReqMsg->pData))->L7KeyId;
	if( !sAdmConfig.WriteKeyBypassId )
	{
		u8KeyId += KEY_ENC_MAX;
		if ( (u8KeyId == KEY_MOB_ID) || (u8KeyId == KEY_MAC_ID) )
		{
			Crypto_WriteKey( ((admin_cmd_writekey_t*)(pReqMsg->pData))->L7KeyVal, u8KeyId );
		}
	}
	else
	{
		if ( (u8KeyId > 0) && (u8KeyId < KEY_MAX_NB) )
		{
			Crypto_WriteKey( ((admin_cmd_writekey_t*)(pReqMsg->pData))->L7KeyVal, u8KeyId );
		}
	}
}

/*!
  * @brief This function execute the ExecPing action
  *
  * @param [in,out] pReqMsg Pointer on request message
  *
  * @return None
  */
__attribute__((weak))
void AdmInt_PostExecping(net_msg_t *pReqMsg)
{
	(void)pReqMsg;
}

/*!
  * @brief This function execute the AnnDonwload action
  *
  * @param [in,out] pReqMsg Pointer on request message
  *
  * @return None
  */
__attribute__((weak))
void AdmInt_PostAnndownload(net_msg_t *pReqMsg)
{
	(void)pReqMsg;
}

/******************************************************************************/

#ifdef __cplusplus
}
#endif

/*! @} */
