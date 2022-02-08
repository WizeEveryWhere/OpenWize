/*!
  * @file proto_private.h
  * @brief This file define common structures, enum, ...
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
  * 1.0.0 : 2019/11/22[GBI]
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
#ifndef _PROTO_PRIVATE_H_
#define _PROTO_PRIVATE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "proto.h"

/******************************************************************************/
//! ID used before Wize was register
#define PRE_WIZE_PROTO_ID (0xB4)
//! Current Wize ID
#define STD_WIZE_PROTO_ID (0x20)

#if L6VERS == L6VER_WIZE_REV_0_0
	#define WIZE_PROTO_ID PRE_WIZE_PROTO_ID
#else
	#define WIZE_PROTO_ID STD_WIZE_PROTO_ID
#endif

#ifndef L6_DOWNLOAD_VER
	#define L6_DOWNLOAD_VER 0x00
#endif

/******************************************************************************/
// L2 related
/*!
 * @def LFIELD_POS
 * @brief Length Field size in Link layer frame.
 */
#define LFIELD_SZ  1

/*!
 * @def CFIELD_SZ
 * @brief Command Field size in Link layer frame.
 */
#define CFIELD_SZ  1

/*!
 * @def MFIELD_SZ
 * @brief Manufacturer Field size in Link layer frame.
 */
#define MFIELD_SZ  2

/*!
 * @def AFIELD_SZ
 * @brief Device Field size in Link layer frame.
 */
#define AFIELD_SZ  6

/*!
 * @def CLFIELD_SZ
 * @brief Cl Field size in Link layer frame.
 */
#define CLFIELD_SZ  1

/*!
 * @def L2DWNID_SZ
 * @brief Download Id Field size in Link layer frame.
 */
#define L2DWNID_SZ  3

/*!
 * @def DATA_PRI_MSK
 * @brief This define the data priority mask
 */
#define DATA_PRI_MSK 0x10

/*!
 * @def CRC_SZ
 * @brief Cyclic Redundancy Check Field size in Link layer frame.
 */
#define CRC_SZ  2

/*!
 * @def RSCODE_SZ
 * @brief Reed-Solomon Field size in Link layer frame.
 */
#define RSCODE_SZ 32

/******************************************************************************/
// L6 related

/*!
 * @def L6_CTRL_SZ
 * @brief Control field size
 */
#define L6_CTRL_SZ 1

/*!
 * @def L6_NETWID_SZ
 * @brief Network Id field size
 */
#define L6_NETWID_SZ 1


/*!
 * @def L6_CPT_SZ
 * @brief Frame Counter field size
 */
#define L6_CPT_SZ 2

/*!
 * @def L6_APP_SZ
 * @brief Application Id field size
 */
#define L6_APP_SZ 1

/*!
 * @def L6_DWN_VERS_SZ
 * @brief Download version protocol field size
 */
#define L6_DWN_VERS_SZ 1

/*!
 * @def L6_DWN_B_NUM_SZ
 * @brief Download block number field size
 */
#define L6_DWN_B_NUM_SZ 3

/*!
 * @def L6_HASH_KENC_SZ
 * @brief Hash Kenc field size
 */
#define L6_HASH_KENC_SZ 4

/*!
 * @def L6_TSTAMP_SZ
 * @brief Time Stamp field size
 */
#define L6_TSTAMP_SZ 2

/*!
 * @def L6_EPOCH_SZ
 * @brief The EPOCH field size
 */
#define L6_EPOCH_SZ 4

/*!
 * @def L6_TX_FREQ_OFFSET_SZ
 * @brief The Transmission Frequency Offset field size
 */
#define L6_TX_FREQ_OFFSET_SZ 2

/*!
 * @def L6_HASH_KMAC_SZ
 * @brief Hash Kmac field size
 */
#define L6_HASH_KMAC_SZ 2

/*!
 * @def L6_HASH_KLOG_SZ
 * @brief Hash Klog field size
 */
