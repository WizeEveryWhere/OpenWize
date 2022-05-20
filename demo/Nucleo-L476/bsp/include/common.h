/*!
  * @file common.h
  * @brief This file contains common definitions.
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
  * @par 1.0.0 : 2019/12/15 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup common
 * @ingroup bsp
 * @{
 */

#ifndef _COMMON_H_
#define _COMMON_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <time.h>
#include <machine/endian.h>

/*!
 * @cond INTERNAL
 * @{
 */

#define DEBUG_STDIO 1
#ifndef DEBUG_STDIO
#   define DEBUG_STDIO 0
#endif

#if DEBUG_STDIO
#   include <stdio.h>
#   define DBG(...) do { printf(__VA_ARGS__); } while(0)
#	if USE_DEBUG_TRACE_BSP
#   	define DBG_BSP(...) do { printf(__VA_ARGS__); } while(0)
#	else
#   	define DBG_BSP(...) {}
#	endif
#else
#   define DBG(...) {}
#endif

typedef void (*pfHandlerCB_t)(void);
typedef void (*pfEventCB_t)(void);

typedef void (*pfEvtCb_t)(void *p_CbParam,  uint32_t evt);

typedef void (*pf_cb_t)(void *p_CbParam, void *p_Arg);

#ifndef ENTER_CRITICAL_REGION
	#define ENTER_CRITICAL_REGION()
#endif
#ifndef EXIT_CRITICAL_REGION
	#define EXIT_CRITICAL_REGION()
#endif

/*!
 * @}
 * @endcond
 */

/*!
 * @brief This enum define the common return code from devices
 */
typedef enum
{
    DEV_SUCCESS      , /*!< Generic success */
    DEV_FAILURE      , /*!< Generic Failure */
	DEV_BUSY         , /*!< Device is Busy */
	DEV_TIMEOUT      , /*!< Device Timeout */
    DEV_INVALID_PARAM, /*!< Parameter is invalid */
} dev_res_e;

#ifdef __cplusplus
}
#endif
#endif /* _COMMON_H_ */

/*! @} */
