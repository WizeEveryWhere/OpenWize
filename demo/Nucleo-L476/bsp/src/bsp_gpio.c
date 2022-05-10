/*!
  * @file bsp_gpio.c
  * @brief This file contains functions to deal with GPIOs.
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
 * @addtogroup gpio
 * @ingroup bsp
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_gpio.h"
#include "platform.h"
#include <stm32l4xx_hal.h>

/*!
  * @brief This function set as input (or analog mode) the given gpio
  *
  * @param [in] u32Port Gpio port
  * @param [in] u16Pin  Gpio pin
  * @param [in] b_Flag  1 : set as input, 0: set in analog mode
  * 
  * @retval DEV_SUCCESS (see @link dev_res_e::DEV_SUCCESS @endlink)
  * 
  */
uint8_t BSP_Gpio_InputEnable (const uint32_t u32Port, const uint16_t u16Pin, const uint8_t b_Flag)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(b_Flag) {
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	}
	else {
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	}

	GPIO_InitStruct.Pin = u16Pin;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init((GPIO_TypeDef*)(u32Port), &GPIO_InitStruct);
	return DEV_SUCCESS;
}

/*!
  * @brief This function set as output (or analog mode) the given gpio
  *
  * @param [in] u32Port Gpio port
  * @param [in] u16Pin  Gpio pin
  * @param [in] b_Flag  1 : set as output, 0: set in analog mode
  * 
  * @retval DEV_SUCCESS (see @link dev_res_e::DEV_SUCCESS @endlink)
  * 
  */
uint8_t BSP_Gpio_OutputEnable(const uint32_t u32Port, const uint16_t u16Pin, const uint8_t b_Flag)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(b_Flag) {
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	}
	else {
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	}

	GPIO_InitStruct.Pin = u16Pin;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init((GPIO_TypeDef*)(u32Port), &GPIO_InitStruct);
	return DEV_SUCCESS;
}

/*!
  * @brief This function get the value of the given gpio
  *
  * @param [in]     u32Port Gpio port
  * @param [in]     u16Pin  Gpio pin
  * @param [in,out] b_Level  Will hold the gpio current value
  * 
  * @retval DEV_SUCCESS (see @link dev_res_e::DEV_SUCCESS @endlink)
  * 
  */
inline uint8_t BSP_Gpio_Get(const uint32_t u32Port, const uint16_t u16Pin, uint8_t *b_Level)
{
	*b_Level = (uint8_t)HAL_GPIO_ReadPin((GPIO_TypeDef*)(u32Port), u16Pin);
	return DEV_SUCCESS;
}

/*!
  * @brief This function set the value of the given gpio
  *
  * @param [in] u32Port Gpio port
  * @param [in] u16Pin  Gpio pin
  * @param [in] b_Level The value to set
  * 
  * @retval DEV_SUCCESS (see @link dev_res_e::DEV_SUCCESS @endlink)
  * 
  */
inline uint8_t BSP_Gpio_Set(const uint32_t u32Port, const uint16_t u16Pin, uint8_t b_Level)
{
	HAL_GPIO_WritePin((GPIO_TypeDef*)(u32Port), u16Pin, (GPIO_PinState)b_Level);
	return DEV_SUCCESS;
}

/*!
  * @brief This function set to 0 the given gpio
  *
  * @param [in] u32Port Gpio port
  * @param [in] u16Pin  Gpio pin
  * 
  * @retval DEV_SUCCESS (see @link dev_res_e::DEV_SUCCESS @endlink)
  * 
  */
inline uint8_t BSP_Gpio_SetLow (const uint32_t u32Port, const uint16_t u16Pin)
{
#if defined(USE_FULL_LL_DRIVER)
	LL_GPIO_ResetOutputPin((GPIO_TypeDef *)u32Port, u16Pin);
	return DEV_SUCCESS;
#else
	return BSP_Gpio_Set(u32Port, u16Pin, GPIO_PIN_RESET);
#endif

}

/*!
  * @brief This function set to 1 the given gpio
  *
  * @param [in] u32Port Gpio port
  * @param [in] u16Pin  Gpio pin
  * 
  * @retval DEV_SUCCESS (see @link dev_res_e::DEV_SUCCESS @endlink)
  * 
  */
inline uint8_t BSP_Gpio_SetHigh (const uint32_t u32Port, const uint16_t u16Pin)
{
#if defined(USE_FULL_LL_DRIVER)
		LL_GPIO_SetOutputPin((GPIO_TypeDef *)u32Port, u16Pin);
		return DEV_SUCCESS;
#else
	return BSP_Gpio_Set(u32Port, u16Pin, GPIO_PIN_SET);
#endif
}

#ifdef __cplusplus
}
#endif

/*! @} */
