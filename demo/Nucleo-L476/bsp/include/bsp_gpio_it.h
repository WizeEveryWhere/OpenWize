/*!
  * @file bsp_gpio_it.h
  * @brief This file defines functions to deal with external interrupt lines.
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
  * @par 1.0.0 : 2019/12/14 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup gpio_it
 * @ingroup bsp
 * @{
 */

#ifndef _BSP_GPIO_IT_H_
#define _BSP_GPIO_IT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "bsp_gpio.h"

/** This enum define the GPIO trigger condition */
typedef enum {
	GPIO_IRQ_NONE_EDGE      =(0b0000),     /*!< Trigger an interrupt on a none edge.    */
    GPIO_IRQ_RISING_EDGE    =(0b0001),     /*!< Trigger an interrupt on a rising edge.    */
    GPIO_IRQ_FALLING_EDGE   =(0b0010),     /*!< Trigger an interrupt on a falling edge.   */
    GPIO_IRQ_EITHER_EDGE    =(0b0011),     /*!< Trigger an interrupt on either edge.      */
    GPIO_IRQ_HIGH_LEVEL     =(0b0100),     /*!< Trigger an interrupt on a high level.     */
    GPIO_IRQ_LOW_LEVEL      =(0b1000),     /*!< Trigger an interrupt on a low level.      */
	GPIO_IRQ_EITHERLEVEL    =(0b1100)      /*!< Trigger an interrupt on a low level.      */
} gpio_irq_trg_cond_e;

int8_t BSP_GpioIt_GetLineId(const uint16_t u16Pin);
uint8_t BSP_GpioIt_ConfigLine (const uint32_t u32Port, const uint16_t u16Pin, const gpio_irq_trg_cond_e ePol);
uint8_t BSP_GpioIt_SetLine (const uint32_t u32Port, const uint16_t u16Pin, const bool bEnable);
uint8_t BSP_GpioIt_SetCallback (const uint32_t u32Port, const uint16_t u16Pin, pf_cb_t const pfCb, void *const pCbParam );
uint8_t BSP_GpioIt_SetGpioCpy( const uint8_t u8ItLineId, const uint32_t u32Port, const uint16_t u16Pin);
uint8_t BSP_GpioIt_ClrGpioCpy( const uint8_t u8ItLineId);
void BSP_GpioIt_Handler(int8_t i8_ItLineId);

#ifdef __cplusplus
}
#endif
#endif /* _BSP_GPIO_IT_H_ */

/*! @} */
