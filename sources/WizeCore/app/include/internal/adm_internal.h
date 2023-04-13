/**
  * @file: adm_internal.h
  * @brief This file define the functions to treat the administration L7
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
#ifndef _ADM_INTERNAL_H_
#define _ADM_INTERNAL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "proto.h"
#include "app_layer.h"

#define BYPASS_DELTASEC_RESTRICTION

#ifndef BYPASS_DELTASEC_RESTRICTION
	#define DWN_DELTA_SEC_MIN_WM2400 10 // in s
	#define DWN_DELTA_SEC_MIN_WM4800 5  // in s
#else
	#define DWN_DELTA_SEC_MIN_WM2400 5 // in s
	#define DWN_DELTA_SEC_MIN_WM4800 3 // in s
#endif

#ifndef DWN_BLK_DURATION_WM2400
	#define DWN_BLK_DURATION_WM2400 1000 // in ms
#endif

#ifndef DWN_BLK_DURATION_WM4800
	#define DWN_BLK_DURATION_WM4800 (DWN_BLK_DURATION_WM2400 / 2)  // in ms
#endif

#ifndef DWN_DAY_PROG_WIN_MIN
	#define DWN_DAY_PROG_WIN_MIN 864000 // (10*24*3600) in s
#endif

#ifndef DWN_DAY_PROG_WIN_MAX
	#define DWN_DAY_PROG_WIN_MAX 5184000 // (60*24*3600) in s
#endif

#ifndef MNT_WIN_DURATION
	#define MNT_WIN_DURATION  14400 // in s from 00:00 UTC
#endif

#ifndef DWN_NB_BLK_MAX
	#define DWN_NB_BLK_MAX  814 // 814 * 210 = 170940 bytes; (524288 - 170940 *3 = 11468)
#endif

//u32DwnBlkNbMax

/*!
 * @brief This struct define some options and filters on admin and Install (L7) Layer
 */
struct adm_config_s{
	uint8_t state;                 /*!< ADM config state*/
	union {
		uint8_t ClkFreqAutoAdj;     /*!< Clock and Frequency Offset Auto-Adjustment */
		struct
		{
			uint8_t ClkAutoPong:1;  /*!< Coarse clock auto-adjust on PONG. 1: enable, 0: disable */
			uint8_t ClkAutoAdm:1;   /*!< Fine clock auto-adjust on ADM CMD. 1: enable, 0: disable */
			uint8_t FreqAutoPong:1; /*!< Frequency Offset auto-adjust on PONG. 1: enable, 0: disable */
		};
	};
	uint8_t ClkFreqAutoAdjRssi;   /*!< Clock and Frequency Offset Auto-Adjustment received frame RSSI min.*/
	// ----
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
		};
	};
	union {
		uint8_t filterDisParam;      /*!< ADM Reception Filter disable*/
		struct
		{
			// TODO : implement it
			uint8_t ParamBypassWO:1; /*!< Read parameter WO filter. 0: enable, 1: disable */
			uint8_t ParamBypassRO:1; /*!< Write parameter RO filter. 0: enable, 1: disable */
		};
	};
	union {
		uint8_t filterDisKey;      /*!< ADM Reception Filter disable*/
		struct
		{
			uint8_t WriteKeyBypassId:1; /*!< Write Key id filter. 0: enable, 1: disable */
			uint8_t KeyChgBypassUse:1;  /*!< Key chg filter. 0: enable, 1: disable */
		};
	};
	// ----
	uint16_t MField;
	// ----
	uint32_t u32DwnDayProgWinMin; /*!< Minimum delay between the AnnDownload and the day of the first block */
	uint32_t u32DwnDayProgWinMax; /*!< Maximum delay between the AnnDownload and the day of the first block */
	uint32_t u32MntWinDuration;   /*!< Maintenance window duration from 00:00 UTC */

	uint32_t u32DwnDeltaSecMin;   /*!< Minimum deltaSec accepted  */
	uint32_t u32DwnBlkDurMod;     /*!< Duration of one block */
	uint32_t u32DwnBlkNbMax;      /*!< Maximum number of block accepted */
	// ----
	uint8_t *pLastWriteParamIds;  /*!< Pointer on table holding the last written parameters id */
	uint8_t *pLastReadParamIds;   /*!< Pointer on table holding the last read parameters id */
	uint8_t u8LastWriteParamNb;   /*!< Number of last written parameters */
	uint8_t u8LastReadParamNb;    /*!< Number of last read parameters */
};

void AdmInt_SetupDefaultConfig(void);
void AdmInt_SetupConfig(void);
uint8_t AdmInt_PreCmd(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_Unknown(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_ReadParam(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_WriteParam(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_WriteKey(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_Execping(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_Anndownload(net_msg_t *pReqMsg, net_msg_t *pRspMsg);

void AdmInt_AnnCheckSession(net_msg_t *pReqMsg, net_msg_t *pRspMsg);

int32_t AdmInt_AnnIsLocalUpdate(void);
uint8_t AdmInt_AnnCheckIntFW(admin_ann_fw_info_t *pFwInfo, uint8_t *u8ErrorParam);

uint8_t AdmInt_PostCmd(net_msg_t *pReqMsg, net_msg_t *pRspMsg);
void AdmInt_PostReadParam(net_msg_t *pReqMsg);
void AdmInt_PostWriteParam(net_msg_t *pReqMsg);
void AdmInt_PostWriteKey(net_msg_t *pReqMsg);
void AdmInt_PostExecping(net_msg_t *pReqMsg);
void AdmInt_PostAnndownload(net_msg_t *pReqMsg);

#ifdef __cplusplus
}
#endif
#endif /* _ADM_INTERNAL_H_ */

/*! @} */
