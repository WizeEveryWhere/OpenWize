/**
  * @file bsp_lp.h
 * @brief This file define the function to deal with low power
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
  * @par 1.0.0 : 2021/11/04 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup low_power
 * @ingroup bsp
 * @{
 */

#ifndef _BSP_LP_H_
#define _BSP_LP_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

/*!
 * @brief This enum define the low power mode
 */
typedef enum {
	LP_STOP0_MODE  = 0x0, /*!< Stop 0 mode */
	LP_STOP1_MODE  = 0x1, /*!< Stop 1 mode */
	LP_STOP2_MODE  = 0x2, /*!< Stop 2 mode */
	LP_STDBY_MODE  = 0x3, /*!< Standby mode */
	LP_SHTDWN_MODE = 0x4, /*!< Shutdown mode */
	// ---
	LP_SLEEP_MODE,  /*!< Sleep mode (CPU is sleeping) */
	// ---
	LP_NB_MODE,
} lp_mode_e;

/*!
 * @brief This enum define the wake-up pins
 */
typedef enum {
	LP_WAKEUP_PIN1_EN = 0b00001,  /*!< Wakeup pin 1 (PA0) */
	LP_WAKEUP_PIN2_EN = 0b00010,  /*!< Wakeup pin 2 (PC13) */
	LP_WAKEUP_PIN3_EN = 0b00100,  /*!< Wakeup pin 3 (PE6) */
	LP_WAKEUP_PIN4_EN = 0b01000,  /*!< Wakeup pin 4 (PA2) */
	LP_WAKEUP_PIN5_EN = 0b10000,  /*!< Wakeup pin 5 (PC5) */
} lp_wakeup_pin_en_msk;

/*!
 * @brief This enum define the wake-up pins polarity
 */
typedef enum {
	LP_WAKEUP_PIN1_POL_LOW = 0b00001,  /*!< Wakeup pin 1 falling edge */
	LP_WAKEUP_PIN2_POL_LOW = 0b00010,  /*!< Wakeup pin 2 falling edge */
	LP_WAKEUP_PIN3_POL_LOW = 0b00100,  /*!< Wakeup pin 3 falling edge */
	LP_WAKEUP_PIN4_POL_LOW = 0b01000,  /*!< Wakeup pin 4 falling edge */
	LP_WAKEUP_PIN5_POL_LOW = 0b10000,  /*!< Wakeup pin 5 falling edge */
} lp_wakeup_pin_poll_msk;

void BSP_LowPower_Enter(lp_mode_e eLpMode);

void BSP_LowPower_OnStandbyShutdwnEnter(lp_mode_e eLpMode);
void BSP_LowPower_OnStopEnter(lp_mode_e eLpMode);
void BSP_LowPower_OnStopExit(lp_mode_e eLpMode);

#ifdef __cplusplus
}
#endif
#endif /* _BSP_LP_H_ */

/*! @} */
