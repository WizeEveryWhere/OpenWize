/**
  * @file: ses_common.h
  * @brief // TODO This file ...
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
  * @par 1.0.0 : 2021/03/16[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup wize_mgr
 * @{
 *
 */
#ifndef _SES_COMMON_H_
#define _SES_COMMON_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include "time_evt.h"
#include "net_mgr.h"
#include "app_layer.h"
#include "logger.h"

typedef union
{
	uint32_t SES_EVT;
	struct
	{
		uint32_t NET_EVT  : 8;
		uint32_t TIM_EVT  : 8;
		uint32_t API_EVT  : 8;
		uint32_t RESERVED : 8;
	} SES_EVT_b;
} ses_evt_t;

typedef union
{
	uint32_t SES_FLG;
	struct
	{
		uint32_t ERROR       : 1; // Error communicating with NetDev
		uint32_t SUCCESS     : 1;
		uint32_t NC1         : 6;

		uint32_t SENT        : 1;
		uint32_t RECEIVED    : 1;
		uint32_t COMPLETE    : 1; // Session terminated
		uint32_t OUT_DATE    : 1; // RSP is not ready
		uint32_t CORRUPTED   : 1; // Received FRM is corrupted
		uint32_t PASSED      : 1; // Received FRM is passed
		uint32_t NC2         : 2;

		uint32_t RX_DELAY    : 1;
		uint32_t TX_DELAY    : 1;
		uint32_t TIMEOUT     : 1;
		uint32_t TIMEOUT_EX  : 1;
		uint32_t DAY_DELAY   : 1;
		uint32_t DELTA_DELAY : 1;
		uint32_t NC3         : 2;

		uint32_t RESEREVD    : 8;
	} SES_FLG_b;
} ses_flg_t;

typedef enum {

	SES_LOG_NONE           = 0,
	SES_LOG_CMD_RECV_READ  = 1,
	SES_LOG_CMD_RECV_WRITE,
	SES_LOG_CMD_RECV_KEY,
	SES_LOG_CMD_RECV_EXEC,
	SES_LOG_CMD_RECV_ANN,

	SES_LOG_PONG_RECV ,
	SES_LOG_BLK_RECV,
	SES_LOG_DATA_SENT,
	SES_LOG_RSP_SENT,
	SES_LOG_PING_SENT,

	SES_LOG_FINE_ADJUST,
	SES_LOG_COARSE_ADJUST,
	SES_LOG_DAY_PASSED,
	SES_LOG_DAYLIGHT_CHG,
} ses_log_e;

/*!
 * @brief This enum define session state
 */
typedef enum
{
	SES_STATE_DISABLE,
	SES_STATE_IDLE,
	SES_STATE_SENDING,
	SES_STATE_LISTENING,
	SES_STATE_WAITING,
	SES_STATE_WAITING_RX_DELAY,
	SES_STATE_WAITING_TX_DELAY
} ses_state_e;

/*!
 * @brief This enum define the session return flags
 */
typedef enum
{
	// -------------------------------------------------------------------------
	// SES_FLG__MSK           = 0x0000000F,
	// ----
	SES_FLG_NONE              = 0x00000000, /*!< Nothing  */
	SES_FLG_FAILED            = 0x00000001, /*!< Failed */
	SES_FLG_SUCCESS           = 0x00000002, /*!< Success */
	SES_FLG_ERROR             = 0x00000004, /*!< Error */
	SES_FLG_TIMEOUT           = 0x00000008, /*!< Time Out*/

	// -------------------------------------------------------------------------
	SES_FLG_RECV_MSK          = 0x000001F0, /*!< Convenient mask on recv flags */
	// ----
	SES_FLG_CMD_RECV          = 0x00000070, /*!< Command has been received */
	SES_FLG_PONG_RECV         = 0x00000080, /*!< Pong has been received*/
	SES_FLG_BLK_RECV          = 0x00000100, /*!< SW Block has been received */

	// -------------------------------------------------------------------------
	SES_FLG_SENT_MSK          = 0x00000E00, /*!< Convenient mask on send flags */
	// ----
	SES_FLG_DATA_SENT         = 0x00000200, /*!< Data has been sent */
	SES_FLG_RSP_SENT          = 0x00000400, /*!< Response has been sent */
	SES_FLG_PING_SENT         = 0x00000800, /*!< Ping has been sent */

	// -------------------------------------------------------------------------
	// SES_FLG__MSK           = 0x0000F000, /*!< */
	// ----
	SES_FLG_COMPLETE          = 0x00001000, /*!< Session is complete */
	SES_FLG_OUT_DATE          = 0x00002000, /*!< Message is out of date */
	SES_FLG_CORRUPTED         = 0x00004000, /*!< Image is corrupted */
	SES_FLG_FRM_PASSED        = 0x00008000, /*!< Frame is by passed*/
} ses_flg_e;