#define L6_HASH_KLOG_SZ 4


//#define FRAME_SEND_MAX_SZ 229
#define FRAME_SEND_MAX_SZ \
(255 - ( \
/* 10 */ \
CFIELD_SZ + MFIELD_SZ + AFIELD_SZ + CLFIELD_SZ + \
/* 5 */ \
L6_CTRL_SZ + L6_NETWID_SZ + L6_CPT_SZ + L6_APP_SZ + \
 /* 8 */ \
L6_HASH_KENC_SZ + L6_TSTAMP_SZ + L6_HASH_KMAC_SZ + \
 /* 2 */ \
CRC_SZ))


/******************************************************************************/
/*!
 * @brief This enum defines the link layer frame type .
 */
typedef enum {
	INSTPONG  = 0x06, /*!< PONG (CNF_IR). From Gateway to Device */
	INSTPING  = 0x46, /*!<  PING (SND_IR). From Device to Gateway */
	COMMAND   = 0x43, /*!<  COMMAND (SND_UD2). From Gateway to Device */
	RESPONSE  = 0x08, /*!<  RESPONSE (RSP_UD). From Device to Gateway */
	DATA      = 0x44, /*!<  DATA (SND_NR). From Device to Gateway */
	DATA_PRIO = DATA | DATA_PRI_MSK, /*!<  Priority DATA. From Device to Gateway */
}cfield_e;

/* L2 Layer fields */

/*!
 * @brief This structure defines the link layer exchange header.
 */
typedef struct {
	uint8_t Cfield;            /*!< Frame type */
	uint8_t Mfield[MFIELD_SZ]; /*!< Indication of the device MANUFACTURER */
	uint8_t Afield[AFIELD_SZ]; /*!< Unique ID of the device */
	uint8_t Cifield;           /*!< Wize Protocol Id*/
}l2_exch_header_t;

/*!
 * @brief This structure defines the link layer download header.
 */
typedef struct {
	uint8_t L2DownId[L2DWNID_SZ]; /*!< Identification number of the software
	                                   download sequence */
}l2_down_header_t;

/*!
 * @brief This structure defines the link layer exchange footer.
 */
typedef struct {
	uint8_t Crc[CRC_SZ]; /*!< Message checksum */
}l2_exch_footer_t;

/*!
 * @brief This structure defines the link layer download footer.
 */
typedef struct {
	uint8_t Crc[CRC_SZ];   /*!< Message checksum */
	uint8_t Rs[RSCODE_SZ]; /*!< Error correction block (Reed-Solomon RS(255,223) ) */
}l2_down_footer_t;

/* L6 Layer fields */

/*!
 * @brief This structure defines the presentation layer exchange header.
 */
typedef struct {
	union {
		uint8_t L6Ctrl;       /*!< Control field */
		struct {
			uint8_t KEYSEL:4; /*!< Current encryption and authentication key id */
			uint8_t WTS:1;    /*!< Reserved value, set to 1. */
			uint8_t VERS:3;   /*!< The LAN protocol Version */
		}L6Ctrl_b;
	};
#if L6VERS == L6VER_WIZE_REV_1_0 || L6VERS == L6VER_WIZE_REV_1_1
	uint8_t L6NetwId;         /*!< Network identifier */
#endif
	uint8_t L6Cpt[L6_CPT_SZ]; /*!< Message incremental number */
#if L6VERS == L6VER_WIZE_REV_1_0 || L6VERS == L6VER_WIZE_REV_1_1
	uint8_t L6App;            /*!< The application layer Id. */
#endif
}l6_exch_header_t;

/*!
 * @brief This structure defines the presentation layer download header.
 */
typedef struct {
	uint8_t L6DownVer;                   /*!< Software download protocol version */
	uint8_t L6DownBnum[L6_DWN_B_NUM_SZ]; /*!< Software sequence block number. */
}l6_down_header_t;

