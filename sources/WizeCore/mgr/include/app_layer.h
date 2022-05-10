/**
  * @file: app_layer.h
  * @brief This file define the structures and enums belongs to the install and
  * administration layers.
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

#ifndef _APP_LAYER_H_
#define _APP_LAYER_H_
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @cond INTERNAL
 * @{
 */
#ifndef EPOCH_UNIX_TO_OURS
#define EPOCH_UNIX_TO_OURS 1356998400U // second between Unix Epoch and 01/01/2013-00:00:00
#endif
/*!
 * @}
 * @endcond
 */

/*!
 * @brief This enumeration define known COMMAND type
 */
typedef enum {
	ADM_READ_PARAM   = 0x10,  /**< Read parameter(s) */
	ADM_WRITE_PARAM  = 0x20,  /**< Write parameter(s) */
	ADM_WRITE_KEY    = 0x28,  /**< Write a key */
	ADM_ANNDOWNLOAD  = 0x30,  /**< "Announcement" of programmed download */
	ADM_EXECINSTPING = 0x40,  /**< Execution of PING/PONG (install) request */
} admin_cmd_id_e;

/******************************************************************************/
/**** Admin layer related ****/

/*!
 * @brief This enumeration define error code on COMMAND type
 */
typedef enum {
	ADM_NONE          = 0x00, /**< No error */
	ADM_UNK_CMD       = 0xFF, /**< Command is unknown */
} admin_err_code_e;


/*!
 * @brief This struct defines the administration response frame format in case of unknown command
 */
typedef struct {
	uint8_t L7ResponseId; /**< Response id (in @link admin_cmd_id_e @endlink)*/
	uint8_t L7ErrorCode;  /**< Error code */
} admin_rsp_cmderr_t;

/*!
 * @brief This struct defines the administration response frame format in case of success
 */
typedef struct {
	uint8_t L7ResponseId;   /**< Response id (in @link admin_cmd_id_e @endlink) */
	uint8_t L7ErrorCode;    /**< Error code */
	uint8_t L7SwVersion[2]; /**< The current SW version */
	uint8_t L7Rssi;         /**< RSSI of the received message command */
} admin_rsp_t;

/*!
 * @brief This struct defines the administration response frame format in case of error in READ or WRITE PARAMETER command
 */
typedef struct {
	uint8_t L7ResponseId;   /**< Response id (in @link admin_cmd_id_e @endlink) */
	uint8_t L7ErrorCode;    /**< Error code */
	uint8_t L7SwVersion[2]; /**< The current SW version */
	uint8_t L7Rssi;         /**< RSSI of the received message command */
	uint8_t L7ErrorParam;   /**< Parameter Id on which error occurs */
} admin_rsp_err_t;

/******************************************************************************/
/**** Admin read/write parameters ****/

/*!
 * @brief This enumeration define error code when command is a READ_PARAMETER
 */
typedef enum {
	READ_UNK_PARAM     = 0x01, /**< Parameter is unknown */
	READ_ACCES_DENIED  = 0x02, /**< Read access is forbidden */
	READ_LENGTH_EXCEED = 0x03, /**< The read result is too long */
} admin_read_err_code_e;

/*!
 * @brief This enumeration define error code when command is a WRITE_PARAMETER
 */
typedef enum {
	WRITE_UNK_PARAM     = 0x01, /**< Parameter is unknown */
	WRITE_ACCES_DENIED  = 0x02, /**< Write access is forbidden */
	WRITE_ILLEGAL_VALUE = 0x03, /**< The given value to write is illegal */
} admin_write_err_code_e;

/******************************************************************************/
/**** Admin write key ****/

/*!
 * @brief This enumeration define error code when command is a WRITEKEY
 */
typedef enum {
	KEY_INCORRECT_FRM_LEN = 0x01, /**< Frame length is incorrect */
	KEY_ILLEGAL_VALUE     = 0x02, /**< Write key to this Key id is forbidden */
	KEY_KCHG_NOT_USED     = 0x03, /**< The cipher key is not Kchg */
} admin_key_err_code_e;


/*!
 * @brief This struct defines the administration response frame format in case of error in WRITE_KEY command
 */
typedef struct {
	uint8_t L7CommandId;  /**< Command id (in @link admin_cmd_id_e @endlink) */
	uint8_t L7KeyId;      /**< The key type to set */
	uint8_t L7KeyVal[32]; /**< The new key value to set */
	uint8_t L7KeyIndex;   /**< The key id to set */
} admin_cmd_writekey_t;

/******************************************************************************/
/**** Admin anndownload ****/

/*!
 * @brief This enumeration define the filed id in ANN_DOWNLOAD command
 */
