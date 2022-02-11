/**
  * @file: bsp_lp.h
  * @brief: // TODO This file ...
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
  * 1.0.0 : 2021/11/04[GBI]
  * Initial version
  *
  *
  */
#ifndef _BSP_LP_H_
#define _BSP_LP_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef enum {
	LP_SLEEP_MODE,
	LP_STOP1_MODE,
	LP_STOP2_MODE,
	LP_STDBY_MODE,
	LP_SHTDWN_MODE,
} lp_mode_e;

void BSP_LowPower_Enter(lp_mode_e eLpMode);


#ifdef __cplusplus
}
#endif
#endif /* _BSP_LP_H_ */
