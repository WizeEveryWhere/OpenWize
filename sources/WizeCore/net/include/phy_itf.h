/**
  * @file: phy_itf.h
  * @brief This file define the interface between the phy and net devices.
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
  * @par 1.0.0 : 2021/10/30[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup wize_phy_itf
 * @{
 *
 */
#ifndef _PHY_ITF_H_
#define _PHY_ITF_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "phy_layer.h"

/*!
 * @cond INTERNAL
 * @{
 */
//------------------------------------------------------------------------------
#define PHY_WM2400_PREAMBLE_DATA 0x5555
#define PHY_WM2400_PREAMBLE_SIZE 16
#define PHY_WM2400_SYNC_WORD 0xF672
#define PHY_WM2400_SYNC_WORD_SIZE 16
#define PHY_WM2400_MODULATION GFSK
#define PHY_WM2400_FILTER_BT 0.5
#define PHY_WM2400_BIT_RATE 2400
#define PHY_WM2400_BIN_ENC NRZ
//------------------------------------------------------------------------------
#define PHY_WM4800_PREAMBLE_DATA 0x5555
#define PHY_WM4800_PREAMBLE_SIZE 16
#define PHY_WM4800_SYNC_WORD_DATA 0xF672
#define PHY_WM4800_SYNC_WORD_SIZE 16
#define PHY_WM4800_MODULATION GFSK
#define PHY_WM4800_FILTER_BT 0.5
#define PHY_WM4800_BIT_RATE 4800
#define PHY_WM4800_BIN_ENC NRZ
//------------------------------------------------------------------------------
#define PHY_WM6400_PREAMBLE_DATA 0xADADADADADADADAD
#define PHY_WM6400_PREAMBLE_SIZE 64
#define PHY_WM6400_SYNC_WORD_DATA 0xDDDDADDAADDDAADA
#define PHY_WM6400_SYNC_WORD_SIZE 64
#define PHY_WM6400_MODULATION 4GFSK
#define PHY_WM6400_FILTER_BT 0.5
#define PHY_WM6400_BIT_RATE 6400
#define PHY_WM6400_BIN_ENC NRZ
//------------------------------------------------------------------------------
#define PHY_FREQUENCY_BASE (169406250UL) // CH100
#define PHY_CHANNEL_WIDTH (12500) // in hertz
#define PHY_FREQUENCY_CH(i) (PHY_FREQUENCY_BASE + i*PHY_CHANNEL_WIDTH)

/******************************************************************************/
#ifndef DEFAULT_MOD
	/*! Define the default modulation (just for initialization) */
	#define DEFAULT_MOD PHY_WM2400
#endif
#ifndef DEFAULT_CH
	/*! Define the default channel (just for initialization) */
	#define DEFAULT_CH PHY_CH120
#endif
#ifndef DEFAULT_TX_POWER
	/*! Define the default TX power (just for initialization) */
	#define DEFAULT_TX_POWER PHY_PMAX_minus_0db
#endif
#ifndef DEFAULT_TX_FREQ_OFFSET
	/*! Define the default TX frequency offset (just for initialization) */
	#define DEFAULT_TX_FREQ_OFFSET 0
#endif
/*!
 * @}
 * @endcond
 */

/******************************************************************************/

/*!
 * @brief This define the PHY device status
 */
typedef enum {
	PHY_STATUS_OK,             /*!< Status OK */
	PHY_STATUS_ERROR,          /*!< Status Error */
	PHY_STATUS_BUSY,           /*!< Status Busy */
	PHY_STATUS_DISABLE = 0xFF, /*!< Status Disable */
} phy_status_e;

/*!
 * @brief This define the available command to change the PHY state
 */
typedef enum {
	PHY_CTL_SET_TX_FREQ_OFF   , /*!< Set the TX frequency Offset */
	PHY_CTL_SET_TX_POWER      , /*!< Set the TX Power */
	PHY_CTL_SET_PA            , /*!< Enable/Disable the PA (if any) */

	PHY_CTL_GET_TX_FREQ_OFF   , /*!< Get the TX frequency Offset */
	PHY_CTL_GET_TX_POWER      , /*!< Get the TX Power */
	PHY_CTL_GET_PA            , /*!< Get the PA state */
	PHY_CTL_GET_FREQ_ERR      , /*!< Get the frequency error */
	PHY_CTL_GET_RSSI          , /*!< Get the RX RSSI */
	PHY_CTL_GET_NOISE         , /*!< Get the TX Noise */
	PHY_CTL_GET_ERR           , /*!< Get the Last error id */
	PHY_CTL_GET_STR_ERR       , /*!< Get the Last error string */

	PHY_CTL_SPE         = 0x40,
	PHY_CTL_SPE_TEST_MODE     , /*!< Test mode (if any) */

	PHY_CTL_CMD         = 0x80,
	PHY_CTL_CMD_PWR_OFF       , /*!< Power off the device */
	PHY_CTL_CMD_PWR_ON        , /*!< Power on the device */
	PHY_CTL_CMD_RESET         , /*!< Reset the PHY device */
	// ----
	PHY_CTL_CMD_READY         , /*!< Go in ready state */
	PHY_CTL_CMD_SLEEP         , /*!< Sleep command */
	// ----
	PHY_CTL_CMD_LAST,
} phy_ctl_e;


