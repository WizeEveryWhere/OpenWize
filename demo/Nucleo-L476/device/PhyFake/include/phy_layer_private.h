/**
  * @file phy_layer_private.h
  * @brief This file define the phy fake device driver.
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
  * @par 1.0.0 : 2021/11/24 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup phy_fake
 * @ingroup device
 * @{
 */

#ifndef _PHY_LAYER_PRIVATE_H_
#define _PHY_LAYER_PRIVATE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "phy_itf.h"

/*!
 * @cond INTERNAL
 * @{
 */
#define BUF_SZ 515 // 2 (packet len) + 256*2 because it communicate on uart, so we need to convert hex to char + 1 (EOB)

/*!
 * @}
 * @endcond
 */


/*!
 * @brief This enum defines the PḧyFake state.
 */
typedef enum
{
	IDLE_STATE         = 0x00,
	INITIALIZED_STATE  = 0x01,
	CONFIGURED_STATE   = 0x02,
	TRANSMITTING_STATE = 0x10,
	RECEIVING_STATE    = 0x20,
} fakeuart_state_e;

/*!
 * @brief This enum defines the PhyFake error
 */
typedef enum
{
	FAKEUART_ERR_NONE      = DEV_SUCCESS,
	FAKEUART_FAILURE       = DEV_FAILURE,
	FAKEUART_BUSY          = DEV_BUSY,
	FAKEUART_TIMEOUT       = DEV_TIMEOUT,
	FAKEUART_INVALID_PARAM = DEV_INVALID_PARAM,
} fakeuart_error_e;

/*!
 * @brief This struct defines PhyFake device context
 */
typedef struct
{
	uint8_t eState;
	uint8_t eError;
	uint16_t u16Len;
	uint8_t pBuf[BUF_SZ];
} fakeuart_device_t;

int32_t Phy_PhyFake_setup(phydev_t *pPhydev, fakeuart_device_t *pCtx);

#ifdef __cplusplus
}
#endif
#endif /* _PHY_LAYER_PRIVATE_H_ */

/*! @} */
/*! @} */
