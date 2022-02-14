/**
  * @file: adm_internal.c
  * @brief: This file implement the functions to treat the administration L7
  * content.
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
  * 1.0.0 : 2020/10/11[GBI]
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

#include "adm_internal.h"

#include "parameters.h"
#include "parameters_lan_ids.h"

#include "crypto.h"
#include "phy_layer.h"

#include <time.h>
#include <string.h>

static void _adm_read_param_(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
static void _adm_write_param_(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
static void _adm_write_key_(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
static void _adm_anndownload_(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
static void _adm_execping_(net_msg_t *pReqMsg, net_msg_t *pRspMsg);

/*!
  * @brief          This function treat the L7 read parameter
  *
  * @param [in,out] *pReqMsg Pointer on request message
  * @param [in,out] *pRspMsg Pointer on response message
  * @return         None
  */
static void _adm_read_param_(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	uint8_t idx;
	uint8_t u8L7TransLenMax;

	uint8_t u8ParamId;
	uint8_t u8ParamSz;
	uint8_t u8NbParam;

	uint8_t *pIn = &(pReqMsg->pData[1]);
	uint8_t *pOut = pRspMsg->pData;

	// Warning : the response buffer size must equal the L7TransLenMax
	Param_Access(L7TRANSMIT_LENGTH_MAX, (uint8_t*)(&u8L7TransLenMax), 0);

	u8NbParam = pReqMsg->u8Size - 1; // -1 for L7CommandId
	// prepare the header
	((admin_rsp_t*)pOut)->L7ResponseId = ADM_READ_PARAM;
	((admin_rsp_t*)pOut)->L7ErrorCode  = ADM_NONE;
	((admin_rsp_t*)pOut)->L7Rssi       = pReqMsg->u8Rssi;
	Param_Access(VERS_FW_TRX, (uint8_t*)(((admin_rsp_t*)pOut)->L7SwVersion), 0);
	pRspMsg->u8Size = sizeof(admin_rsp_t);;

	// update the position with header
	pOut += sizeof(admin_rsp_t);
	for (idx = 0; idx < u8NbParam; idx++)
	{
		u8ParamId = pIn[idx];

		if (Param_IsValidId(u8ParamId))
		{
			// check that parameter size is not exceed the end of buffer (don(t forget the paramId)
			u8ParamSz = Param_GetSize(u8ParamId);
			if ( (pOut + u8ParamSz +1) > (pRspMsg->pData + u8L7TransLenMax) )
			{
				// parameter size dpesn't fit into the remaining buffer size
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
}

/*!
  * @brief          This function treat the L7 write parameter
  *
  * @param [in,out] *pReqMsg Pointer on request message
  * @param [in,out] *pRspMsg Pointer on response message
  * @return         None
  */
static void _adm_write_param_(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	uint8_t u8ParamId;
	uint8_t u8ParamSz;
	param_access_e eAccess;

	uint8_t *pIn = &(pReqMsg->pData[1]);
	uint8_t *pOut = pRspMsg->pData;

	// prepare the header
	((admin_rsp_t*)pOut)->L7ResponseId = ADM_WRITE_PARAM;
	((admin_rsp_t*)pOut)->L7ErrorCode  = ADM_NONE;
	((admin_rsp_t*)pOut)->L7Rssi       = pReqMsg->u8Rssi;
	Param_Access(VERS_FW_TRX, (uint8_t*)(((admin_rsp_t*)pOut)->L7SwVersion), 0);
	pRspMsg->u8Size = sizeof(admin_rsp_t);
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
  * @brief          This function treat the L7 write key
  *
  * @param [in,out] *pReqMsg Pointer on request message
  * @param [in,out] *pRspMsg Pointer on response message
  * @return         None
  */
static void _adm_write_key_(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	admin_cmd_writekey_t *pIn = (admin_cmd_writekey_t*)(pReqMsg->pData);
	uint8_t *pOut = pRspMsg->pData;

	// prepare the header
	((admin_rsp_t*)pOut)->L7ResponseId = ADM_WRITE_KEY;
	((admin_rsp_t*)pOut)->L7ErrorCode  = ADM_NONE;
	((admin_rsp_t*)pOut)->L7Rssi       = pReqMsg->u8Rssi;
	Param_Access(VERS_FW_TRX, (uint8_t*)(((admin_rsp_t*)pOut)->L7SwVersion), 0);
	pRspMsg->u8Size = sizeof(admin_rsp_t);

	if ( (pIn->L7KeyId != 0x01) && (pIn->L7KeyId != 0x02) )
	{
		// other keys are forbidden
		((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorCode = KEY_ILLEGAL_VALUE;
		((admin_rsp_err_t*)pRspMsg->pData)->L7ErrorParam = 0;
		pRspMsg->u8Size = sizeof(admin_rsp_err_t);
	}
}

/*!
  * @brief          This function treat the L7 exec_ping
  *
  * @param [in,out] *pReqMsg Pointer on request message
  * @param [in,out] *pRspMsg Pointer on response message
  * @return         None
  */
static void _adm_execping_(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	uint8_t *pOut = pRspMsg->pData;

	// prepare the header
	((admin_rsp_t*)pOut)->L7ResponseId = ADM_EXECINSTPING;
	((admin_rsp_t*)pOut)->L7ErrorCode  = ADM_NONE;
	((admin_rsp_t*)pOut)->L7Rssi       = pReqMsg->u8Rssi;
	Param_Access(VERS_FW_TRX, (uint8_t*)(((admin_rsp_t*)pOut)->L7SwVersion), 0);

	// Fill rsp
	Param_Access(PING_NBFOUND, &(((admin_rsp_execinstping_t*)pOut)->L7NbPong), 0);
	Param_Access(PING_REPLY1, &(((admin_rsp_execinstping_t*)pOut)->L7Pong[0][0]), 0);
	Param_Access(PING_REPLY2, &(((admin_rsp_execinstping_t*)pOut)->L7Pong[1][0]), 0);
	Param_Access(PING_REPLY3, &(((admin_rsp_execinstping_t*)pOut)->L7Pong[2][0]), 0);
	pRspMsg->u8Size = sizeof(admin_rsp_execinstping_t);
}

#define DWN_BLK_DURATION_WM2400 1000 // in ms
#define DWN_BLK_DURATION_WM4800 500  // in ms

#define BYPASS_DELTASEC_RESTRICTION

#ifndef BYPASS_DELTASEC_RESTRICTION
	#define DWN_DELTA_SEC_MIN_WM2400 10 // in s
	#define DWN_DELTA_SEC_MIN_WM4800 5  // in s
#else
	#define DWN_DELTA_SEC_MIN_WM2400 2 // in s
	#define DWN_DELTA_SEC_MIN_WM4800 2 // in s
#endif

#define DWN_DAY_PROG_WIN_MIN 864000 // (10*24*3600) in s
#define DWN_DAY_PROG_WIN_MAX 5184000 // (60*24*3600) in s

struct adm_config_s{
	union {
		uint8_t filterDisAnn;      /*!< ADM Reception Filter disable*/
		struct
		{
			uint8_t AnnDaysProg:1; /*!< AnnDown Day Prog. filter. 0: enable, 1: disable */
			uint8_t AnnDeltaSec:1; /*!< AnnDown Delta Second filter. 0: enable, 1: disable */
			uint8_t AnnHwId:1;     /*!< AnnDown HW ver.filter. 0: enable, 1: disable */
			uint8_t AnnSwVerIni:1; /*!< AnnDown SW Ver. Initial filter. 0: enable, 1: disable */
			uint8_t AnnSwVerTgt:1; /*!< AnnDown SW Ver. Target filter. 0: enable, 1: disable */
			uint8_t AnnBlocksNB:1; /*!< AnnDown Max. Block Number filter. 0: enable, 1: disable */
		} ;//filterDisAnn_b;
	};
	union {
		uint8_t filterDisParam;      /*!< ADM Reception Filter disable*/
		struct
		{
			uint8_t ParamBypassWO:1;
			uint8_t ParamBypassRO:1;
		} ; //filterDisAnn_b;
	};
	union {
		uint8_t filterDisKey;      /*!< ADM Reception Filter disable*/
		struct
		{
			uint8_t WriteKeyBypassId:1;
			uint8_t KeyChgBypassUse:1;
		} ; //filterDisKey_b;
	};
	union {
		uint8_t ClkFreqAutoAdj;     /*!< Clock and Frequency Offset Auto-Adjustment */
		struct
		{
			uint8_t ClkAutoPong:1;  /*!< Coarse clock auto-adjust on PONG. 1: enable, 0: disable */
			uint8_t ClkAutoAdm:1;   /*!< Fine clock auto-adjust on ADM CMD. 1: enable, 0: disable */
			uint8_t FreqAutoPong:1; /*!< Frequency Offset auto-adjust on PONG. 1: enable, 0: disable */
		} ; //ClkFreqAutoAdj_b;
	};
	uint8_t ClkFreqAutoAdjRssi;   /*!< Clock and Frequency Offset Auto-Adjustment received frame RSSI min.*/


	uint32_t u32DwnDayProgWinMin;
	uint32_t u32DwnDayProgWinMax;
	uint32_t u32DwnDeltaSecMin;
	uint32_t u32DwnBlkDurMod;
	uint32_t u32DwnBlkNbMax;
};

// FIXME
struct adm_config_s sAdmConfig =
{
	.u32DwnDayProgWinMin = DWN_DAY_PROG_WIN_MIN,
	.u32DwnDayProgWinMax = DWN_DAY_PROG_WIN_MAX,
	.u32DwnDeltaSecMin = DWN_DELTA_SEC_MIN_WM2400,
	.u32DwnBlkDurMod = DWN_BLK_DURATION_WM2400
};

/*!
  * @brief          This function treat the L7 ann_download
  *
  * @param [in,out] *pReqMsg Pointer on request message
  * @param [in,out] *pRspMsg Pointer on response message
  * @return         None
  */
static void _adm_anndownload_(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	admin_cmd_anndownload_t *pIn = (admin_cmd_anndownload_t*)(pReqMsg->pData);
	uint8_t *pOut = pRspMsg->pData;
	time_t currentEpoch;
	uint8_t aTemp[4];
	uint32_t u32DaysProg;
	uint32_t u32DwnDuration;
	uint16_t u16BlksCnt;
	uint8_t u8DeltaSecMin;
	uint8_t u8ChId;
	int8_t i8Cmp;

	// prepare the header
	((admin_rsp_t*)pOut)->L7ResponseId = ADM_ANNDOWNLOAD;
	((admin_rsp_t*)pOut)->L7ErrorCode  = ADM_NONE;
	((admin_rsp_t*)pOut)->L7Rssi       = pReqMsg->u8Rssi;
	Param_Access(VERS_FW_TRX, (uint8_t*)(((admin_rsp_t*)pOut)->L7SwVersion), 0);

	do
	{
		/**********************************************************************/
		// Check local update in progress
		/* TODO : get the local update
		if (LocalUpdate)
		{
			((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_UPD_IN_PROGRESS;
			((admin_rsp_err_t*)pOut)->L7ErrorParam = LocalSwVer_LSB;
			break;
		}
		*/
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
		// Check MField
		/* TODO : get the MField
		i8Cmp = memcmp(pIn->L7MField, aTemp, sizeof(pIn->L7MField));
		if ( i8Cmp != 0 )
		{
			((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_ILLEGAL_VALUE;
			((admin_rsp_err_t*)pOut)->L7ErrorParam = ANN_FIELD_ID_L7MField;
			break;
		}
		*/

		// Check Blocks Count
		if(!sAdmConfig.AnnBlocksNB)
		{
			u16BlksCnt = __ntohs(*(uint16_t*)(pIn->L7BlocksCount));
			/* TODO : get the maximum allowed number of block
			if (u16BlksCnt > ImgStore_GetMaxBlockNb() )
			{
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_ILLEGAL_VALUE;
				((admin_rsp_err_t*)pOut)->L7ErrorParam = ANN_FIELD_ID_L7BlocksCount;
				break;
			}
			*/
		}
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
			u32DwnDuration = DWN_BLK_DURATION_WM2400;
			u8DeltaSecMin = DWN_DELTA_SEC_MIN_WM2400;

		}
		else if (pIn->L7ModulationId == PHY_WM4800)
		{
			u32DwnDuration = DWN_BLK_DURATION_WM4800;
			u8DeltaSecMin = DWN_DELTA_SEC_MIN_WM4800;
		}
		else
		{
			//error
			((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_ILLEGAL_VALUE;
			((admin_rsp_err_t*)pOut)->L7ErrorParam = ANN_FIELD_ID_L7ModulationId;
			break;
		}

		// Check Day repeat
		if ( (pIn->L7DayRepeat < 1) || (pIn->L7DayRepeat > 15) )
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

		/**********************************************************************/
		// Check HW version matching
		if (!sAdmConfig.AnnHwId)
		{
			Param_Access(VERS_HW_TRX, aTemp, 0);
			i8Cmp = memcmp(pIn->L7DcHwId, aTemp, sizeof(pIn->L7DcHwId));
			if ( i8Cmp != 0 )
			{
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_INCORRECT_HW_VER;
				((admin_rsp_err_t*)pOut)->L7ErrorParam = aTemp[0];
				break;
			}
		}
		// Check Initial SW version
		if(!sAdmConfig.AnnSwVerIni)
		{
			Param_Access(VERS_FW_TRX, aTemp, 0);
			i8Cmp = memcmp(pIn->L7SwVersionIni, aTemp, sizeof(pIn->L7SwVersionIni));
			if ( i8Cmp != 0 )
			{
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_INCORRECT_INI_SW_VER;
				((admin_rsp_err_t*)pOut)->L7ErrorParam = aTemp[0];
				break;
			}
		}

		// Check Target SW version
		if(!sAdmConfig.AnnSwVerTgt)
		{
			i8Cmp = memcmp(pIn->L7SwVersionTarget, aTemp, sizeof(pIn->L7SwVersionIni));
			if (  i8Cmp == 0 ) // target version already download
			{
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_TGT_VER_DWL;
				((admin_rsp_err_t*)pOut)->L7ErrorParam = aTemp[0];
				break;
			}
			else if (  i8Cmp < 0 ) // target version is lower than current one
			{
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_TGT_SW_VER;
				((admin_rsp_err_t*)pOut)->L7ErrorParam = aTemp[0];
				break;
			}
		}

		// Check Broadcast day
		if(!sAdmConfig.AnnDaysProg)
		{
			u32DaysProg = __ntohl( *(uint32_t*)(pIn->L7DaysProg) );
			time(&currentEpoch);
			currentEpoch -= EPOCH_UNIX_TO_OURS;
			// TODO :
			//currentEpoch = pReqMsg->u32Epoch - EPOCH_UNIX_TO_OURS;

			if ( ( u32DaysProg < ( currentEpoch - (DWN_DAY_PROG_WIN_MIN) ) )  ||
				 ( u32DaysProg > ( currentEpoch + (DWN_DAY_PROG_WIN_MAX) ) )
			   )
			{
				// error on L7DaysProg
				((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_INCORRECT_BCAST_START_DAY;
				((admin_rsp_err_t*)pOut)->L7ErrorParam = (int8_t)(currentEpoch - u32DaysProg);
				break;
			}
		}

		// Check diffusion window
		u32DwnDuration += pIn->L7DeltaSec*1000;
		u32DwnDuration = (u32DwnDuration/1000) * u16BlksCnt - pIn->L7DeltaSec;
		if (u32DwnDuration > 86400)
		{
			// error
			((admin_rsp_err_t*)pOut)->L7ErrorCode = ANN_DIFF_TIME_OUT_OF_WINDOWS;
			((admin_rsp_err_t*)pOut)->L7ErrorParam = 1;
			break;
		}
	} while(0);

	// Check if error
	if (((admin_rsp_t*)pOut)->L7ErrorCode != ADM_NONE)
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
  * @brief          This function treat the L7 unknown
  *
  * @param [in,out] *pReqMsg Pointer on request message
  * @param [in,out] *pRspMsg Pointer on response message
  * @return         None
  */
static void _adm_unknown_(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	((admin_rsp_cmderr_t*)(pRspMsg->pData))->L7ResponseId = pReqMsg->pData[0];
	((admin_rsp_cmderr_t*)(pRspMsg->pData))->L7ErrorCode = ADM_UNK_CMD;
	pReqMsg->u8Size = sizeof(admin_rsp_cmderr_t);
}

typedef void (*pfCmd_t)(net_msg_t *pReqMsg, net_msg_t *pRspMsg);

struct exec_cmd_s{
	pfCmd_t pfCmd;
	uint8_t u8CmdId;
	union {
		uint8_t u8State;
		struct
		{
			uint8_t rsp_ready:1;
			uint8_t cmd_imm:1;
		};
	};
};

const struct exec_cmd_s aExecCmd[] =
{
	[0] = {	.u8CmdId = ADM_READ_PARAM,   .pfCmd = _adm_read_param_,  .rsp_ready = 1, .cmd_imm = 0},
	[1] = {	.u8CmdId = ADM_WRITE_PARAM,  .pfCmd = _adm_write_param_, .rsp_ready = 1, .cmd_imm = 0},
	[2] = {	.u8CmdId = ADM_ANNDOWNLOAD,  .pfCmd = _adm_anndownload_, .rsp_ready = 1, .cmd_imm = 0},
	[3] = {	.u8CmdId = ADM_EXECINSTPING, .pfCmd = _adm_execping_,    .rsp_ready = 0, .cmd_imm = 1},
	[4] = {	.u8CmdId = ADM_WRITE_KEY,    .pfCmd = _adm_write_key_,   .rsp_ready = 1, .cmd_imm = 0},
	// UNKNOWN CMD must be the last one
	[5] = {	.u8CmdId = 0xFF,             .pfCmd = _adm_unknown_,     .rsp_ready = 1, .cmd_imm = 0},
};

/*!
  * @brief       This function pre-treat //TODO
  *
  * @details     This...//TODO
  * @param [//TODO: in or out] None.
  * @retval  0 : response not yet available
  * @retval  1 : response is available
  * @retval  2 : action already done
  */
/*
uint8_t AdmInt_PreCmd(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	uint8_t ret = 1;


	uint8_t i;
	const struct exec_cmd_s *pExecCmd = &(aExecCmd[sizeof(aExecCmd)-1]);
	for (i =0; i < sizeof(aExecCmd); i++)
	{
		if (aExecCmd[i].u8CmdId == pReqMsg->pData[0])
		{
			if(aExecCmd[i].pfCmd)
			{
				pExecCmd = &(aExecCmd[i]);
				break;
			}
			break;
		}
	}

	// check if CMD Id has not previously been received
	if (pReqMsg->u16Id != pRspMsg->u16Id)
	{
		pRspMsg->u8Type = APP_ADMIN;
		pRspMsg->u16Id = pReqMsg->u16Id;

		if (pExecCmd->rsp_ready == 1)
		{
			pExecCmd->pfCmd(pReqMsg, pRspMsg);
		}
		ret = pExecCmd->u8State;

	}
	else // CMD was previously processed
	{
		if (pExecCmd->rsp_ready == 0)
		{
			pExecCmd->pfCmd(pReqMsg, pRspMsg);
		}
		ret = pExecCmd->u8State;
		// if RSP is available, send it (previously build RSP)
		// else, build RSP => EXEC_PING case only
	}
	return ret;
}
*/
uint8_t AdmInt_PreCmd(net_msg_t *pReqMsg, net_msg_t *pRspMsg)
{
	uint8_t ret = 1;
	// check if CMD Id has not previously been received
	if (pReqMsg->u16Id != pRspMsg->u16Id)
	{
		pRspMsg->u8Type = APP_ADMIN;
		pRspMsg->u16Id = pReqMsg->u16Id;
		switch (pReqMsg->pData[0]) //L7CommandId
		{
			case ADM_READ_PARAM :
				_adm_read_param_(pReqMsg, pRspMsg);
				break;
			case ADM_WRITE_PARAM :
				_adm_write_param_(pReqMsg, pRspMsg);
				break;
			case ADM_WRITE_KEY :
				_adm_write_key_(pReqMsg, pRspMsg);
				break;
			case ADM_ANNDOWNLOAD :
				_adm_anndownload_(pReqMsg, pRspMsg);
				break;
			case ADM_EXECINSTPING :
				// response not yet available
				ret = 0;
				break;
			default :
				((admin_rsp_cmderr_t*)(pRspMsg->pData))->L7ResponseId = pReqMsg->pData[0];
				((admin_rsp_cmderr_t*)(pRspMsg->pData))->L7ErrorCode = ADM_UNK_CMD;
				pReqMsg->u8Size = sizeof(admin_rsp_cmderr_t);
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
			_adm_execping_(pReqMsg, pRspMsg);
		}
		ret = 2;
	}
	return ret;
}

#ifdef __cplusplus
}
#endif

/*! @} */
