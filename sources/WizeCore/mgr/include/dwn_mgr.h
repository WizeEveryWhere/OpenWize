/**
  * @file dwn_mgr.h
  * @brief This file declare functions to use the Download session Manager
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
  * 1.0.0 : 2020/09/04[GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup WizeCore
 * @{
 *
 */

#ifndef _DWN_MGR_H_
#define _DWN_MGR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "ses_common.h"

/******************************************************************************/
#undef RECV_BUFFER_SZ
#define RECV_BUFFER_SZ 210 // L7: 210; L6: 8 ; L2: 38

struct dwn_mgr_ctx_s
{
	net_msg_t sRecvMsg;

    uint8_t aRecvBuff[RECV_BUFFER_SZ]  __attribute__ ((aligned (8)));


    uint32_t u32DaysProg;
	uint8_t  u8DayRepeat;
	uint8_t  u8DeltaSec; // [ 5s @WM4800, 10s @WM2400; 255s]
	uint16_t u16BlocksCount;

	uint32_t u32HashSW;
    uint8_t L7SwVersionTarget[2];

    uint8_t u8ChannelId;
	uint8_t u8ModulationId;

	uint8_t L7DwnId[3];

    uint8_t u8DownRxLength;     // [0.5s/1s; 255s]


	// uint8_t L7MField[2];
	// uint8_t L7DcHwId[2];
	// uint8_t L7SwVersionIni[2];
	// uint8_t L7Klog[32];
	uint8_t u8ParamUpdate;
	uint8_t u8Pending;


	uint32_t u32InitDelayMin;

	uint32_t _u32DayNext;
	uint16_t _u16BlocksCount;
	uint8_t  _u8DayCount;


	uint32_t _u32NextBlkDelay;

};

void DwnMgr_Setup(struct ses_ctx_s *pCtx);

#ifdef __cplusplus
}
#endif
#endif /* _DWN_MGR_H_ */

/*! @} */
