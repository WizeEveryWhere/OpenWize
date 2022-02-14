/**
  * @file inst_mgr.h
  * @brief This file declare functions to use the Install session Manager
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

#ifndef _INST_MGR_H_
#define _INST_MGR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include "ses_common.h"

/******************************************************************************/

#undef RECV_BUFFER_SZ
#define RECV_BUFFER_SZ 8 // L7: 8; L6:13 ; L2: 13

#undef SEND_BUFFER_SZ
#define SEND_BUFFER_SZ 4 // L7: 4; L6:13 ; L2: 13

struct inst_mgr_ctx_s
{
	net_msg_t sCmdMsg;
	net_msg_t sRspMsg;

    uint8_t aRecvBuff[RECV_BUFFER_SZ];
    uint8_t aSendBuff[SEND_BUFFER_SZ];

	uint8_t u8InstRxLength;    // [1s; ]
	uint8_t u8InstRxDelay;     // [1s; 255s]

	uint8_t u8ParamUpdate;
	uint8_t u8Pending;

};

void InstMgr_Setup(struct ses_ctx_s *pCtx);

#ifdef __cplusplus
}
#endif
#endif /* _INST_MGR_H_ */

/*! @} */
