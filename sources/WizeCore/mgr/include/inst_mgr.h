/**
  * @file inst_mgr.h
  * @brief This file declare functions to use the Install session Manager
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
  * @par 1.0.0 : 2020/09/04[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup wize_inst_mgr
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

/*!
 * @cond INTERNAL
 * @{
 */
#undef RECV_BUFFER_SZ
#define RECV_BUFFER_SZ 8 // L7: 8; L6:13 ; L2: 13

#undef SEND_BUFFER_SZ
#define SEND_BUFFER_SZ 4 // L7: 4; L6:13 ; L2: 13
/*!
 * @}
 * @endcond
 */

/*!
 * @brief This struct defines the install manager internal context.
 */
struct inst_mgr_ctx_s
{
	net_msg_t sCmdMsg;                 /**< Input message content */
	net_msg_t sRspMsg;                 /**< Output message content */

    uint8_t aRecvBuff[RECV_BUFFER_SZ]; /**< Buffer that hold the received frame */
    uint8_t aSendBuff[SEND_BUFFER_SZ]; /**< Buffer that hold the frame to send*/

    uint8_t u8InstRxLength;            /**< Install reception window length ([1s; 255s]) */
	uint8_t u8InstRxDelay;             /**< Opening reception window delay ([1s; 255s]) */

	uint8_t u8Pending;                 /**< Received message is pending  */
};

void InstMgr_Setup(struct ses_ctx_s *pCtx);

#ifdef __cplusplus
}
#endif
#endif /* _INST_MGR_H_ */

/*! @} */
