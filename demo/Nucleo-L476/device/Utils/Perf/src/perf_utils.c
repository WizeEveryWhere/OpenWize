/**
  * @file: perf_utils.c
  * @brief: This file implement the HW cycle counter read (if any)
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
  * 1.0.0 : 2019/12/25[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Sources
 * @{
 * @ingroup Libraries 
 * @{
 * @ingroup Perf
 * @{
 */

#include "perf_utils.h"


#if defined(__i386__)
/*!
 * @fn __inline__ void __cycle_counter_enable__(uint8_t en)
 * @brief Enable the cycle counter
 *
 * @param[in] en Enable : 1; Disable : 0
 */
__inline__ void __cycle_counter_enable__(uint8_t en){}

/*!
 * @fn ___inline__ uint32_t __cycle_counter_read__(void)
 * @brief Read the cycle counter
 *
 * @return The cycle counter current value
 */
__inline__ uint32_t __cycle_counter_read__(void)
{
unsigned long a, d;
__asm__ __volatile__ (
"rdtsc\n\t"
: "=a" (a), "=d" (d)
);
return (d << 32) | (a & 0xffffffff);
}
#elif defined(__x86_64__)
/*!
 * @fn __inline__ void __cycle_counter_enable__(uint8_t en)
 * @brief Enable the cycle counter
 *
 * @param[in] en Enable : 1; Disable : 0
 */
__inline__ void __cycle_counter_enable__(uint8_t en){}

/*!
 * @fn ___inline__ uint64_t __cycle_counter_read__(void)
 * @brief Read the cycle counter
 *
 * @return The cycle counter current value
 */
__inline__ uint64_t __cycle_counter_read__(void)
{
    uint32_t lo, hi;
    __asm__ __volatile__ (      // serialize
    "xorl %%eax,%%eax \n        cpuid"
    ::: "%rax", "%rbx", "%rcx", "%rdx");
    //** We cannot use "=A", since this would use %rax on x86_64 and return only the lower 32bits of the TSC
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return (uint64_t)hi << 32 | lo;
}
#elif defined(__arm__) //&& defined(__CORTEX_M)
#include <stm32l4xx.h>
/*!
 * @fn __inline__ void __cycle_counter_enable__(uint8_t en)
 * @brief Enable the cycle counter
 *
 * @param[in] en Enable : 1; Disable : 0
 */
__inline__ void __cycle_counter_enable__(uint8_t en){
	if ( ! ((DWT->CTRL) & DWT_CTRL_NOCYCCNT_Msk )) {
		if (en){ // enable counter
			DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
		}
		else{// disable counter
			DWT->CTRL &= ~(DWT_CTRL_CYCCNTENA_Msk);
		}
	}
	// else Counter is not supported
}

/*!
 * @fn ___inline__ uint32_t __cycle_counter_read__(void)
 * @brief Read the cycle counter
 *
 * @return The cycle counter current value
 */
__inline__ uint32_t __cycle_counter_read__(void){
	return DWT->CYCCNT;
}
#else
#warning "No cycle counter is found"
/*!
 * @fn __inline__ void __cycle_counter_enable__(uint8_t en)
 * @brief Enable the cycle counter
 *
 * @param[in] en Enable : 1; Disable : 0
 */
__inline__ void __cycle_counter_enable__(uint8_t en){}

/*!
 * @fn ___inline__ uint32_t __cycle_counter_read__(void)
 * @brief Read the cycle counter
 *
 * @return The cycle counter current value
 */
__inline__ uint32_t __cycle_counter_read__(void){return 0;}
#endif

/*! @} */
/*! @} */
/*! @} */
