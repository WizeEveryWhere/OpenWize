/**
  * @file: proto_public.h
  * @brief: This file contains the common ptotocol definitions.
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
  * 1.0.0 : 2020/09/14[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Sources
 * @{
 * @ingroup Wize
 * @{
 * @ingroup Protocol
 * @{
 *
 */

#ifndef _PROTO_H_
#define _PROTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
/*!
 * @def L6VER_WIZE_REV_x_y
 * @brief This macro define the Wize revision number.
 */
#define L6VER_WIZE_REV_0_0 (0x0) //!< Wize Revision 0.0
#define L6VER_WIZE_REV_1_0 (0x1) //!< Wize Revision 1.0
#define L6VER_WIZE_REV_1_1 (0x1) //!< Wize Revision 1.1
#define L6VER_WIZE_REV_1_2 (0x1) //!< Wize Revision 1.2

#ifndef L6VERS
	#define L6VERS L6VER_WIZE_REV_1_2
#endif

#ifndef L6APP_ADM
	#define L6APP_ADM 0x01
#endif

#ifndef L6APP_INST
	#define L6APP_INST 0x02
#endif

//#ifdef WIZE_OPT_USE_CONST_ERR_MSG
extern const char * const wize_err_msg[];
//#endif

/*!
 * @brief This enum defines the application layer payload type .
 */
typedef enum {
	APP_INSTALL,   //!< Application Install (used for PING and PONG).
	APP_ADMIN,     //!< Application Administration (used for COMMAND and RESPONSE).
	APP_DOWNLOAD,  //!< Application Download.
	APP_DATA,      //!< Application Data (used by the specific L7).
	APP_DATA_PRIO, //!< Application Priority Data (used by the specific L7).
	APP_UNKNOWN,   //!< Application payload type is unknown.
	//
	APP_TYPE_NB = APP_UNKNOWN
}app_type_e;

/*!
 * @brief This enum define the common return codes.
 */
typedef enum{
	/* Generic */
	PROTO_SUCCESS               = 0x00, //!< Success, everything is fine.
	PROTO_FAILED                = 0x01, //!< Failed, something wrong happened.
	/* Internal Stack related */
	PROTO_STACK_MISMATCH_ERR          , //!< Incoherent call/parameters between layers
	PROTO_INTERNAL_CRC_ERR            , //!< Internal CRC error computation
	PROTO_INTERNAL_HASH_ERR           , //!< Internal hash error computation
	PROTO_INTERNAL_CIPH_ERR           , //!< Internal cipher or decipher error computation
	PROTO_INTERNAL_NULL_ERR           , //!< Error due to a NULL pointer
	/* Frame related */
	PROTO_FRM_ERR                     , //!< Frame error related
	// error_e
	PROTO_HEAD_END_AUTH_ERR = PROTO_FRM_ERR, //!< Head-End authentication failed (Hash Kenc, Klog, kchg computation result is not valid)
	PROTO_GATEWAY_AUTH_ERR            , //!< Gateway authentication failed (Hash Kmac  computation result is not valid)
	PROTO_PROTO_UNK_ERR               , //!< The Protocol is unknown (not Wize, CiField doesn't match).
	PROTO_FRAME_UNK_ERR               , //!< The Frame is unknown (not Wize, CField doesn't match).
	PROTO_FRAME_CRC_ERR               , //!< The Frame is corrupted (CRC computation is not valid).
	PROTO_FRAME_RS_ERR                , //!< The Frame is corrupted (RS could not correct the frame).
	PROTO_FRAME_SZ_ERR                , //!< The Frame size is too short (< 0x16)
	PROTO_APP_MSG_SZ_ERR              , //!< The APP message length is too long
	PROTO_KEYID_UNK_ERR               , //!< The key id is unknown or not match
	PROTO_NETWID_UNK_ERR              , //!< The Network id is unknown or not match
	// warning_e
	PROTO_FRM_WRN                     , //!< Frame warning related
	PROTO_GATEWAY_AUTH_WRN = PROTO_FRM_WRN, //!< Don't know how to authenticate the Gateway (Kmac selection OprID/NetwID)
	PROTO_DOWNLOAD_VER_WRN            , //!< The Download version is invalid (not register from previous)
	// info_e
	PROTO_FRM_INF                     , //!< Frame info related
	PROTO_DW_BLK_PASS_INF = PROTO_FRM_INF, //!< The Download block id is bypassed (already down loaded)
	PROTO_FRAME_PASS_INF              , //!< The received frame was bypass (we are not the destination device or didn't received an anndownload or not waiting PONG)
	//
	PROTO_RET_CODE_NB
}ret_code_e;

