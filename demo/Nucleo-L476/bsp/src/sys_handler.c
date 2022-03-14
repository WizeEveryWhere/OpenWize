/**
  * @file sys_handler.c
  * @brief This file contains systems/platform handlers.
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
  * @par 1.0.0 : 2020/10/01 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup platform
 * @ingroup bsp
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <cmsis_compiler.h>
#include <stdio.h>

/*!
 * @cond INTERNAL
 * @{
 */

void __init_sys_handlers__(void)
{
	return;
}

#define HANDLER_SECTION(hsection) __attribute__(( section(hsection) ))

HANDLER_SECTION(".exception")
void NMI_Handler(void)  __attribute__((naked, noreturn));
HANDLER_SECTION(".sys")
void DebugMon_Handler(void) __attribute__((naked, noreturn));

#if defined (__OS__) && (__OS__ == Generic)
	HANDLER_SECTION(".sys")
	void SVC_Handler(void) __attribute__((weak, naked, noreturn));
	HANDLER_SECTION(".sys")
	void PendSV_Handler(void) __attribute__((weak, naked, noreturn));
	HANDLER_SECTION(".sys")
	void SysTick_Handler(void) __attribute__((weak));
#endif
/**
  * @brief This function handles Non mask-able interrupt.
  */
void NMI_Handler(void)
{
	while(1);
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{

}


#if defined (__OS__) && (__OS__ == Generic)
/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{

}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{

}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{

}
#endif

/*!
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

/*! @} */
