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
#include <machine/endian.h>

#include "time_evt.h"
#include "net_mgr.h"
#include "logger.h"

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
	SES_FLG_NONE              = 0x00000000, /*!< Nothing  */
	// -------------------------------------------------------------------------
	SES_FLG_SENDRECV_MSK      = 0x000000FF,
	// -------------------------------------------------------------------------
	SES_FLG_SENT_MSK          = 0x0000000F, /*!< Convenient mask on send flags */
	SES_FLG_DATA_SENT         = 0x00000001, /*!< Data has been sent */
	SES_FLG_RSP_SENT          = 0x00000002, /*!< Response has been sent */
	SES_FLG_PING_SENT         = 0x00000004, /*!< Ping has been sent */
	// ----
	SES_FLG_RECV_MSK          = 0x000000F0, /*!< Convenient mask on recv flags */
	SES_FLG_CMD_RECV          = 0x00000010, /*!< Command has been received */
	SES_FLG_PONG_RECV         = 0x00000020, /*!< Pong has been received*/
	SES_FLG_BLK_RECV          = 0x00000040, /*!< SW Block has been received */

	// -------------------------------------------------------------------------
	SES_FLG_SES_MSK           = 0xFFF00000,
	// -------------------------------------------------------------------------
	SES_FLG_INST_MSK          = 0x00F00000,
	SES_FLG_INST_COMPLETE     = 0x00100000, /*!< Session is complete */
	SES_FLG_INST_ERROR        = 0x00200000, /*!< Error */
	SES_FLG_INST_OUT_DATE     = 0x00400000, /*!< Message is out of date */
	SES_FLG_INST_TIMEOUT      = 0x00800000, /*!< Time Out*/
	// ----
	SES_FLG_ADM_MSK           = 0x0F000000,
	SES_FLG_ADM_COMPLETE      = 0x01000000, /*!< Session is complete */
	SES_FLG_ADM_ERROR         = 0x02000000, /*!< Error */
	SES_FLG_ADM_OUT_DATE      = 0x04000000, /*!< Message is out of date */
	SES_FLG_ADM_TIMEOUT       = 0x08000000, /*!< Time Out*/
	// ----
	SES_FLG_DWN_MSK           = 0xF0000000,
	SES_FLG_DWN_COMPLETE      = 0x10000000, /*!< Session is complete */
	SES_FLG_DWN_ERROR         = 0x20000000, /*!< Error */
	SES_FLG_DWN_OUT_DATE      = 0x40000000, /*!< Message is out of date */
	SES_FLG_DWN_TIMEOUT       = 0x80000000, /*!< Time Out*/

	// -------------------------------------------------------------------------
	SES_FLG_SENDRECV_INST_MSK = 0x00000024,
	SES_FLG_SENDRECV_ADM_MSK  = 0x00000013,
	SES_FLG_SENDRECV_DWN_MSK  = 0x00000040,

	SES_FLG_SES_COMPLETE_MSK  = 0x11100000,
	SES_FLG_SES_ERROR_MSK     = 0x22200000,
	// -------------------------------------------------------------------------
	SES_FLG_UNKNOWN_ERROR     = SES_FLG_SES_ERROR_MSK,
} ses_flg_e;

typedef enum
{
	// -------------------------------------------------------------------------
	SES_EVT_NET_MSK               = NET_EVENT_MSK,
	/* Events from NetMgr intended to the current (own the NetDev) active session
	 * Note : for these event the current active session should be in SENDING
	 * or LISTENING state. Otherwise, something wrong happened.*/
	// -------------------------------------------------------------------------
	SES_EVT_NONE                  = NET_EVENT_NONE,
	SES_EVT_SEND_DONE             = NET_EVENT_SEND_DONE,
	SES_EVT_RECV_DONE             = NET_EVENT_RECV_DONE,
	SES_EVT_FRM_PASSED            = NET_EVENT_FRM_PASSED,
	SES_EVT_TIMEOUT               = NET_EVENT_TIMEOUT,

	// -------------------------------------------------------------------------
	SES_EVT_SES_MSK               = 0xFFF00000,
	// -------------------------------------------------------------------------
	SES_EVT_INST_MSK              = 0x00F00000,
	SES_EVT_INST_OPEN             = 0x00100000, // Event from API to Open the session
	SES_EVT_INST_CANCEL           = 0x00200000, // Event from API to Cancel the session
	SES_EVT_INST_READY            = 0x00400000, // Events from internal to validate (be available) the send or receive buffer.
	SES_EVT_INST_DELAY_EXPIRED    = 0x00800000, // Events from TimeEvt or Timer intended to sessions in waiting state.
	// ----
	SES_EVT_ADM_MSK               = 0x0F000000,
	SES_EVT_ADM_OPEN              = 0x01000000, // Event from API to Open the session
	SES_EVT_ADM_CANCEL            = 0x02000000, // Event from API to Cancel the session
	SES_EVT_ADM_READY             = 0x04000000, // Events from internal to validate (be available) the send or receive buffer.
	SES_EVT_ADM_DELAY_EXPIRED     = 0x08000000, // Events from TimeEvt or Timer intended to sessions in waiting state.
	// ----
	SES_EVT_DWN_MSK               = 0xF0000000,
	SES_EVT_DWN_OPEN              = 0x10000000, // Event from API to Open the session
	SES_EVT_DWN_CANCEL            = 0x20000000, // Event from API to Cancel the session
	SES_EVT_DWN_READY             = 0x40000000, // Events from internal to validate (be available) the send or receive buffer.
	SES_EVT_DWN_DELAY_EXPIRED     = 0x80000000, // Events from TimeEvt or Timer intended to sessions in waiting state.

	// -------------------------------------------------------------------------
	SES_EVT_SES_OPEN_MSK          = 0x11100000,
	SES_EVT_SES_CANCEL_MSK        = 0x22200000,
	SES_EVT_SES_READY_MSK         = 0x44400000,
	SES_EVT_SES_DELAY_EXPIRED_MSK = 0x88800000,

} ses_evt_e;

typedef enum {
	SES_INST = 0x00,
	SES_ADM  = 0x01,
	SES_DWN  = 0x02,
	// ----
	// don't remove the 2 following
	SES_NB,
	SES_NONE = 0xFF,
} ses_type_t;

/******************************************************************************/
struct ses_ctx_s
{
	void *pPrivate;
	void *hTask;
	void *hLock;

	void (*ini)(struct ses_ctx_s *pCtx, uint8_t bCtrl);
	uint32_t (*fsm)(struct ses_ctx_s *pCtx, uint32_t u32Evt);

	time_evt_t sTimeEvt;
	ses_state_e eState;
	ses_type_t eType;
};

/******************************************************************************/

static const char * const _ses_state_str_[] =
{
	[SES_STATE_DISABLE]            = "DISABLE",
	[SES_STATE_IDLE]               = "IDLE",
	[SES_STATE_SENDING]            = "SENDING",
	[SES_STATE_LISTENING]          = "LISTENING",
	[SES_STATE_WAITING]            = "WAITING",
	[SES_STATE_WAITING_RX_DELAY]   = "WAIT_RX_DELAY",
	[SES_STATE_WAITING_TX_DELAY]   = "WAIT_TX_DELAY",
};

#ifdef __cplusplus
}
#endif
#endif /* _SES_DISPATCHER_H_ */

/*! @} */