/*!
 * @brief This structure hold the frame statistics
 */
typedef struct frm_err_stats_s
{
	uint32_t u32CrcErr;         /*!< Number of Frame CRC error */
	uint32_t u32RsErr;          /*!< Number of Frame RS error */
	uint32_t u32UnkErr;         /*!< Number of Frame unknown error */
	uint32_t u32LenErr;         /*!< Number of Frame length error */
	uint32_t u32HeadAuthErr;    /*!< Number of Frame head-end authenticate error */
	uint32_t u32GatewayAuthErr; /*!< Number of Frame gateway authenticate error */
	uint32_t u32AppSzErr;       /*!< Number of Frame application size error */
	uint32_t u32PassedErr;      /*!< Number of Frame passed error */
} frm_err_stats_t;

/*!
 * @brief This structure hold the net device statistics
 */
typedef struct proto_stats_s
{
    // Statistics
	// TX
	uint32_t u32TxNbBytes;        /*!< Total transmitted bytes */
	uint32_t u32TxNbFrames;       /*!< Total transmitted frames */
	// RX
	uint32_t u32RxNbBytes;        /*!< Total received bytes */
	uint32_t u32RxNbFrmOK;        /*!< Total received frames without error*/
	uint32_t u32RxNbFrmErr;       /*!< Total received frames with error */
	frm_err_stats_t sFrmErrStats; /*!< Frames statistics */

	// TX
	uint8_t u8TxNoiseMax;         /*!< Maximum Noise */
	uint8_t u8TxNoiseAvg;         /*!< Average Noise */
	uint8_t u8TxNoiseMin;         /*!< Minimum Noise */
	uint8_t u8TxReserved;         /*!< Reserved */
	// RX
	uint8_t u8RxRssiMax;          /*!< Maximum RSSI */
	uint8_t u8RxRssiAvg;          /*!< Average RSSI */
	uint8_t u8RxRssiMin;          /*!< Minimum RSSI */
	uint8_t u8RxReserved;         /*!< Reserved */
} net_stats_t;

/*!
 * @brief This structure is used to hold net device message
 */
typedef struct net_msg_s
{
	uint8_t *pData;              /*!< Pointer on data (transmit or received) */
	uint32_t u32Epoch;           /*!< Epoch of the last frame */
	uint16_t u16Id;              /*!< Counter of the last frame */
	uint8_t u8Size;              /*!< Size of the last frame */
	uint8_t u8Type;              /*!< Type of the last frame*/
	uint8_t u8Rssi;              /*!< RSSI of the last frame */
	uint8_t u8KeyId;             /*!< Key id used for the last frame */
	union {
		uint16_t u16Tstamp;      /*!< Time stamp of the last received frame */
		int16_t i16TxFreqOffset; /*!< Frequency offset of the last received frame */
	};
	union {
		uint8_t Option;          /*!< Application payload option */
		struct
		{
			uint8_t App:1;      /*!< Application paylaod has L6App embedded. */
			uint8_t Ciph:1;     /*!< Application paylaod has ciphered L7 */
		} Option_b;
	};
} net_msg_t;

#ifdef __cplusplus
}
#endif
#endif /* _PROTO_H_ */

/*! @} */
/*! @} */
/*! @} */
