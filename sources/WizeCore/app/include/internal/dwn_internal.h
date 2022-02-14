/**
  * @file: dwn_internal..h
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
  * 1.0.0 : 2020/10/11[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup WizeCore
 * @{
 *
 */
#ifndef _DWN_INTERNAL__H_
#define _DWN_INTERNAL__H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

struct ann_dwn_ctx_s
{
    uint8_t u8ChannelId;
	uint8_t u8ModulationId;
	uint16_t u16BlocksCount;
	uint8_t  u8DayRepeat;
	uint8_t  u8DeltaSec; // [ 5s @WM4800, 10s @WM2400; 255s]
	uint32_t u32DaysProg;
	uint32_t u32HashSW;
	uint8_t L7SwVersionTarget[2];
	uint8_t L7DwnId[3];

	// uint8_t L7MField[2];
	// uint8_t L7DcHwId[2];
	// uint8_t L7SwVersionIni[2];
	// uint8_t L7Klog[32];
};

#ifdef __cplusplus
}
#endif
#endif /* _DWN_INTERNAL__H_ */

/*! @} */
