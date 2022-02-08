/**
  * @file: app_layer.h
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
  * 1.0.0 : 2020/10/11[GBI]
  * Initial version
  *
  *
  */
#ifndef _APP_LAYER_H_
#define _APP_LAYER_H_
#ifdef __cplusplus
extern "C" {
#endif

#ifndef EPOCH_UNIX_TO_OURS
#define EPOCH_UNIX_TO_OURS 1356998400U // second between Unix Epoch and 01/01/2013-00:00:00
#endif

typedef enum {
	ADM_READ_PARAM   = 0x10,
	ADM_WRITE_PARAM  = 0x20,
	ADM_WRITE_KEY    = 0x28,
	ADM_ANNDOWNLOAD  = 0x30,
	ADM_EXECINSTPING = 0x40,
}admin_cmd_id_e;

/******************************************************************************/
/*
 * Admin layer related
 */

typedef enum {
	ADM_NONE          = 0x00,
	ADM_UNK_CMD       = 0xFF,
}admin_err_code_e;

typedef struct {
	uint8_t L7CommandId;
	uint8_t *data;
}admin_cmd_t;

typedef struct {
	uint8_t L7ResponseId;
	uint8_t L7ErrorCode;
}admin_rsp_cmderr_t;

typedef struct {
	uint8_t L7ResponseId;
	uint8_t L7ErrorCode;
	uint8_t L7SwVersion[2];
	uint8_t L7Rssi;
}admin_rsp_t;

typedef struct {
	uint8_t L7ResponseId;
	uint8_t L7ErrorCode;
	uint8_t L7SwVersion[2];
	uint8_t L7Rssi;
	uint8_t L7ErrorParam;
}admin_rsp_err_t;

/*
 * Admin read/write parameters
 */

typedef enum {
	READ_UNK_PARAM     = 0x01,
	READ_ACCES_DENIED  = 0x02,
	READ_LENGTH_EXCEED = 0x03,
}admin_read_err_code_e;

typedef enum {
	WRITE_UNK_PARAM     = 0x01,
	WRITE_ACCES_DENIED  = 0x02,
	WRITE_ILLEGAL_VALUE = 0x03,
}admin_write_err_code_e;

/*
 * Admin write key
 */
typedef enum {
	KEY_INCORRECT_FRM_LEN = 0x01,
	KEY_ILLEGAL_VALUE     = 0x02,
	KEY_KCHG_NOT_USED     = 0x03,
}admin_key_err_code_e;


typedef enum {
	PMAX_SET	= 0x01,
	CLK_SET		= 0x02,
	DAYLI_SET	= 0x04,
	CLK_CORR	= 0x08,
	INST_PONG	= 0x1000,
	DEFAULT		= 0x8000,
}main_param_e;


// Write key cmd
typedef struct {
	uint8_t L7CommandId;
	uint8_t L7KeyId;
	uint8_t L7KeyVal[32];
	uint8_t L7KeyIndex;
}admin_cmd_writekey_t;

/*
 * Admin anndownload
 */

// anndownload field id
typedef enum {
	ANN_FIELD_ID_L7DwnId,
	ANN_FIELD_ID_L7Klog,
	ANN_FIELD_ID_L7SwVersionIni,
	ANN_FIELD_ID_L7SwVersionTarget,
	ANN_FIELD_ID_L7MField,
	ANN_FIELD_ID_L7DcHwId,
	ANN_FIELD_ID_L7BlocksCount,
	ANN_FIELD_ID_L7ChannelId,
	ANN_FIELD_ID_L7ModulationId,
	ANN_FIELD_ID_L7DaysProg,
	ANN_FIELD_ID_L7DayRepeat,
	ANN_FIELD_ID_L7DeltaSec,
	ANN_FIELD_ID_L7HashSW,
}admin_ann_param_id_e;


typedef enum {
	ANN_ILLEGAL_VALUE             = 0x01,
	ANN_INCORRECT_FRM_LEN         = 0x02,
	ANN_INCORRECT_INI_SW_VER      = 0x03,
	ANN_INCORRECT_HW_VER          = 0x04,
	ANN_KCHG_NOT_USED             = 0x05,
	ANN_INCORRECT_BCAST_START_DAY = 0x06,
	ANN_UPD_IN_PROGRESS           = 0x07,
	ANN_TGT_SW_VER                = 0x08,
	ANN_TGT_VER_DWL               = 0x09,
	ANN_DIFF_TIME_OUT_OF_WINDOWS  = 0x0A,
}admin_ann_err_code_e;

// anndownload cmd
typedef struct {
	uint8_t L7CommandId;
	uint8_t L7DwnId[3];
	uint8_t L7Klog[16];
	uint8_t L7SwVersionIni[2];
	uint8_t L7SwVersionTarget[2];
	uint8_t L7MField[2];
	uint8_t L7DcHwId[2];
	uint8_t L7BlocksCount[2];
	uint8_t L7ChannelId;
	uint8_t L7ModulationId;
	uint8_t L7DaysProg[4];
	uint8_t L7DayRepeat;
	uint8_t L7DeltaSec;
	uint8_t L7HashSW[4];
}admin_cmd_anndownload_t;

/*
 * Admin execping
 */

// exec_ping
typedef struct {
	uint8_t L7CommandId;
}admin_cmd_execinstping_t;

typedef struct {
	admin_rsp_t header;
	uint8_t L7NbPong;
	uint8_t L7Pong[3][9];
}admin_rsp_execinstping_t;

/******************************************************************************/
/*
 * Install layer related
 */
typedef struct {
	uint8_t L7DownChannel;
	uint8_t L7DownMod;
	uint8_t L7PingRxDelay;
	uint8_t L7PingRxLength;
}inst_ping_t;

typedef struct {
	uint8_t L7ConcentId[6];
	uint8_t L7ModemId;
	uint8_t L7RssiUpstream;
	uint8_t L7RssiDownstream;
}inst_pong_t;

#ifdef __cplusplus
}
#endif
#endif /* _APP_LAYER_H_ */
