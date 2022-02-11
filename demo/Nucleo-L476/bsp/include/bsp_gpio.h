/*!
  * @file: bsp_gpio.h
  * @brief: This file defines functions to deal with GPIOs.
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
  * 1.0.0 : 2019/12/14[GBI]
  * Initial version
  *
  *
  */
#ifndef _BSP_GPIO_H_
#define _BSP_GPIO_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#define GPIO_PIN(name) (uint16_t)name##_Pin
#define GPIO_PORT(name) (uint32_t)name##_GPIO_Port

uint8_t BSP_Gpio_InputEnable (const uint32_t u32Port, const uint16_t u16Pin, const uint8_t bFlag);
uint8_t BSP_Gpio_OutputEnable(const uint32_t u32Port, const uint16_t u16Pin, const uint8_t bFlag);

uint8_t BSP_Gpio_Get(const uint32_t u32Port, const uint16_t u16Pin, uint8_t *bLevel);
uint8_t BSP_Gpio_Set(const uint32_t u32Port, const uint16_t u16Pin, uint8_t bLevel);
uint8_t BSP_Gpio_SetLow (const uint32_t u32Port, const uint16_t u16Pin);
uint8_t BSP_Gpio_SetHigh (const uint32_t u32Port, const uint16_t u16Pin);

#ifdef __cplusplus
}
#endif
#endif /* _BSP_GPIO_H_ */
