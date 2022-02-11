/*!
  * @file: adm_mgr.h
  * @brief:This file declare functions to use the Administration session  Manager
  * 
  *****************************************************************************
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
  *****************************************************************************
  *
  * Revision history
  * ----------------
  * 1.0.0 : 2019/11/15 12:14:27 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Sources
 * @{
 * @ingroup Wize
 * @{
 * @ingroup AdmMgr
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
#undef RECV_BUFFER_SZ
#define RECV_BUFFER_SZ 229 // L7: 229; L6: 13 ; L2: 12

#undef SEND_BUFFER_SZ
#define SEND_BUFFER_SZ 229 // L7: 229; L6: 13 ; L2: 12

typedef enum
{
	ADM_RSP_NONE  = 0b00,
	ADM_RSP_PEND  = 0b01,
	ADM_RSP_READY = 0b10,
} adm_rsp_pend_e;

struct adm_mgr_ctx_s
{
	net_msg_t sDataMsg;
	net_msg_t sCmdMsg;
	net_msg_t sRspMsg;

	uint8_t aDataBuff[SEND_BUFFER_SZ];
    uint8_t aRecvBuff[RECV_BUFFER_SZ];
    uint8_t aSendBuff[SEND_BUFFER_SZ];

	uint8_t u8ExchRxDelay;      // [1s; 255s]
	uint8_t u8ExchRespDelay;    // [0s; 255s]
	uint8_t u8ExchRxLength;     // 0 : disable, otherwise [5ms; 1.27s]

	uint8_t u8ParamUpdate;
	uint8_t u8Pending;
	uint8_t u8ByPassCmd;

};

void AdmMgr_Setup(struct ses_ctx_s *pCtx);

#ifdef __cplusplus
}
#endif
#endif /* _ADM_MGR_H_ */

/*! @} */
/*! @} */
/*! @} */
