/**
  * @file: inst_internal.c
  * @brief: This file implement the functions to treat the install L7
  * content.
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
#ifdef __cplusplus
extern "C" {
#endif

#include "inst_internal.h"

#include "parameters.h"
#include "parameters_lan_ids.h"

#include <stddef.h>
#include <string.h>
#include <time.h>
/*
 * Note : on ping_reply_list_t organization
 *
 *                  ___
 * pWorst   ]----->|   |
 *                 |_ _|<--
 *                   |    |
 *            pNext _v_   |
 *                 |   |  |
 *                 |___|  |
 *                   |    |
 *            pNext  v    |
 *                   :    :
 *                   :    :
 *                   |    |
 *            pNext _v_   |
 *                 |   |  |
 *                 |___|  |
 *                   |    |
 *            pNext _v_   |
 * pBest     ]---->|   |  |
 *                 |___|  |
 *                   |    |
 *            pNext  |____|
 *
 *
 */


/******************************************************************************/

void InstInt_Init(struct ping_reply_ctx_s *ping_reply_ctx, net_msg_t *pNetMsg)
{
	uint8_t idx;
	inst_ping_t *pInstPing = (inst_ping_t*)pNetMsg->pData;
	ping_reply_ctx->u8NbPong = 0;

	// set the first in table as the "best" one (arbitrary)
	ping_reply_ctx->pBest = &(ping_reply_ctx->aPingReplyList[0]);
	// set the last in table as the "worst" one (arbitrary)
	ping_reply_ctx->pWorst = &(ping_reply_ctx->aPingReplyList[NB_PING_REPLPY-1]);
	// link both
	ping_reply_ctx->pBest->pNext = ping_reply_ctx->pWorst;

	// link and initialize internal ones
	for (idx = 0; idx <  NB_PING_REPLPY; idx++)
	{
		// clear content values
		memset(&(ping_reply_ctx->aPingReplyList[idx].xPingReply), 0, 7);
		// set all to the "worst" value (0 represent the worst rssi -147.5 dBm )
		ping_reply_ctx->aPingReplyList[idx].xPingReply.L7RssiUpstream = 0;
		ping_reply_ctx->aPingReplyList[idx].xPingReply.L7RssiDownstream = 0;
		ping_reply_ctx->aPingReplyList[idx].u32RecvEpoch = 0;
		ping_reply_ctx->aPingReplyList[idx].u32PongEpoch = 0;
		ping_reply_ctx->aPingReplyList[idx].i16PongFreqOff = 0;

		if ( idx)
		{
			ping_reply_ctx->aPingReplyList[idx].pNext = &(ping_reply_ctx->aPingReplyList[idx-1]);
		}
	}
	// prepare the ping message
	//Param_Access(RF_UPLINK_CHANNEL, (uint8_t*)&(pInstPing->L7DownChannel), 0);
	//Param_Access(RF_UPLINK_MOD, (uint8_t*)&(pInstPing->L7DownMod), 0);
	Param_Access(RF_DOWNLINK_CHANNEL, (uint8_t*)&(pInstPing->L7DownChannel), 0);
	Param_Access(RF_DOWNLINK_MOD, (uint8_t*)&(pInstPing->L7DownMod), 0);
	Param_Access(PING_RX_DELAY, (uint8_t*)&(pInstPing->L7PingRxDelay), 0);
	Param_Access(PING_RX_LENGTH, (uint8_t*)&(pInstPing->L7PingRxLength), 0);
	pNetMsg->u8Size = 4;
	pNetMsg->u16Id++;
	pNetMsg->u8KeyId = 0;
	pNetMsg->u8Type = APP_INSTALL;

	time_t t;
	time(&t);
	ping_reply_ctx->u32PingEpoch = t - EPOCH_UNIX_TO_OURS;// TODO : time take stack but doesn't release it
}