/*!
 * @brief This structure defines the presentation layer exchange footer.
 */
typedef struct {
	union {
		uint8_t L6HashKenc[L6_HASH_KENC_SZ];        /*!< Authentication signature
		                                                 of the transmitter of
		                                                 the message between the
		                                                 Head-End system and the
		                                                 device. */

		uint8_t Epoch[L6_EPOCH_SZ];                 /*!< */
	};
	union {
		uint8_t L6TStamp[L6_TSTAMP_SZ];             /*!< Time stamp of the
		                                                 transmission time of
		                                                 the message by the device
		                                                  or the gateway.*/
		uint8_t TxFreqOffset[L6_TX_FREQ_OFFSET_SZ]; /*!< */
	};
	uint8_t L6HKmac[L6_HASH_KMAC_SZ];               /*!< Authentication signature
	                                                     of the transmitter of
	                                                     the message between the
	                                                     gateway and the device.*/

}l6_exch_footer_t;

/*!
 * @brief This structure defines the presentation layer download footer.
 */
typedef struct {
	uint8_t L6HashLog[L6_HASH_KLOG_SZ]; /*!< Authentication footprint of the
	                                         transmitter of the message between
	                                         the gateway and the device. */
}l6_down_footer_t;

/******************************************************************************/

/*!
 * @brief This structure define the protocol configuration
 */
struct proto_config_s {
	union {
		uint8_t filterDisL2;           /*!< L2 Reception Filter disable*/
		struct
		{
			uint8_t Crc:1;             /*!< Crc filter. 0: enable, 1: disable */
			uint8_t AField:1;          /*!< Afield filter. 0: enable, 1: disable */
			uint8_t MField:1;          /*!< MField filter. 0: enable, 1: disable */
			uint8_t CiField:1;         /*!< CiFiled filter. 0: enable, 1: disable */
			uint8_t DownId:1;          /*!< DownId filter. 0: enable, 1: disable */
		} filterDisL2_b;
	};
	union {
		uint8_t filterDisL6;           /*!< L6 Reception Filter disable*/
		struct
		{
			uint8_t App:1;             /*!< L6App filter. 0: enable, 1: disable */
			uint8_t HashKmac:1;        /*!< HashKmac filter. 0: enable, 1: disable */
			uint8_t HashKenc:1;        /*!< HashKenc filter. 0: enable, 1: disable */
			uint8_t NetId:1;           /*!< L6NetId filter. 0: enable, 1: disable */
			uint8_t Ciph:1;            /*!< L7 un-cipher filter. 0: enable, 1: disable
			                                If disable, it is assume that the caller
			                                will un-cipher the L7. */
		} filterDisL6_b;
	};
	uint8_t u8TransLenMax;             /*!< Max Length to transmit */
	uint8_t u8RecvLenMax;              /*!< Max Length to received */
	uint8_t u8NetId;                   /*!< Network Id */
	uint8_t DwnId[3];                  /*!< Download Sequence Number */

	uint8_t AppInst;                   /*!< L6App used for Install layer */
	uint8_t AppAdm;                    /*!< L6App used for Admin layer */
	uint8_t AppData;                   /*!< L6App used for Data layer */
};

/*!
 * @brief This structure define the protocol context
 */
struct proto_ctx_s
{
	struct proto_config_s sProtoConfig; /*!< Protocol filter configuration*/
	struct proto_stats_s  sProtoStats;  /*!< Protocol TX/RX statistics */
	uint8_t *pBuffer;                   /*!< Pointer on input/output buffer*/
	uint8_t u8Size;                     /*!< Frame Size to send or received */

	uint8_t aDeviceManufID[MFIELD_SZ];  /*!< Device Manufacturer Id */
	uint8_t aDeviceAddr[AFIELD_SZ];     /*!< Device Unique Id */
};

#ifdef __cplusplus
}
#endif
#endif /* _PROTO_PRIVATE_H_ */

/*! @} */
/*! @} */
/*! @} */
