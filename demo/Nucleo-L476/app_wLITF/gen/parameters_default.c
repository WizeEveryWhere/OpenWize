/*!
  * @file parameters_default.c
  * @brief This file was generated from demo/Nucleo-L476/app_2/cfg/MergedParam.xml(Modif. : 2022-03-17 09:48:29.709582400 +0100).
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
  * @par Generation Date
  * 
  * @par x.x.x : 17/03/2022 09:48 [neo]
  *
  */

#include "parameters_def.h"


/******************************************************************************/
/*!
 * @brief This array define the parameter default value
 */
const uint8_t a_ParamDefault[] = {
   0x1,0x0, //!< Hardware version number of the device (or transceiver for a remote module)
   0x1,0x0, //!< Software version number run by the device (or transceiver for a remote module)
   0x0,0x0,0x0,0x0, //!< Date/time of the last successful firmware download
   0x0, //!< Version of the application layer
   0x64, //!< Frequency channel to be used for all uplink message transmissions
   0x78, //!< Frequency channel to be used for all message receptions (except firmware download)
   0x0, //!< Modulation to be used for all uplink message transmissions
   0x0, //!< Modulation to be used for all message receptions (except firmware download)
   0x0, //!< Transceiver nominal transmission power
   0x0,0x0, //!< Maximum time between two COMMAND messages before the device automatically returns to maximum transmission power
   0x0,0x0, //!< Absolute transmission correction frequency offset
   0x1, //!< Fixed wait time after transmission of a DATA message by the device and before opening the COMMAND message listening window
   0xa, //!< Duration of the COMMAND message listening window by the device
   0x1, //!< Time between reception of a COMMAND message by the device and transmission of the corresponding RESPONSE message
   0x0, //!< Minimum value accepted for the EXCH_RESPONSE_DELAY parameter (defined by the device MANUFACTURER)
   0x50, //!< Maximum length of application messages that can be sent by the device
   0x64, //!< Maximum length of application messages that can be received by the device
   0x50,0xe2,0x27,0x0, //!< Current time of device
   0x0,0x0, //!< Relative correction (time delta) to be applied to the device clock once only to correct its absolute drift
   0x0,0x1, //!< Correction of device clock frequency
   0x2, //!< Current key number
   0xe, //!< Number of encryption keys available in the device
   0x1, //!< Kmac key index
   0x1, //!< Fixed waiting time after transmission of an INSTPING message by the device and before opening the INSTPONG message listening window
   0x1, //!< Duration of the INSTPONG message listening window by the device
   0x0, //!< Minimum value of the PING_RX_DELAY parameter
   0x14, //!< Maximum value of the PING_RX_LENGTH parameter
   0x0,0x0,0x0,0x0, //!< Execution time of the last connectivity test (INSTPING/INSTPONG)
   0x0, //!< Number of different INSTPONG messages received in response to the last connectivity test
   0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, //!< Response 1 received for the last connectivity test (Bigest L7RssiDown)
   0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, //!< Response 2 received for the last connectivity test
   0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, //!< Response 3 received for the last connectivity test
   0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, //!< Response 4 received for the last connectivity test
   0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, //!< Response 5 received for the last connectivity test
   0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, //!< Response 6 received for the last connectivity test
   0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, //!< Response 7 received for the last connectivity test
   0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, //!< Response 8 received for the last connectivity test (Lowest L7RssiDown)
   0x6, //!< Periodic time of execping sending by the device, in months
   0x0,0x0,0x0,0x0,0x0,0x0, //!< Radio module serial number
   0x0,0x0, //!< Radio Module manufacturer code
   0x0,0x0,0x0,0x0,0x0,0x0, //!< Module Identification number
   0x0, //!< Module manufacturer code
   0x0, //!< Get or Set the Logger level
   0x0, //!< Get or Set the Logger time option
   };

/******************************************************************************/
const uint8_t a_ENUM_01[6] = { 100, 110, 120, 130, 140, 150, };
const uint8_t a_RANGE_02[2] = { 0, 2, };
const uint8_t a_RANGE_03[2] = { 40, 200, };
const uint8_t a_RANGE_04[2] = { 50, 200, };
const uint8_t a_RANGE_05[2] = { 0, 14, };

