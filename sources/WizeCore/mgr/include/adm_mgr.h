/*!
  * @file: adm_mgr.h
  * @brief This file declare functions to use the Administration session  Manager
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
  * @par 1.0.0 : 2019/11/15 12:14:27 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup wize_admin_mgr
 * @{
 *
 */

#ifndef _ADM_MGR_H_
#define _ADM_MGR_H_

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
#define RECV_BUFFER_SZ 229 // L7: 229; L6: 13 ; L2: 13

#undef SEND_BUFFER_SZ
#define SEND_BUFFER_SZ 229 // L7: 229; L6: 13 ; L2: 13

typedef enum
{
	ADM_RSP_NONE  = 0b00,
	ADM_RSP_PEND  = 0b01,
	ADM_RSP_READY = 0b10,
} adm_rsp_pend_e;

/*!
 * @}
 * @endcond
 */

/*!
 * @brief This struct defines the admin manager internal context.
 */
struct adm_mgr_ctx_s
{
	net_msg_t sDataMsg;                /**< Data message content */
	net_msg_t sCmdMsg;                 /**< Command message content */
	net_msg_t sRspMsg;                 /**< Response message content */

	uint8_t aDataBuff[SEND_BUFFER_SZ]; /**< Buffer that hold the data frame to send*/
    uint8_t aRecvBuff[RECV_BUFFER_SZ]; /**< Buffer that hold the received command frame */
    uint8_t aSendBuff[SEND_BUFFER_SZ]; /**< Buffer that hold the response frame to send*/

	uint8_t u8ExchRxLength;            /**< Admin reception window length (0 : disable, otherwise [5ms; 1.27s]) */
	uint8_t u8ExchRxDelay;     		   /**< Opening reception window delay ([1s; 255s]) */
	uint8_t u8ExchRespDelay;    	   /**< Admin response delay ([0s; 255s]) */

	uint8_t u8Pending;                 /**< Received command is pending */
	uint8_t u8ByPassCmd;               /**< Bypass the Received command */

    int16_t i16DeltaRxMs;              /**< Offset to listening window (in millisecond) */
};

void AdmMgr_Setup(struct ses_ctx_s *pCtx);

#ifdef __cplusplus
}
#endif
#endif /* _ADM_MGR_H_ */

/*! @} */
