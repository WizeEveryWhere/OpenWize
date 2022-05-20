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
 * @brief This enum define the sleep mode
 */
typedef enum {
	LP_SLEEP_MODE,  /*!< Sleep mode (CPU is sleeping) */
	LP_STOP1_MODE,  /*!< Stop 1 mode */
	LP_STOP2_MODE,  /*!< Stop 2 mode */
	LP_STDBY_MODE,  /*!< Standby mode */
	LP_SHTDWN_MODE, /*!< Shutdown mode */
} lp_mode_e;

void BSP_LowPower_Enter(lp_mode_e eLpMode);


#ifdef __cplusplus
}
#endif
#endif /* _BSP_LP_H_ */

/*! @} */
