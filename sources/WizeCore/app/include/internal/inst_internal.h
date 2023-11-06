/**
  * @file: inst_internal.h
  * @brief This file define the functions and structures to treat the install L7
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
  * @par 1.0.0 : 2020/10/11[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup wize_inst_layer
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
#include <time.h>
#include <sys/time.h>

/*!
 * @cond INTERNAL
 * @{
 */

#define NB_PING_REPLPY 8

/*!
 * @}
 * @endcond
 */

/*!
 * @brief This struct defines the ping_reply element
 */
typedef struct {
	uint8_t GatewayId[6];       /**< The gateway Id (big endian) */
	uint8_t ModemId;            /**< The modem Id */
	uint8_t RssiUpstream;       /**< The upstream RSSI */
	uint8_t RssiDownstream;     /**< The downstream RSSI (of the received frame) */
	// ---
	int16_t i16PongFreqOff;     /*!< Pong content Frequency offset */
	uint32_t u32PongEpoch;      /*!< Pong content Epoch (relative to 2013-01-01_00:00:00*/
	struct timeval tmRecvEpoch; /*!< Epoch of received Pong (relative to local time) (in ms or us for HiResTimer)*/
} ping_reply_t;
/*!
 * @brief This struct defines the ping_reply element list.
 */
typedef struct ping_reply_list_s{
	struct ping_reply_list_s *pNext; /*!< Pointer on the next element */
	ping_reply_t xPingReply;         /*!< Content values of the received pong */
} ping_reply_list_t;

typedef struct {
	struct timeval tmErr;            /*!< Error second and microsecond to be applied to the current time/epoch */
	struct timeval tmFrom;           /*!< Current time at which the correction has been computed */
} epoch_error_t;

/*!
 * @brief This struct defines the ping_reply context.
 */
struct ping_reply_ctx_s
{
	ping_reply_list_t aPingReplyList[NB_PING_REPLPY]; /*!< Table of received Pong */
	ping_reply_list_t *pWorst;                        /*!< Pointer on the worst RSSI */
	ping_reply_list_t *pBest;                         /*!< Pointer on the best RSSI */
	epoch_error_t sEpochErr;                          /*!< Epoch error correction to be applied */
	struct timeval tmPingEpoch;                              /*!< Epoch of sent Ping (relative to local time) */
	uint8_t bGwErrCorr;                               /*!< Enable (1 / Disable (0) the correction of gateway error */
	uint8_t u8NbPong;                                 /*!< Number of received Pong */
};

inst_ping_t InstInt_Init(struct ping_reply_ctx_s *ping_reply_ctx);
uint8_t InstInt_End(struct ping_reply_ctx_s *ping_reply_ctx);

void InstInt_Fill(ping_reply_t *pPingReply, net_msg_t *pNetMsg);
uint8_t InstInt_Insert(struct ping_reply_ctx_s *ping_reply_ctx, ping_reply_t *pPingReply);

void InstInt_Add(struct ping_reply_ctx_s *ping_reply_ctx, net_msg_t *pxNetMsg);

#ifdef __cplusplus
}
#endif
#endif /* _INST_INTERNAL_H_ */

/*! @} */
