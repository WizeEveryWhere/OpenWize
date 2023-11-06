/**
  * @file inst_internal.c
  * @brief This file implement the functions to treat the install L7
  * content.
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
#ifdef __cplusplus
extern "C" {
#endif

#include "inst_internal.h"

#include "parameters.h"
#include "parameters_lan_ids.h"

#include <stddef.h>
#include <string.h>
#include <machine/endian.h>

// FIXME
#ifdef HAS_HIRES_TIME_MEAS
	#define CAPTURE_ID_RX_COMPLETE 4

	extern int32_t BSP_HiResTmr_EnDis(uint8_t bEnable);
	extern void BSP_HiResTmr_Capture(register uint8_t id);
	extern uint32_t BSP_HiResTmr_Get(register uint8_t id);

	#define INST_INT_TMR_ENABLE() BSP_HiResTmr_EnDis(1);
    #define INST_INT_TMR_DISABLE() BSP_HiResTmr_EnDis(0);
	#define INST_INT_TMR_GET_PONG_COMPLETE() BSP_HiResTmr_Get(CAPTURE_ID_RX_COMPLETE)
	#define INST_INT_TMR_GET() BSP_HiResTmr_Get(1)
	#define INST_INT_TMR_CAPTURE() BSP_HiResTmr_Capture(1)
#endif


/*
 * Note : The ping_reply_list_t is a linked list organization as follow :
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
 * - It can accept at most "NB_PING_REPLPY" pong values
 * - Pointer "pWorst" and "pBest" gives respectively the worst and best pong in term of received RSSI
 * - If the couple (Gateway, MLAN id) is already in the list, this pong is discarded
 */

/*!
 * @cond INTERNAL
 * @{
 */

static void _get_adjust_clock_offset_(struct ping_reply_ctx_s *ping_reply_ctx);
static uint32_t _error_estimator_(ping_reply_t *pPingReply);

#ifndef RSSI_REVERSED
/*
 * RSSI
 *   0 represent the worst one : -147.5 dBm
 * 255 represent the best one  : -20 dBm
 */
#define BEST_RSSI  255
#define WORST_RSSI 0
#define IS_WORST(rssi_1, rssi_2) (rssi_1 < rssi_2)
#define IS_BEST_EQUAL(rssi_1, rssi_2) (rssi_1 >= rssi_2)

#else
/*
 * RSSI
 * 255 represent the worst one : -147.5 dBm
 *   0 represent the best one  : -20 dBm
 */
#define BEST_RSSI  0
#define WORST_RSSI 255
#define IS_WORST(rssi_1, rssi_2) (rssi_1 > rssi_2)
#define IS_BEST_EQUAL(rssi_1, rssi_2) (rssi_1 <= rssi_2)

#endif
/*!
 * @}
 * @endcond
 */
/******************************************************************************/

/*!
  * @brief This function initialize the ping_reply context
  *
  * @param [in,out] ping_reply_ctx Pointer on the current context
  *
  * @return  The inst_ping message to send
  */
inst_ping_t InstInt_Init(struct ping_reply_ctx_s *ping_reply_ctx)
{
	uint8_t idx;
	inst_ping_t sInstPing;
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
		memset(&(ping_reply_ctx->aPingReplyList[idx].xPingReply), 0, sizeof(ping_reply_t));

		// set all to the "worst" value
		ping_reply_ctx->aPingReplyList[idx].xPingReply.RssiUpstream = WORST_RSSI;
		ping_reply_ctx->aPingReplyList[idx].xPingReply.RssiDownstream = WORST_RSSI;
		if ( idx)
		{
			ping_reply_ctx->aPingReplyList[idx].pNext = &(ping_reply_ctx->aPingReplyList[idx-1]);
		}
	}
	// prepare the ping message
	Param_Access(RF_DOWNLINK_CHANNEL, (uint8_t*)&(sInstPing.L7DownChannel), 0);
	Param_Access(RF_DOWNLINK_MOD, (uint8_t*)&(sInstPing.L7DownMod), 0);
	Param_Access(PING_RX_DELAY, (uint8_t*)&(sInstPing.L7PingRxDelay), 0);
	Param_Access(PING_RX_LENGTH, (uint8_t*)&(sInstPing.L7PingRxLength), 0);

	gettimeofday(&(ping_reply_ctx->tmPingEpoch), NULL);

