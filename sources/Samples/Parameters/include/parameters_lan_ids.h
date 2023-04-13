/*!
  * @file parameters_lan_ids.h
  * @brief This define the generic LAN parameters ids
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
  * @par 1.0.0 : 2020/11/22[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup parameters
 * @{
 *
 */
#ifndef _PARAMETERS_LAN_IDS_H_
#define _PARAMETERS_LAN_IDS_H_

#ifdef __cplusplus
extern C {
#endif

#include "parameters_def.h"

/******************************************************************************/
/*!
 * @brief This enum defines the lan parameter ids.
 */
typedef enum {
    VERS_HW_TRX = 0x01, //!< Hardware version number of the device (or transceiver for a remote module)
    VERS_FW_TRX = 0x02, //!< Software version number run by the device (or transceiver for a remote module)
    DATEHOUR_LAST_UPDATE = 0x03, //!< Date/time of the last successful firmware download
    L6App = 0x04, //!< Version of the application layer
    RF_UPLINK_CHANNEL = 0x08, //!< Frequency channel to be used for all uplink message transmissions
    RF_DOWNLINK_CHANNEL = 0x09, //!< Frequency channel to be used for all message receptions (except firmware download)
    RF_UPLINK_MOD = 0x0A, //!< Modulation to be used for all uplink message transmissions
    RF_DOWNLINK_MOD = 0x0B, //!< Modulation to be used for all message receptions (except firmware download)
    TX_POWER = 0x10, //!< Transceiver nominal transmission power
    TX_DELAY_FULLPOWER = 0x11, //!< Maximum time between two COMMAND messages before the device automatically returns to maximum transmission power
    TX_FREQ_OFFSET = 0x12, //!< Absolute transmission correction frequency of
    EXCH_RX_DELAY = 0x18, //!< Fixed wait time after transmission of a DATA message by the device and before opening the COMMAND message listening window
    EXCH_RX_LENGTH = 0x19, //!< Duration of the COMMAND message listening window by the device
    EXCH_RESPONSE_DELAY = 0x1A, //!< Time between reception of a COMMAND message by the device and transmission of the corresponding RESPONSE message
    EXCH_RESPONSE_DELAY_MIN = 0x1B, //!< Minimum value accepted for the EXCH_RESPONSE_DELAY parameter (defined by the device MANUFACTURER)
    L7TRANSMIT_LENGTH_MAX = 0x1C, //!< Maximum length of application messages that can be sent by the device
    L7RECEIVE_LENGTH_MAX = 0x1D, //!< Maximum length of application messages that can be received by the device
    CLOCK_CURRENT_EPOC = 0x20, //!< Current time of device
    CLOCK_OFFSET_CORRECTION = 0x21, //!< Relative correction (time delta) to be applied to the device clock once only to correct its absolute drift
    CLOCK_DRIFT_CORRECTION = 0x22, //!< Correction of device clock frequency
    CIPH_CURRENT_KEY = 0x28, //!< Current key number
    CIPH_KEY_COUNT = 0x29, //!< Number of encryption keys available in the device
	L6NetwIdSelect = 0x2A, //!< Kmac key index
    PING_RX_DELAY = 0x30, //!< Fixed waiting time after transmission of an INSTPING message by the device and before opening the INSTPONG message listening window
    PING_RX_LENGTH = 0x31, //!< Duration of the INSTPONG message listening window by the device
    PING_RX_DELAY_MIN = 0x32, //!< Minimum value of the PING_RX_DELAY parameter
    PING_RX_LENGTH_MAX = 0x33, //!< Maximum value of the PING_RX_LENGTH parameter
    PING_LAST_EPOCH = 0x34, //!< Execution time of the last connectivity test (INSTPING/INSTPONG)
    PING_NBFOUND = 0x35, //!< Number of different INSTPONG messages received in response to the last connectivity test
    PING_REPLY1 = 0x36, //!< Response 1 received for the last connectivity test (Bigest L7RssiDown)
    PING_REPLY2 = 0x37, //!< Response 2 received for the last connectivity test
    PING_REPLY3 = 0x38, //!< Response 3 received for the last connectivity test
    PING_REPLY4 = 0x39, //!< Response 4 received for the last connectivity test
    PING_REPLY5 = 0x3A, //!< Response 5 received for the last connectivity test
    PING_REPLY6 = 0x3B, //!< Response 6 received for the last connectivity test
    PING_REPLY7 = 0x3C, //!< Response 7 received for the last connectivity test
    PING_REPLY8 = 0x3D, //!< Response 8 received for the last connectivity test (Weakess L7RssiDown)
    EXECPING_PERIODE = 0x3E, //!< Periodic time of execping sending by the device, in months
	// ---
#ifdef HAS_WIZE_CORE_EXTEND_PARAMETER
	AUTO_ADJ_CLK_FREQ    = 0xDA, //!< Clock and Frequency Offset Auto-Adjustment. MSB : control, LSB : RSSI min. level (see struct adm_config_s).
	// ---
	ADM_ANN_DIS_FLT      = 0xDD, //!< ADM ANN Reception Filter disable (see struct adm_config_s)
    ADM_PARAM_DIS_FLT    = 0xDE, //!< ADM R/W parameters Reception Filter disable (see struct adm_config_s)
    ADM_KEY_DIS_FLT      = 0xDF, //!< ADM Key Change Reception Filter disable (see struct adm_config_s)
	// ---
	DWN_DAY_PRG_WIN_MIN  = 0xE0, //!< Minimum delay between the AnnDownload and the day of the first block (see struct adm_config_s)
    DWN_DAY_PRG_WIN_MAX  = 0xE1, //!< Maximum delay between the AnnDownload and the day of the first block (see struct adm_config_s)
    MNT_WINDOW_DURATION  = 0xE2, //!< Maintenance Window duration in second from 00:00:00 UTC (see struct adm_config_s)
	// ---
	DWN_DELTA_SEC_MIN    = 0xE3, //!< Minimum deltaSec accepted in second (see struct adm_config_s)
    DWN_BLK_DURATION_MOD = 0xE4, //!< Duration of one block in ms (see struct adm_config_s)
    DWN_BLK_NB_MAX       = 0xE5, //!< Maximum number of block accepted (see struct adm_config_s)
	// ---
	ADM_RECEPTION_OFFSET = 0xEA, //!< Offset to start RX before (in ms) (MSB first, Signed number limited to +/-63ms).
    DWN_RECEPTION_OFFSET = 0xEB, //!< Offset to start RX before (in ms) (MSB first, Signed number limited to +/-1000ms).
	// ---
	L2_EXCH_DIS_FLT      = 0xEE, //!< L2 Reception Filter disable (see struct proto_config_s)
    L6_EXCH_DIS_FLT      = 0xEF, //!< L6 Reception Filter disable (see struct proto_config_s)
	// ---
	LAST_ID = L6_EXCH_DIS_FLT, //!< Don't remove, it marks the end of table.
#else
	LAST_ID = EXECPING_PERIODE, //!< Don't remove, it marks the end of table.
#endif

}param_lan_ids_e;

/******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* _PARAMETERS_LAN_IDS_H_ */

/*! @} */