uint8_t InstInt_End(struct ping_reply_ctx_s *ping_reply_ctx)
{
	int8_t idx;
	ping_reply_list_t *pCurrent = ping_reply_ctx->pWorst;
	if (ping_reply_ctx->u8NbPong > 0)
	{
		uint32_t tmp;
		uint32_t diff_time;
		if(ping_reply_ctx->pBest->xPingReply.L7RssiDownstream > ping_reply_ctx->sPingReplyConfig.AutoAdj_Rssi)
		{
			if (ping_reply_ctx->sPingReplyConfig.AutoClk)
			{
				// Adjust the Last Install request epoch to the new one
				diff_time = ping_reply_ctx->pBest->u32RecvEpoch - ping_reply_ctx->u32PingEpoch;
				ping_reply_ctx->u32PingEpoch = ping_reply_ctx->pBest->u32PongEpoch - diff_time;
				// Setup the CLOCK_CURRENT_EPOC
				tmp = __htonl(ping_reply_ctx->pBest->u32PongEpoch);
				Param_Access(CLOCK_CURRENT_EPOC, (uint8_t*)&( tmp ), 1);
			}
			if (ping_reply_ctx->sPingReplyConfig.AutoFreq)
			{
				tmp = (uint32_t)__htons(ping_reply_ctx->pBest->i16PongFreqOff);
				Param_Access(TX_FREQ_OFFSET, (uint8_t*)&( tmp ), 1);
			}
		}

		for (idx = (NB_PING_REPLPY-1); idx >= 0; idx--)
		{
			Param_Access(PING_REPLY1 + idx, (uint8_t*)&(pCurrent->xPingReply), 1);
			pCurrent = pCurrent->pNext;
		}

		tmp = __htonl(ping_reply_ctx->u32PingEpoch);
		Param_Access(PING_LAST_EPOCH, (uint8_t*)&(tmp), 1);
		Param_Access(PING_NBFOUND, (uint8_t*)&(ping_reply_ctx->u8NbPong), 1);
		return 0;
	}
	else
	{
		return 1;
	}
}

void InstInt_Add(struct ping_reply_ctx_s *ping_reply_ctx, net_msg_t *pxNetMsg)
{
	// warning RSSI => 0 : best; 255 : worst
	uint8_t idx;
	time_t t;
	ping_reply_list_t *pCurrent;
	ping_reply_list_t *pNew;

	// increment the number of received pong
	if (ping_reply_ctx->u8NbPong < 255) {
		ping_reply_ctx->u8NbPong++;
	}

	// check couple concentrator ID - MLAN ID
	for (idx = 0; idx < NB_PING_REPLPY; idx++)
	{
		if ( memcmp( (void*)&(ping_reply_ctx->aPingReplyList[idx].xPingReply), (void*)(pxNetMsg->pData), 7) == 0 )
		{
			// The couple Concentrator,MLAN is already in the list
			// ...should never happened : 1 pong per K-MLAN
			return;
		}
	}

	// set Current as the worst
	pCurrent = ping_reply_ctx->pWorst;

	// check if we must drop the new entry
	if (pxNetMsg->u8Rssi < pCurrent->xPingReply.L7RssiDownstream)
	{
		// the new entry RSSI is worst than worst, so drop it
		return;
	}

	// find the right "place/position" to insert the new entry
	while (pCurrent != ping_reply_ctx->pBest)
	{
		if (pxNetMsg->u8Rssi >= pCurrent->pNext->xPingReply.L7RssiDownstream)
		{
			// go to the next
			pCurrent = pCurrent->pNext;
		}
		else // pxNetMsg->u8Rssi > pCurrent->pNext->xPingReply.L7RssiDownstream
		{
			break;
		}
	}
	// prepare to take the container of the worst one
	pNew = ping_reply_ctx->pWorst;
	// replace the "worst" one
	ping_reply_ctx->pBest->pNext = ping_reply_ctx->pWorst->pNext;
	ping_reply_ctx->pWorst = ping_reply_ctx->pBest->pNext;

	// fill the new one
	memcpy(&(pNew->xPingReply), pxNetMsg->pData, 8);
	pNew->xPingReply.L7RssiDownstream = pxNetMsg->u8Rssi;
    time(&t);
	pNew->u32RecvEpoch = t - EPOCH_UNIX_TO_OURS;
	pNew->u32PongEpoch = pxNetMsg->u32Epoch;
	pNew->i16PongFreqOff = pxNetMsg->i16TxFreqOffset;

	// insert between pCurrent and pCurrent->pNext
	pNew->pNext = pCurrent->pNext;
	pCurrent->pNext = pNew;
    // check if it is the "new best"
	if (pCurrent == ping_reply_ctx->pBest)
	{
		ping_reply_ctx->pBest = pNew;
	}
}

#ifdef __cplusplus
}
#endif

/*! @} */
