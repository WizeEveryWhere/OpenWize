/**
  * @file storage.c
  * @brief This file implement storage functions
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
  * @par 1.0.0 : 2021/02/07 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup nucleo_L476_app
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <string.h>
#include "storage.h"

/*!
 * @cond INTERNAL
 * @{
 */

#ifndef PERM_SECTION
#define PERM_SECTION(psection) __attribute__(( section(psection) )) __attribute__((used))
#endif

#ifndef KEY_SECTION
#define KEY_SECTION(ksection) __attribute__(( section(ksection) )) __attribute__((used))  __attribute__(( aligned (2048) ))
#endif

/*!
 * @}
 * @endcond
 */

/******************************************************************************/
#include "parameters_cfg.h"
#include "parameters.h"

extern const uint8_t a_ParamDefault[];

/*!
  * @brief The parameters values table size
  */
const uint16_t u16_ParamValueSz = PARAM_DEFAULT_SZ;

/*!
  * @brief The parameters access table size
  */
const uint8_t u8_ParamAccessCfgSz = PARAM_ACCESS_CFG_SZ;

/*!
  * @brief The restriction table size
  */
const uint8_t u8_ParamRestrCfgSz = PARAM_RESTR_CFG_SZ;


/*!
  * @brief Table of parameters values
  */
PERM_SECTION(".param") uint8_t a_ParamValue[PARAM_DEFAULT_SZ];

/******************************************************************************/
#include "phy_layer.h"
#include "wize_api.h"

/*!
 * @brief This define hard-coded default device id
 */
const device_id_t sDefaultDevId =
{
//==========================================================================
/* How does it work ?
 *
 * ( The Manufacturer ID shall be registered with the Flag association 30
 * http://www.dlms.com/organization/flagmanufacturesids/index.html)
 *
 * Device number : SET 00 02 82 22 30 03
 *
 * Position in alphabet
 * SET => 'S' : 19; 'E' : 5; 'T' : 20
 * Manufacturer ID => ( pos('S') << 5 + pos('E')) << 5 + pos('T') = (19 << 5 + 5) << 5 + 20
 *
 * So, :
 * Manufacturer = 0xB44C
 * TRx Number   = 0x00028222
 * Version      = 0x30
 * Type         = 0x03
 *
 * Then :
 * sDeviceInfo =
   {
		.aManuf = { 0x4C, 0xB4 },
		.aNum = {0x22, 0x82, 0x02, 0x00},
		.u8Ver = 0x30,
		.u8Type = 0x03
	}
 *
 */
//==========================================================================
	.aManuf = { 0xFF, 0xFF },
	.aNum = {0x00, 0x00, 0x00, 0x00},
	.u8Ver = 0x00,
	.u8Type = 0x00
};

/******************************************************************************/
#include "crypto.h"
#include "key_priv.h"

/*!
 * @brief This define some hard-coded default keys
 */
const key_s sDefaultKey[KEY_MAX_NB] =
{
	[0] = {
	.key = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	}},
	[KEY_ENC_MIN+1] = {
	.key = {
		0x55, 0x22, 0x19, 0xe2, 0x65, 0xeb, 0xb4, 0x8c,
		0x8a, 0xdf, 0x58, 0x71, 0x79, 0xd9, 0xc6, 0xb0,
		0x63, 0xd5, 0x7c, 0xa8, 0xd3, 0x7e, 0xd6, 0xcb,
		0x11, 0x51, 0xa7, 0x59, 0xcc, 0xad, 0xba, 0x40
	}},
	[KEY_MAC_ID] = {
	.key = {
		0x88, 0xe3, 0x35, 0x63, 0x8f, 0x52, 0x19, 0x46,
		0xc3, 0x8e, 0x32, 0xee, 0xba, 0xa3, 0xc9, 0x9f,
		0x4a, 0xe7, 0x0b, 0xfb, 0x2b, 0xb2, 0x53, 0x40,
		0x25, 0x04, 0x85, 0x76, 0xe3, 0x81, 0xfe, 0xad
	}}
};

/*!
  * @brief Table of keys
  */
KEY_SECTION(".data.keys") key_s _a_Key_[KEY_MAX_NB];

/******************************************************************************/

/******************************************************************************/
/******************************************************************************/

/*!
  * @brief  This initialize the storage area
  *
  * @param [in] bForce Force to defaults.
  *
  * @retval  None
  *
  */
void Storage_Init(uint8_t bForce)
{
	uint8_t tmp;
	Storage_SetDefault();
	// set the current key id to 0, so un-ciphered
	tmp = 0;
	Param_Access(CIPH_CURRENT_KEY, &tmp, 1);
	tmp = 1;
	// set sessions delay and length to 1
	Param_Access(EXCH_RX_DELAY, &tmp, 1);
	Param_Access(EXCH_RX_LENGTH, &tmp, 1);
	Param_Access(EXCH_RESPONSE_DELAY, &tmp, 1);
	Param_Access(PING_RX_DELAY, &tmp, 1);
	Param_Access(PING_RX_LENGTH, &tmp, 1);
}

/*!
  * @brief  Set to defaults device id, all parameters and all keys
  *
  * @retval  None
  *
  */
void Storage_SetDefault(void)
{
	WizeApi_SetDeviceId(&sDefaultDevId);

	Param_Init(a_ParamDefault);
	memcpy(_a_Key_, sDefaultKey, sizeof(_a_Key_));
}

#ifdef __cplusplus
}
#endif

/*! @} */