/*!
 * @brief This define the available test mode
 */
typedef enum {
    PHY_TST_MODE_NONE   = 0x0,  /*!< Test mode deactivated */
	PHY_TST_MODE_RX     = 0x1,  /*!< RX test mode */
	PHY_TST_MODE_PER_RX = 0x2,  /*!< PER RX test mode */
	PHY_TST_MODE_TX     = 0x3,  /*!< TX test mode */
	//PHY_TST_MODE_PER_TX = 0x4,  /*!<  */
	//
	PHY_NB_TST_MODE,
} phy_test_mode_e;

/*!
 * @brief This define the PHY device events
 */
typedef enum {
	PHYDEV_EVT_NONE,         /**< Nothing special*/
	PHYDEV_EVT_RX_STARTED,   /**< Received packet started (preamble received) */
    PHYDEV_EVT_RX_COMPLETE,  /**< Packet has been received*/
	PHYDEV_EVT_TX_STARTED,   /**< Transfer packet started */
    PHYDEV_EVT_TX_COMPLETE,  /**< Transfer packet complete */
	PHYDEV_EVT_CCA_STARTED,  /**< CCA sequence has started*/
	PHYDEV_EVT_CCA_COMPLETE, /**< CCA sequence complete */
	PHYDEV_EVT_ERROR,		 /**< Error occurs */
} phydev_evt_e;

/*!
 * @brief PHY device event call-back
 */
typedef void (*phydev_evt_cb_t)(void *pCbParam, uint32_t eEvt);

/*!
 * @brief The PHY device type
 */
typedef struct phydev_s phydev_t;

/*!
 * @brief PHY device interface
 */
typedef struct phy_if_s {
	// Function
    int32_t (*pfInit)(phydev_t *pPhydev);   /*!< Initialization function */
    int32_t (*pfUnInit)(phydev_t *pPhydev); /*!< Un-Initialization function */

    int32_t (*pfTx)(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation); /*!< TX or Send function */
    int32_t (*pfRx)(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation); /*!< RX or listen function */

    int32_t (*pfNoise)(phydev_t *pPhydev, phy_chan_e eChannel, phy_mod_e eModulation); /*!< Function to measure the noise (on TX Channel) */

    int32_t (*pfSetSend)(phydev_t *pPhydev, uint8_t *pBuf, uint8_t u8Len);  /*!< Set or write the frame to send into the PHY device internal buffer */
    int32_t (*pfGetRecv)(phydev_t *pPhydev, uint8_t *pBuf, uint8_t *u8Len); /*!< Get or read the received frame from the PHY device internal buffer */

    int32_t (*pfIoctl)(phydev_t *pPhydev, uint32_t eCtl, uint32_t args); /*!< IOCTL function */
} phy_if_t;

/*!
 * @brief PHY device structure
 */
struct phydev_s {
	const phy_if_t *pIf;          /*!< Pointer on device interface */
    void *pCxt;                   /*!< Device internal context */
    uint32_t lock;                /*!< Device lock */
    phydev_evt_cb_t pfEvtCb;      /*!< Event notifier callback function */
    void *pCbParam;

    // private field
	phy_chan_e  eChannel;          /*!< Current Channel */
	phy_mod_e   eModulation;       /*!< Current modulation */
	phy_power_e eTxPower;          /*!< Current TX power */
	int16_t     i16TxFreqOffset;   /*!< Current Frequency offset */

	uint16_t    u16_Noise;         /*!< Last noise measured */
	uint16_t    u16_Rssi;          /*!< Last RSSI measured */
	int16_t     u16_Ferr;          /*!< Last frequency error measured */

	uint8_t     bCrcOn;            /*!< Enable/Disable PHY crc computation */
	uint8_t     bPreSyncOn;        /*!< Enable/Disable interrupt on PREMABLE and SYNCHRO */

	phy_test_mode_e eTestMode;     /*!< Current Test mode (internal)*/
};

#ifdef __cplusplus
}
#endif
#endif /* _PHY_ITF_H_ */

/*! @} */
