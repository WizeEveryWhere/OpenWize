/**
  * @file: net_api_private.h
  * @brief This file declare all required to access network device (aka. netdev_t).
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
  * @par 1.0.0 : 2020/05/21[GBI]
  * Initial version
  * @par 2.0.0 : 2021/10/24[GBI]
  * Replace Setter/Getter function by one Ioctl function
  *
  */

/*!
 * @addtogroup wize_net_api
 * @{
 *
 */
#ifndef _NET_API_PRIVATE_H_
#define _NET_API_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @cond INTERNAL
 * @{
 */
#ifndef RECV_BUF_SZ
#define RECV_BUF_SZ 256
#endif
#ifndef SEND_BUF_SZ
#define SEND_BUF_SZ 256
#endif
/*!
 * @}
 * @endcond
 */

#include "net_api.h"
#include "phy_itf.h"
#include "proto_api.h"

/*!
 * @brief This define netdev control/configuration
 */
typedef enum
{
	// Setter
	NETDEV_CTL_SET_UPLINK_CH,  /*!< Set the up-link channel */
	NETDEV_CTL_SET_UPLINK_MOD, /*!< Set the up-link modulation */
	NETDEV_CTL_SET_DWLINK_CH,  /*!< Set the down-link channel */
	NETDEV_CTL_SET_DWLINK_MOD, /*!< Set the down-link modulation */
	NETDEV_CTL_SET_FOFFSET,    /*!< Set the frequency offset */
	NETDEV_CTL_SET_PWR,        /*!< Set the RF transmission power */
	NETDEV_CTL_SET_TRANSLEN,   /*!< Set the max. application layer transmission length */
	NETDEV_CTL_SET_RECVLEN,    /*!< Set the max. application layer reception length */
	NETDEV_CTL_SET_NETWID,     /*!< Set the network ID */
	NETDEV_CTL_SET_DWNID,      /*!< Set the download ID */
	NETDEV_CTL_SET_DEVID,      /*!< Set the device ID */

	NETDEV_CTL_CFG_MEDIUM,     /*!< Configure the medium */
	NETDEV_CTL_CFG_PROTO,      /*!< Configure the protocol */

	// Getter
	NETDEV_CTL_GET_DEVID,     /*!< Get the device ID */

	// Get / Set / Clr Error and stats
	_NETDEV_CTL_ERR_,          /*!< Delimiter for netdev error control */
	NETDEV_CTL_GET_ERR,        /*!< Get the current error code */
	NETDEV_CTL_CLR_ERR,        /*!< Clear the current error */
	NETDEV_CTL_GET_STR_ERR,    /*!< Get the current error string */

	_NETDEV_CTL_STATS_,        /*!< Delimiter for netdev statistics control */
	NETDEV_CTL_GET_STATS,      /*!< Get the statistics */
	NETDEV_CTL_CLR_STATS,      /*!< Clear the statistics */

	NETDEV_CTL_PHY_CMD = 0x8000,  /*!< Pass command to the PHY */
} netdev_ctl_e;

/*!
 * @brief This define the netdev possible events
 */
typedef enum {
	NETDEV_EVT_NONE        = 0x00,
	NETDEV_EVT_RX_STARTED  = 0x01, /*!< Started to receive a packet */
    NETDEV_EVT_RX_COMPLETE = 0x02, /*!< Finished receiving a packet */
	NETDEV_EVT_TX_STARTED  = 0x03, /*!< Started to transfer a packet */
    NETDEV_EVT_TX_COMPLETE = 0x04, /*!< Transfer packet complete */
	NETDEV_EVT_ERROR       = 0x05, /*!< An error occurs in net, phy or stack */
	//----
	NETDEV_EVT_TIMEOUT     = 0x08, /**< Timeout event occurs */
	// ----
	NETDEV_EVT_LAST        = 0x0F, /*!< Last event ID */
} netdev_evt_e;

/*!
 * @brief This define the netdev return status
 */
typedef enum {
    NETDEV_STATUS_OK     = 0x0, /*!< Net device return status OK */
	NETDEV_STATUS_ERROR  = 0x1, /*!< Net device return status ERROR */
	NETDEV_STATUS_BUSY   = 0x2, /*!< Net device return status BUSY */
} netdev_status_e;

/*!
 * @brief This define the netdev possible error types
 */
typedef enum {
    NETDEV_ERROR_NONE  = 0x0, /*!< Error None */
	NETDEV_ERROR_PHY   = 0x1, /*!< Error in phy dev */
	NETDEV_ERROR_PROTO = 0x2, /*!< Error in protocol */
	NETDEV_ERROR_NET   = 0x3, /*!< Error in net dev */
} netdev_err_type_e;

/*!
 * @brief This define the netdev possible states
 */
typedef enum {
    NETDEV_STATE_UNKWON = 0x0, /*!< Net Device state is UNKNOWN */
	NETDEV_STATE_IDLE   = 0x1, /*!< Net device is IDLE */
	NETDEV_STATE_BUSY   = 0x2, /*!< Net device is BUSY */
	NETDEV_STATE_ERROR  = 0x8, /*!< Net device is ERROR */
} netdev_state_e;

/*!
 * @brief This define the netdev type
 */
typedef struct netdev_s netdev_t;

/*!
 * @brief This define the event call-back pointer function
 */
typedef void (*netdev_evt_cb_t)(uint32_t eEvt);

/*!
 * @brief This define the netdev context
 */
struct netdev_s {
    void *pCtx;                 /*!< Pointer on internal context */
    void *hLock;                /*!< Pointer on lock */
    phydev_t *pPhydev;          /*!< Pointer on phy context */
    netdev_state_e eState;      /*!< Current state  */
    netdev_evt_cb_t cbEvent;    /*!< Event call-back */
    netdev_err_type_e eErrType; /*!< Last error type */
 };

/*!
 * @brief This structure define the medium configuration (RX/TX channel,
 * modulation, power...
 */
struct medium_cfg_s {
	// Receiving
	phy_chan_e  eRxChannel;      /*!< RX channel (aka: RF_DOWNLINK_CHANNEL) */
	phy_mod_e   eRxModulation;   /*!< RX modulation (aka: RF_DOWNLINK_MOD) */
	// Transmitting
	phy_chan_e  eTxChannel;      /*!< TX channel (RF_UPLINK_CHANNEL) */
	phy_mod_e   eTxModulation;   /*!< TX modulation (aka: RF_UPLINK_MOD) */
	phy_power_e eTxPower;        /*!< TX power (aka: TX_POWER) */
	int16_t     i16TxFreqOffset; /*!< TX frequency offset (aka: TX_FREQ_OFFSET) */
} ;


/*!
 * @brief This structure define the Wize network context
 */
typedef struct wize_net_s {
	struct proto_ctx_s  sProtoCtx;  /*!< Hold the protocol context (see
	                                     @link proto_ctx_s @endlink)*/
	struct medium_cfg_s sMediumCfg; /*!< Hold the medium configuration (see
	                                     @link medium_cfg_s @endlink)*/

	uint8_t u8ProtoErr;             /*!< Hold the last error code (see
	                                     @link ret_code_e @endlink).*/
    uint8_t aSendBuff[SEND_BUF_SZ]; /*!< Transmission buffer */
    uint8_t aRecvBuff[RECV_BUF_SZ]; /*!< Reception buffer */
} wize_net_t;

// Public WizeNet API
int32_t WizeNet_Setup(netdev_t* pNetdev, wize_net_t* pWizeCtx, phydev_t *pPhydev);

int32_t WizeNet_Init(netdev_t* pNetdev, netdev_evt_cb_t pfcbEvent);
int32_t WizeNet_Uninit(netdev_t* pNetdev);

int32_t WizeNet_Send(netdev_t* pNetdev, net_msg_t *pNetMsg);
int32_t WizeNet_Recv(netdev_t* pNetdev, net_msg_t *pNetMsg);
int32_t WizeNet_Listen(netdev_t* pNetdev);

int32_t WizeNet_Ioctl(netdev_t* pNetdev, uint32_t eCtl, uint32_t args);

#ifdef __cplusplus
}
#endif
#endif /* _NET_API_PRIVATE_H_ */

/*! @} */
