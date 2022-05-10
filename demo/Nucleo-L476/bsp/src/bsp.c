/**
  * @file bsp.c
  * @brief This file contains functions to initialize the BSP.
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
  * @par 1.0.0 : 2021/09/09 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup common
 * @ingroup bsp
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"
#include "platform.h"
#include <stdint.h>
#include <stm32l4xx_hal.h>

/******************************************************************************/
/* Usefull  */
/******************************************************************************/
/*!
  * @brief Function convert a hexa represented as 2 bytes char into hexa value (1 byte).
  *
  * @param [in] u16Char Two byte char to convert
  * 
  * @return converted one byte hex value
  */
uint8_t ascii2hex(uint16_t u16Char)
{
	register uint8_t t;
	register uint8_t hex;

	t = (uint8_t)(u16Char >> 8);
	t -= (t > 57)?(55):(48);
	hex = t << 4;

	t = (uint8_t)(u16Char & 0xFF);
	t -= (t > 57)?(55):(48);
	hex |= t & 0xF;

	return hex;
}

/*!
  * @brief Function convert a hexa value (1 byte) into its 2 bytes char representation.
  *
  * @param [in] u8Hex One byte hexa value to convert
  * 
  * @return converted two bytes char representation
  */
uint16_t hex2ascii(uint8_t u8Hex)
{
	uint8_t t;
	uint16_t c;

	t = ((u8Hex >> 4) & 0xF);
	t += (t > 9)?(55):(48);
	c = t << 8;

	t = (u8Hex & 0xF);
	t += (t > 9)?(55):(48);
	c |= t;

	return c;
}

/******************************************************************************/
/* Alias for HAL */
/******************************************************************************/
/*!
  * @brief Alias for HAL_Delay function
  *
  * @param [in] milisecond Number of milisecond to wait
  */
//inline __attribute__((always_inline))
void msleep(uint32_t milisecond) { HAL_Delay(milisecond); }

/******************************************************************************/
/* Libc print wrapper functions */
/******************************************************************************/

/*! @cond INTERNAL @{ */

extern uart_dev_t aDevUart[UART_ID_MAX];

#ifdef USE_SEMIHOSTING
#warning SEMIHOSTING is defined. 1) You should exclude "syscalls.c" from build.
#warning SEMIHOSTING is defined. 2) Add "rdimon" in link.
#warning SEMIHOSTING is defined. 2) Add "-specs=rdimon.specs" to compiler CFLAGS
#warning SEMIHOSTING is defined. 3) For debugging : Select OpenOCD
#warning SEMIHOSTING is defined. 3) For debugging : add "monitor arm semihosting enable" into "Startup", "Initalization Commands"

extern void initialise_monitor_handles(void);
#else
int __io_putchar(int ch){
	uint16_t nb = 1;
	if ((uint8_t)ch == '\n'){
		nb = 2;
		((uint8_t *)&ch)[1] = '\r';
	}
	HAL_UART_Transmit(aDevUart[STDOUT_UART_ID].hHandle, (uint8_t *)&ch, nb, CONSOLE_TX_TIMEOUT);
	return ch;
}

int __io_getchar(void){
	int c;
	HAL_UART_Receive(aDevUart[STDOUT_UART_ID].hHandle, (uint8_t*)&c, 1, CONSOLE_RX_TIMEOUT);
	return c;
}
#endif

extern void __init_exception_handlers__(void);
extern void __init_sys_handlers__(void);
extern void __init_sys_calls__(void);

/*! @} @endcond */


boot_state_t gBootState;

/*!
  * @brief This function initialize the bsp
  *
  * @param [in] u32BootState The current boot state
  * 
  * @return None
  */
void BSP_Init(uint32_t u32BootState)
{

	__init_exception_handlers__();
	__init_sys_handlers__();
	__init_sys_calls__();

	gBootState.state = u32BootState;

	// Setup the RTC
	//BSP_Rtc_Setup(255, 127);
	BSP_Rtc_Setup(RTC_PREDIV_S, RTC_PREDIV_A);

	// Check if required to init calendar
	if ( gBootState.state & CALENDAR_UNINIT_MSK )
	{
		//BSP_Rtc_Time_Write((time_t)EPOCH_UNIX_TO_OURS);
		BSP_Rtc_Time_Write((time_t)(1356998400U));
		gBootState.state &= ~(CALENDAR_UNINIT_MSK);
	}
}
#ifdef __cplusplus
}
#endif

/*! @} */
