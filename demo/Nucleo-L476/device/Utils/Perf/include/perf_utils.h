/**
  * @file: perf_utils.h
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

#ifndef _PERF_UTILS_H_
#define _PERF_UTILS_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#if defined(__i386__) || defined(__arm__)
void __cycle_counter_enable__(uint8_t en);
uint32_t __cycle_counter_read__(void);
#elif defined(__x86_64__)
uint64_t __cycle_counter_read__(void);
#else
#warning "No cycle counter is found"
uint32_t __cycle_counter_read__(void);
#endif

#ifdef __cplusplus
}
#endif
#endif /* _PERF_UTILS_H_ */

/*! @} */
/*! @} */
/*! @} */