typedef enum
{
	// -------------------------------------------------------------------------
	SES_EVT_NET_MGR_MSK        = NET_EVENT_MSK,
	// ----
	/* Events from NetMgr intended to the current (own the NetDev) active session
	 * Note : for these event the current active session should be in SENDING
	 * or LISTENING state. Otherwise, something wrong happened.*/
	SES_EVT_NONE               = NET_EVENT_NONE,
	SES_EVT_SEND_DONE          = NET_EVENT_SEND_DONE,
	SES_EVT_RECV_DONE          = NET_EVENT_RECV_DONE,
	SES_EVT_FRM_PASSED         = NET_EVENT_FRM_PASSED,
	SES_EVT_TIMEOUT            = NET_EVENT_TIMEOUT,

	// -------------------------------------------------------------------------
	SES_EVT_SES_MGR_MSK        = 0x0000FF00,
	// ----
	/* Events from TimeEvt or Timer intended to sessions in waiting state. */
	SES_EVT_ADM_DELAY_EXPIRED  = 0x00000100,
	SES_EVT_INST_DELAY_EXPIRED = 0x00000200,
	SES_EVT_DWN_DELAY_EXPIRED  = 0x00000400,
	/* Events from internal to validate (be available) the send or receive buffer. */
	SES_EVT_ADM_READY          = 0x00001000,
	SES_EVT_INST_READY         = 0x00002000,
	SES_EVT_DWN_READY          = 0x00004000,

	// -------------------------------------------------------------------------
	SES_EVT_MSK                = 0x00FF0000,
	// ----
	/* Events from API (Adm and Install) to request session start */
	SES_EVT_OPEN               = 0x00010000,
	SES_EVT_CLOSE              = 0x00020000,
	SES_EVT_CANCEL             = SES_EVT_CLOSE,

	// -------------------------------------------------------------------------
	SES_EVT_EXT_MSK            = 0xFF000000,
	// ----
	/* Events from Time Mgr task signaling special time passed. */
	//SES_EVT_FULL_POWER         = 0x08000000,
	//SES_EVT_PERIODIC_INST      = 0x80000000,
	SES_EVT_DAY_PASSED         = 0x04000000,
	//SES_EVT_TIME_ADJUST        = 0x03000000,
} ses_evt_e;

typedef enum {
	SES_INST = 0x00,
	SES_ADM  = 0x01,
	SES_DWN  = 0x02,
	//
	SES_NB,
	SES_NONE = 0xFF,
} ses_type_t;

/******************************************************************************/
struct ses_ctx_s
{
	void *pPrivate;
	void *hTask;
	void *hMutex;
	void *hEvents;
	void (*ini)(struct ses_ctx_s *pCtx);
	uint32_t (*fsm)(struct ses_ctx_s *pCtx, uint32_t u32Evt);

	time_evt_t sTimeEvt;
	ses_state_e eState;
	ses_type_t eType;
};

/******************************************************************************/

static const char * const _ses_name_str_[] =
{
	[SES_INST] = "INST",
	[SES_ADM]  = "ADM",
	[SES_DWN]  = "DWN",
};

static const char * const _ses_state_str_[] =
{
	[SES_STATE_DISABLE]            = "DISABLE",
	[SES_STATE_IDLE]               = "IDLE",
	[SES_STATE_SENDING]            = "SENDING",
	[SES_STATE_LISTENING]          = "LISTENING",
	[SES_STATE_WAITING]            = "WAITING",
	[SES_STATE_WAITING_RX_DELAY]   = "WAITING_RX_DELAY",
	[SES_STATE_WAITING_TX_DELAY]   = "WAITING_TX_DELAY",
};

static const char * const _ses_log_str_[] =
{
	[SES_LOG_NONE]           = "...",
	[SES_LOG_CMD_RECV_READ]  = "READ",
	[SES_LOG_CMD_RECV_WRITE] = "WRITE",
	[SES_LOG_CMD_RECV_KEY]   = "KEY",
	[SES_LOG_CMD_RECV_EXEC]  = "EXEC",
	[SES_LOG_CMD_RECV_ANN]   = "ANN",

	[SES_LOG_PONG_RECV]      = "PONG",
	[SES_LOG_BLK_RECV]       = "BLK",
	[SES_LOG_DATA_SENT]      = "DATA",
	[SES_LOG_RSP_SENT]       = "RSP",
	[SES_LOG_PING_SENT]      = "PING",

	[SES_LOG_FINE_ADJUST]    = "FINE ADJ",
	[SES_LOG_COARSE_ADJUST]  = "COARSE ADJ",
	[SES_LOG_DAY_PASSED]     = "DAY PASSED",
	[SES_LOG_DAYLIGHT_CHG]   = "DAYLIGHT CHG"
};

#ifdef __cplusplus
}
#endif
#endif /* _SES_DISPATCHER_H_ */

/*! @} */
