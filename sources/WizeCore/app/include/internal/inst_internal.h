/**
  * @file: inst_internal.h
  * @brief: This file define the functions and structures to treat the install L7
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
#ifndef _INST_INTERNAL_H_
#define _INST_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "proto.h"
#include "app_layer.h"

#include <stdint.h>

#define NB_PING_REPLPY 8

struct ping_reply_config_s{
	union {
		uint8_t AutoAdj_ClkFreq;     /*!< Clock and Frequency Offset Auto-Adjustment */
		struct
		{
			uint8_t AutoClk:1;  /*!< Coarse clock auto-adjust on PONG. 1: enable, 0: disable */
			uint8_t AutoFreq:1; /*!< Frequency Offset auto-adjust on PONG. 1: enable, 0: disable */
		} ;
	};
	uint8_t AutoAdj_Rssi;   /*!< Clock and Frequency Offset Auto-Adjustment received frame RSSI min.*/
};

typedef struct ping_reply_list_s{
	struct ping_reply_list_s *pNext;
	inst_pong_t xPingReply;
	uint32_t u32RecvEpoch;
	uint32_t u32PongEpoch;
	int16_t i16PongFreqOff;
} ping_reply_list_t;

struct ping_reply_ctx_s
{
	ping_reply_list_t aPingReplyList[NB_PING_REPLPY];
	ping_reply_list_t *pWorst;
	ping_reply_list_t *pBest;
	uint32_t u32PingEpoch;
	uint8_t u8NbPong;
	struct ping_reply_config_s sPingReplyConfig;
};

void InstInt_Init(struct ping_reply_ctx_s *ping_reply_ctx, net_msg_t *pNetMsg);
uint8_t InstInt_End(struct ping_reply_ctx_s *ping_reply_ctx);
void InstInt_Add(struct ping_reply_ctx_s *ping_reply_ctx, net_msg_t *pxNetMsg);

#ifdef __cplusplus
}
#endif
#endif /* _INST_INTERNAL_H_ */

/*! @} */