typedef enum {
	ANN_FIELD_ID_L7DwnId,           /**< Download number */
	ANN_FIELD_ID_L7Klog,            /**< Klog */
	ANN_FIELD_ID_L7SwVersionIni,    /**< SW initial version */
	ANN_FIELD_ID_L7SwVersionTarget, /**< SW target version */
	ANN_FIELD_ID_L7MField,          /**< MField */
	ANN_FIELD_ID_L7DcHwId,          /**< HW version */
	ANN_FIELD_ID_L7BlocksCount,     /**< Block number to download */
	ANN_FIELD_ID_L7ChannelId,       /**< Channel Id to use */
	ANN_FIELD_ID_L7ModulationId,    /**< Modulation Id to use */
	ANN_FIELD_ID_L7DaysProg,        /**< Programmed day to download */
	ANN_FIELD_ID_L7DayRepeat,       /**< Number of day to repeat */
	ANN_FIELD_ID_L7DeltaSec,        /**< Delta in second between each block */
	ANN_FIELD_ID_L7HashSW,          /**< Hash on the entire SW to download */
} admin_ann_param_id_e;

/*!
 * @brief This enumeration define error code when command is a ANN_DOWNLOAD
 */
typedef enum {
	ANN_ILLEGAL_VALUE             = 0x01, /**< One field value is illegal */
	ANN_INCORRECT_FRM_LEN         = 0x02, /**< The frame length is incorrect */
	ANN_INCORRECT_INI_SW_VER      = 0x03, /**< The initial version is incorrect */
	ANN_INCORRECT_HW_VER          = 0x04, /**< The HW version is incorrect */
	ANN_KCHG_NOT_USED             = 0x05, /**< The Kchg has not been used to cipher the command frame */
	ANN_INCORRECT_BCAST_START_DAY = 0x06, /**< The programmed first day for download is incorrect */
	ANN_UPD_IN_PROGRESS           = 0x07, /**< An update is already in progress */
	ANN_TGT_SW_VER                = 0x08, /**< The target SW version is incorrect */
	ANN_TGT_VER_DWL               = 0x09, /**<  */
	ANN_DIFF_TIME_OUT_OF_WINDOWS  = 0x0A, /**<  */
}admin_ann_err_code_e;

/*!
 * @brief This struct defines the administration command frame in case of ANN_DOWNLOAD command
 */
typedef struct {
	uint8_t L7CommandId;          /**< Command id (in @link admin_cmd_id_e @endlink) */
	uint8_t L7DwnId[3];           /**< The download session identification  */
	uint8_t L7Klog[16];           /**< The klog use to ciphering SW blocks */
	uint8_t L7SwVersionIni[2];    /**< The expected initial SW version  */
	uint8_t L7SwVersionTarget[2]; /**< The target SW version */
	uint8_t L7MField[2];          /**< The expected MField */
	uint8_t L7DcHwId[2];          /**< The expected HW version */
	uint8_t L7BlocksCount[2];     /**< The number of block to download */
	uint8_t L7ChannelId;          /**< The channel Id to use */
	uint8_t L7ModulationId;       /**< The modulation Id to use */
	uint8_t L7DaysProg[4];        /**< Eppch of the programmed first day to download */
	uint8_t L7DayRepeat;          /**< The number of repeat days */
	uint8_t L7DeltaSec;           /**< The delta in second between SW block */
	uint8_t L7HashSW[4];          /**< The Hash computed on the entire downloaded SW */
} admin_cmd_anndownload_t;

/******************************************************************************/
/**** Admin execping ****/

/*!
 * @brief This struct defines the
 */
typedef struct {
	uint8_t L7CommandId; /**< Command id (in @link admin_cmd_id_e @endlink) */
}admin_cmd_execinstping_t;

/*!
 * @brief This struct defines the
 */
typedef struct {
	admin_rsp_t header;   /**<  */
	uint8_t L7NbPong;     /**<  */
	uint8_t L7Pong[3][9]; /**<  */
}admin_rsp_execinstping_t;

/******************************************************************************/
/**** Install layer related ****/

/*!
 * @brief This struct defines the
 */
typedef struct {
	uint8_t L7DownChannel;  /**<  */
	uint8_t L7DownMod;      /**<  */
	uint8_t L7PingRxDelay;  /**<  */
	uint8_t L7PingRxLength; /**<  */
}inst_ping_t;

/*!
 * @brief This struct defines the
 */
typedef struct {
	uint8_t L7ConcentId[6];   /**<  */
	uint8_t L7ModemId;        /**<  */
	uint8_t L7RssiUpstream;   /**<  */
	uint8_t L7RssiDownstream; /**<  */
}inst_pong_t;


/*!
 * @brief This enumeration define
 */
typedef enum {
	PMAX_SET	= 0x01,   /**<  */
	CLK_SET		= 0x02,   /**<  */
	DAYLI_SET	= 0x04,   /**<  */
	CLK_CORR	= 0x08,   /**<  */
	INST_PONG	= 0x1000, /**<  */
	DEFAULT		= 0x8000, /**<  */
}main_param_e;


#ifdef __cplusplus
}
#endif
#endif /* _APP_LAYER_H_ */

/*! @} */
