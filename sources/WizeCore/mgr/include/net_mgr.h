/**
  * @file net_mgr.h
  * @brief This file declare functions to use to deal with network device
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
  * @par 2.0.0 : 2021/10/19[GBI]
  * Integrate a more complete management for sending and listening.
  *
  */

/*!
 * @addtogroup wize_net_mgr
 * @{
 *
 */
#ifndef _NET_MGR_H_
#define _NET_MGR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <limits.h>

#include "net_api_private.h"
#include "proto_api.h"
#include "time_evt.h"

/*!
 * @brief This enumeration define the net device events
 */
typedef enum {
	NET_EVENT_NONE        = 0x00, /**< Empty event*/
	NET_EVENT_ERROR       = 0x01, /**< Error event*/
	// ----
	NET_EVENT_SEND_DONE   = 0x02, /**< Message has been sent */
	NET_EVENT_RECV_DONE   = 0x04, /**< Message has been received */
	// ----
	NET_EVENT_TIMEOUT     = 0x08, /**< Timeout event occurs */
	NET_EVENT_FRM_PASSED  = 0x10, /**< Frame received and passed */
	// ----
	NET_EVENT_MSK         = 0x1F
} net_event_e;

/*!
 * @brief This define the network return status
 */
typedef enum {
    NET_STATUS_OK     = 0x0, /*!< Network return status OK */
	NET_STATUS_ERROR  = 0x1, /*!< Network return status ERROR */
	NET_STATUS_BUSY   = 0x2, /*!< Network return status BUSY */
} net_status_e;

/*!
 * @brief This enumeration define the listen type before timeout
 */
typedef enum
{
	NET_LISTEN_TYPE_ONE    = 0x01, /**< One matching message until timeout */
	NET_LISTEN_TYPE_DETECT = 0x02, /**< One matching message, extend timeout if detect occurs */
	NET_LISTEN_TYPE_MANY   = 0x03, /**< Many matching messages until timeout*/
} net_listen_type_e;

/*!
 * @brief This struct defines the network manager context.
 */
struct wize_ctx_s
{
	void *hCaller;				   /*!< Hold the caller task's id */
	void *hTask;				   /*!< Hold the net_mgr task's id  */
	void *hMutex;				   /*!< Hold a mutex to lock the net_mgr */

	void *pBuffDesc;               /*!< Pointer on passed buffer descriptor */
	uint8_t u8TransRetries;        /*!< Number of transmission retry in case of
	                                    internal error */
	uint8_t u8RecvRetries;         /*!< Number of reception retry in case of
	                                    internal error */
	int16_t i16ExpandTmo;          /*!< Expand timeout duration in millisecond */
	uint8_t u8Detected;            /*!< Indicate that a listen message has been detected*/
	uint8_t bListenPend;           /*!< Indicate that a listen is pending*/
	uint8_t u8Type;                /*!< The expected listen application message */
	net_listen_type_e eListenType; /*!< The current listen type */
	time_evt_t sTimeOut;           /*!< Internal timeout on send or listen */
};

void NetMgr_Setup(phydev_t *pPhyDev, wize_net_t *pWizeNet);
int32_t NetMgr_Init(void);

int32_t NetMgr_Open(void *hTaskToNotify);
int32_t NetMgr_Close(void);

int32_t NetMgr_SetUplink(phy_chan_e eChannel, phy_mod_e eMod);
int32_t NetMgr_SetDwlink(phy_chan_e eChannel, phy_mod_e eMod);
int32_t NetMgr_Ioctl(uint32_t eCtl, uint32_t args);
int32_t NetMgr_Send(net_msg_t *pxNetMsg, uint32_t u32TimeOut);
int32_t NetMgr_Listen(net_msg_t *pxNetMsg, uint32_t u32TimeOut, net_listen_type_e eListenType);
int32_t NetMgr_ListenReady(void);

#ifdef __cplusplus
}
#endif
#endif /* _NET_MGR_H_ */

/*! @} */