#ifdef HAS_HIRES_TIME_MEAS
	INST_INT_TMR_ENABLE();
#endif
	return sInstPing;
}

/*!
  * @brief This function terminate the ping_reply (aka. write ping_reply values
  * into the parameters table).
  *
  * @param [in,out] ping_reply_ctx Pointer on the current context
  *
  * @return  The number of received pong
  */
uint8_t InstInt_End(struct ping_reply_ctx_s *ping_reply_ctx)
{
	int8_t idx;
	ping_reply_list_t *pCurrent = ping_reply_ctx->pWorst;
	if (ping_reply_ctx->u8NbPong > 0)
	{
		uint32_t tmp;
		for (idx = (NB_PING_REPLPY-1); idx >= 0; idx--)
		{
			Param_Access(PING_REPLY1 + idx, (uint8_t*)&(pCurrent->xPingReply), 1);
			pCurrent = pCurrent->pNext;
		}

		tmp = __htonl( ping_reply_ctx->tmPingEpoch.tv_sec - EPOCH_UNIX_TO_OURS );
		Param_Access(PING_LAST_EPOCH, (uint8_t*)&(tmp), 1);
		Param_Access(PING_NBFOUND, (uint8_t*)&(ping_reply_ctx->u8NbPong), 1);

		/*
		 *  Maybe we should add some microsecond or millisecond to take into
		 *  account the computation time of "_get_adjust_clock_offset_"
		*/
		gettimeofday(&(ping_reply_ctx->sEpochErr.tmFrom), NULL);
		_get_adjust_clock_offset_(ping_reply_ctx);
	}
#ifdef HAS_HIRES_TIME_MEAS
	INST_INT_TMR_DISABLE();
#endif
	return ping_reply_ctx->u8NbPong;
}

/*!
  * @brief This function fill the "ping_reply" with content of passed net_msg.
  *
  * @param [out] pPingReply Pointer on the ping_reply to be filled
  * @param [in]  pNetMsg    Pointer on received pong message
  *
  * @return  None
  */
void InstInt_Fill(ping_reply_t *pPingReply, net_msg_t *pNetMsg)
{
	if (pPingReply && pNetMsg)
	{
		memcpy(pPingReply, pNetMsg->pData, 8);
		pPingReply->RssiDownstream = pNetMsg->u8Rssi;
		pPingReply->u32PongEpoch = pNetMsg->u32Epoch;
		pPingReply->i16PongFreqOff = pNetMsg->i16TxFreqOffset;
		// Get the received time
	#ifndef HAS_HIRES_TIME_MEAS
		gettimeofday(&(pPingReply->tmRecvEpoch), NULL);
	#else
		pPingReply->tmRecvEpoch.tv_sec = 0;
		pPingReply->tmRecvEpoch.tv_usec = INST_INT_TMR_GET_PONG_COMPLETE();
	#endif
	}
}

/*!
  * @brief This function insert a pong to the list of "ping_reply"
  *
  * @param [in,out] ping_reply_ctx Pointer on the current context
  * @param [in,out] pPingReply     Pointer on received ping_reply message
  *
  * @return  0 if the ping_reply was inserted, 1 otherwise
  */
