/**
  * @file: phy_layer.h
  * @brief: This file define the available modulation, channel and power
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
  * 1.0.0 : 2020/10/13[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup WizeCore
 * @{
 *
 */
#ifndef _PHY_LAYER_H_
#define _PHY_LAYER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>

/*!
 * @brief This define the available channel
 */
typedef enum {
    PHY_CH100  = 0x0, /*!< 100+0*10 100 0x64 */
    PHY_CH110  = 0x1, /*!< 100+1*10 110 0x6E */
    PHY_CH120  = 0x2, /*!< 100+2*10 120 0x78 */
    PHY_CH130  = 0x3, /*!< 100+3*10 130 0x82 */
    PHY_CH140  = 0x4, /*!< 100+4*10 140 0x8C */
    PHY_CH150  = 0x5, /*!< 100+5*10 150 0x96 */
    //
	PHY_NB_CH,
} phy_chan_e;

/*!
 * @brief This define the available modulation
 */
typedef enum {
    PHY_WM2400  = 0x0,  /*!< WM2400 modulation */
    PHY_WM4800  = 0x1,  /*!< WM4800 modulation */
    PHY_WM6400  = 0x2,  /*!< WM6400 modulation - TX only */
	//
	PHY_NB_MOD,
} phy_mod_e;

/*!
 * @brief This define the available TX power
 */
typedef enum {
    PHY_PMAX_minus_0db  = 0, /*!< Maximum TX power */
    PHY_PMAX_minus_6db  = 1, /*!< Maximum TX power minus 6dB */
    PHY_PMAX_minus_12db = 2, /*!< Maximum TX power minus 12db */
	//
	PHY_NB_PWR,
} phy_power_e;

#ifdef __cplusplus
}
#endif
#endif /* _PHY_LAYER_H_ */

/*! @} */
