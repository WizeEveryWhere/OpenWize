/**
  * @file bsp_lptimer.h
  * @brief This file defines functions to deal with LPTim as simple timer.
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
  * @par 1.0.0 : 2020/08/29[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup lptimer
 * @ingroup bsp
 * @{
 */

#ifndef _BSP_LPTIMER_H_
#define _BSP_LPTIMER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

extern pfHandlerCB_t pfLptim1Event;
extern pfHandlerCB_t pfLptim2Event;

uint32_t BSP_LpTimer_Start(const uint8_t u8TimerId, uint32_t u32Elapse);
uint32_t BSP_LpTimer_Stop(const uint8_t u8TimerId);
void BSP_LpTimer_SetHandler (const uint8_t u8TimerId, pfHandlerCB_t const pfCb);


#ifdef __cplusplus
}
#endif
#endif /* _BSP_LPTIMER_H_ */

/*! @} */