uint8_t InstInt_Insert(struct ping_reply_ctx_s *ping_reply_ctx, ping_reply_t *pPingReply)
{
	ping_reply_list_t *pCurrent;
	ping_reply_list_t *pNew;
	uint8_t idx;

	if (pPingReply == NULL)
	{
		return 1;
	}

	// increment the number of received pong
	if (ping_reply_ctx->u8NbPong < 255) {
		ping_reply_ctx->u8NbPong++;
	}

	// check couple concentrator ID - MLAN ID
	for (idx = 0; idx < NB_PING_REPLPY; idx++)
	{
		if ( memcmp( (void*)&(ping_reply_ctx->aPingReplyList[idx].xPingReply), (void*)(pPingReply), 7) == 0 )
		{
			// The couple Concentrator,MLAN is already in the list
			// ...should never happened : 1 pong per Gateway-MLAN
			return 1;
		}
	}

	// set Current as the worst
	pCurrent = ping_reply_ctx->pWorst;

	// check if we must drop the new entry
	if ( IS_WORST(pPingReply->RssiDownstream, pCurrent->xPingReply.RssiDownstream) )
	{
		// the new entry RSSI is worst than worst, so drop it
		return 1;
	}

	// find the right "place/position" to insert the new entry
	while (pCurrent != ping_reply_ctx->pBest)
	{
		if ( IS_BEST_EQUAL(pPingReply->RssiDownstream, pCurrent->pNext->xPingReply.RssiDownstream) )
		{
			// go to the next
			pCurrent = pCurrent->pNext;
		}
		else
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
	memcpy(&(pNew->xPingReply), pPingReply, sizeof(ping_reply_t));

	// insert between pCurrent and pCurrent->pNext
	pNew->pNext = pCurrent->pNext;
	pCurrent->pNext = pNew;
    // check if it is the "new best"
	if (pCurrent == ping_reply_ctx->pBest)
	{
		ping_reply_ctx->pBest = pNew;
	}
	return 0;
}

/*!
  * @brief This function add a pong to the "ping_reply"
  *
  * @param [in,out] ping_reply_ctx Pointer on the current context
  * @param [in,out] pNetMsg        Pointer on received pong message
  *
  * @return  None
  */
void InstInt_Add(struct ping_reply_ctx_s *ping_reply_ctx, net_msg_t *pNetMsg)
{
	ping_reply_list_t *pCurrent;
	ping_reply_list_t *pNew;
	uint8_t idx;

	// increment the number of received pong
	if (ping_reply_ctx->u8NbPong < 255) {
		ping_reply_ctx->u8NbPong++;
	}

	// check couple concentrator ID - MLAN ID
	for (idx = 0; idx < NB_PING_REPLPY; idx++)
	{
		if ( memcmp( (void*)&(ping_reply_ctx->aPingReplyList[idx].xPingReply), (void*)(pNetMsg->pData), 7) == 0 )
		{
			// The couple Concentrator,MLAN is already in the list
			// ...should never happened : 1 pong per Gateway-MLAN
			return;
		}
	}

	// set Current as the worst
	pCurrent = ping_reply_ctx->pWorst;

	// check if we must drop the new entry
	if ( IS_WORST(pNetMsg->u8Rssi, pCurrent->xPingReply.RssiDownstream) )
	{
		// the new entry RSSI is worst than worst, so drop it
		return;
	}

	// find the right "place/position" to insert the new entry
	while (pCurrent != ping_reply_ctx->pBest)
	{
		if ( IS_BEST_EQUAL(pNetMsg->u8Rssi, pCurrent->pNext->xPingReply.RssiDownstream) )
		{
			// go to the next
			pCurrent = pCurrent->pNext;
		}
		else
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
	memcpy(&(pNew->xPingReply), pNetMsg->pData, 8);
	pNew->xPingReply.RssiDownstream = pNetMsg->u8Rssi;
	pNew->xPingReply.u32PongEpoch = pNetMsg->u32Epoch;
	pNew->xPingReply.i16PongFreqOff = pNetMsg->i16TxFreqOffset;

	// Get the received time
#ifndef HAS_HIRES_TIME_MEAS
	gettimeofday(&(pNew->xPingReply.tmRecvEpoch), NULL);
#else
	pNew->xPingReply.tmRecvEpoch.tv_usec = INST_INT_TMR_GET_PONG_COMPLETE();
#endif
	// insert between pCurrent and pCurrent->pNext
	pNew->pNext = pCurrent->pNext;
	pCurrent->pNext = pNew;
    // check if it is the "new best"
	if (pCurrent == ping_reply_ctx->pBest)
	{
		ping_reply_ctx->pBest = pNew;
	}
}

/*!
 * @static
 * @brief This function compute the offset (correction) to be applied to the
 *        new clock received from a Pong frame. The correction to be applied is
 *        store in ping_reply_ctx->sEpochErr.tmErr as second and microsecond.
 *
 * @details It compute the difference between time "now" and the pong frame
 *          reception time. Furthermore, it take into account the pong frame
 *          duration (126.667 ms)
 *          If ping_reply_ctx->bGwErrCorr is not 0, then the error due to the
 *          gw is also add to the correction.
 *
 * @param [in,out] ping_reply_ctx Pointer on the current context
 *
 */
static
void _get_adjust_clock_offset_(struct ping_reply_ctx_s *ping_reply_ctx)
{
	struct timeval diff;
	ping_reply_t *pxPingReply = &(ping_reply_ctx->pBest->xPingReply);
	// Add error due to the gw
	uint32_t error_us = 0;

	// If this bit is set, then apply the correction due to gateway error ("alea")
	// Note that this "alea" can, in terms of mathematics, be between 0 and 999999 uS.
	if (ping_reply_ctx->bGwErrCorr)
	{
		/*
		 * Case of testing with the TRX (SmartBrick) which doesn't implement the
		 * same algorithm as the gateway to distribute the PONG message over the
		 * RX window.
		 * If received gateway has a serial number starting with 0x99, then we
		 * consider it as a TRX.
		 */
		if (pxPingReply->GatewayId[0] != 0x99)
		{
			error_us = _error_estimator_(pxPingReply);
		}
	}

#ifdef HAS_HIRES_TIME_MEAS
	// Get microsecond now
	INST_INT_TMR_CAPTURE();
	// Compute delta between "now" and the best Pong EOF
	uint32_t delta = INST_INT_TMR_GET() - pxPingReply->tmRecvEpoch.tv_usec;
	diff.tv_sec = delta / 1000000;
	diff.tv_usec = delta % 1000000;

#else
	struct timeval now;
	gettimeofday(&now, NULL);

	// diff.tv_sec if ever >= 0
	diff.tv_sec = now.tv_sec - pxPingReply->tmRecvEpoch.tv_sec;

	// diff.tv_usec could be <= 0
	if (now.tv_usec > pxPingReply->tmRecvEpoch.tv_usec)
	{
		diff.tv_usec = now.tv_usec - pxPingReply->tmRecvEpoch.tv_usec;
	}
	else
	{
		diff.tv_usec = pxPingReply->tmRecvEpoch.tv_usec - now.tv_usec;
		diff.tv_usec = 1000000 - diff.tv_usec;
	}
#endif
	// Add error due to the gw
	diff.tv_usec += error_us;

	// take into account the PONG frame duration
	// TODO : add treat for other bitrate
	/*
	 * 4 (L1) + 13 (L2) + 13 (L6) + 8 (L7) = 38 bytes = 304 bits
	 * @2400 bit/s --> 126.667 ms
	*/
	diff.tv_usec += 126667;

	// Compute the new values
	diff.tv_sec += (diff.tv_usec / 1000000);
	diff.tv_usec %= 1000000;

	ping_reply_ctx->sEpochErr.tmErr = diff;
}

static uint32_t _error_estimator_(ping_reply_t *pPingReply)
{
#define GW_INSTPONG_DURATION_MS_MAX (140)
	uint32_t error_us;
	uint32_t gw_slot_assigned;
	uint32_t u32RxLenMs;
	char *p_gw_id;

	uint16_t a_gw_id[7];
	uint16_t hash_gw = 0;
	uint8_t v;

	for (v = 0; v < 6; v++)
	{
		a_gw_id[v] =  ( (pPingReply->GatewayId[v] & 0xF) + 0x30) << 8 ;
		a_gw_id[v] |= ( (pPingReply->GatewayId[v] >> 4) + 0x30 );
	}
	a_gw_id[6] = 0;

	// compute the gw hash based on its serial number as ascii (0-9) string (12 digit)
	p_gw_id = (char *)a_gw_id;
	for(; *p_gw_id; ++p_gw_id)
	{
		hash_gw += *p_gw_id;
		hash_gw += (hash_gw << 9);
		hash_gw ^= (hash_gw >> 6);
	}
	hash_gw += (hash_gw << 3);
	hash_gw ^= (hash_gw >> 11);
	hash_gw += (hash_gw << 15);

	// get the window duration
	Param_Access(PING_RX_LENGTH, (uint8_t*)&(v), 0);
	u32RxLenMs = v * 1000;

	// estimate the gw used slot
	gw_slot_assigned = (hash_gw + ((pPingReply->ModemId - 1) * (u32RxLenMs / 4)) ) % (u32RxLenMs - GW_INSTPONG_DURATION_MS_MAX);

	// get error in uS
	error_us = (gw_slot_assigned % 1000) * 1000;

	return error_us;
}

#ifdef __cplusplus
}
#endif

/*! @} */
