#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <machine/endian.h>

TEST_GROUP(WizeCoreAppInt_inst);
#include "inst_internal.h"
#include "adm_internal.h"
#include "parameters_lan_ids.h"
#include "phy_layer.h"

// Mock Samples
#include "mock_parameters.h"

/******************************************************************************/
struct ping_reply_ctx_s sPingReplyCtx;

/******************************************************************************/

#define DEF_RF_DOWNLINK_CHANNEL 110
#define DEF_RF_DOWNLINK_MOD PHY_WM4800
#define DEF_PING_RX_DELAY 1
#define DEF_PING_RX_LENGTH 12

#define BUFFER_SZ_MAX 8
uint8_t gaSendBuff[BUFFER_SZ_MAX];
net_msg_t gsPingMsg = { .pData = gaSendBuff};

uint8_t gaRecvBuff[BUFFER_SZ_MAX];
net_msg_t gsPongMsg = { .pData = gaRecvBuff};

static uint8_t __Param_Access_stub__(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir, int cmock_num_calls)
{
	uint8_t ret = 1;
	if(!u8_Dir)
	{
		switch(u8_Id)
		{
			case RF_DOWNLINK_CHANNEL:
				*p_Data = DEF_RF_DOWNLINK_CHANNEL;
				break;
			case RF_DOWNLINK_MOD:
				*p_Data = DEF_RF_DOWNLINK_MOD;
				break;
			case PING_RX_DELAY:
				*p_Data = DEF_PING_RX_DELAY;
				break;
			case PING_RX_LENGTH:
				*p_Data = DEF_PING_RX_LENGTH;
				break;
			default:
				break;
		}
	}
	else
	{
		switch(u8_Id)
		{
			case CLOCK_CURRENT_EPOC:
				break;
			case TX_FREQ_OFFSET:
				break;
			case PING_LAST_EPOCH:
				break;
			case PING_NBFOUND:
				break;
			default:
				if( (u8_Id >= PING_REPLY1) || (u8_Id <= PING_REPLY1 + (NB_PING_REPLPY-1)) )
				{

				}
				break;
		}
	}
	return ret;
}

/******************************************************************************/

TEST_SETUP(WizeCoreAppInt_inst)
{
	Param_Access_Stub(__Param_Access_stub__);
	sPingReplyCtx.sPingReplyConfig.AutoAdj_ClkFreq = 0;
	sPingReplyCtx.sPingReplyConfig.AutoAdj_Rssi = 0;
}

TEST_TEAR_DOWN(WizeCoreAppInt_inst)
{

}

TEST(WizeCoreAppInt_inst, test_InstInt_Init)
{
	inst_ping_t sPing;
	sPing = InstInt_Init(&sPingReplyCtx);

	TEST_ASSERT_EQUAL_UINT8_MESSAGE(DEF_RF_DOWNLINK_CHANNEL, sPing.L7DownChannel, "Channel");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(DEF_RF_DOWNLINK_MOD, sPing.L7DownMod, "Modulation");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(DEF_PING_RX_DELAY, sPing.L7PingRxDelay, "RX delay");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(DEF_PING_RX_LENGTH, sPing.L7PingRxLength, "RX length");
}

TEST(WizeCoreAppInt_inst, test_InstInt_Add)
{
	uint8_t i;
	char msg_str[10];
	uint8_t rssi_thr;
	inst_ping_t sPing;
	inst_pong_t *pRsp;
	ping_reply_list_t *pReply;

	gsPongMsg.u8Size = sizeof(inst_pong_t);

	pRsp = (inst_pong_t*)gaRecvBuff;
	memset(pRsp->L7ConcentId, 0, 6);
	srand( time( NULL ) );


	sPing = InstInt_Init(&sPingReplyCtx);


	for (i = 0; i < 20; i++)
	{
		pRsp->L7ConcentId[5] = i;
		pRsp->L7ModemId = (i%4);
		pRsp->L7RssiUpstream  = rand() % 255 ;
		gsPongMsg.u8Rssi = rand() % 255 ;
		gsPongMsg.i16TxFreqOffset = 0;
		InstInt_Add(&sPingReplyCtx, &gsPongMsg);
	}

	pReply = sPingReplyCtx.pBest;
	while (pReply != sPingReplyCtx.pWorst)
	{
		if(pReply->pNext == sPingReplyCtx.pWorst)
		{
			rssi_thr = 0;
		}
		else
		{
			rssi_thr = pReply->pNext->xPingReply.RssiDownstream;
		}
		sprintf(msg_str, "RSSI %d", i);
		TEST_ASSERT_GREATER_OR_EQUAL_UINT8_MESSAGE(rssi_thr, pReply->xPingReply.RssiDownstream, msg_str);
		pReply = pReply->pNext;
	}
}

TEST(WizeCoreAppInt_inst, test_InstInt_End)
{
	uint8_t ret;
	uint8_t i;
	char msg_str[10];
	uint8_t rssi_thr;
	inst_ping_t sPing;
	inst_pong_t *pRsp;
	ping_reply_list_t *pReply;

	gsPingMsg.u16Id = 0;
	gsPongMsg.u8Size = sizeof(inst_pong_t);

	pRsp = (inst_pong_t*)gaRecvBuff;
	memset(pRsp->L7ConcentId, 0, 6);
	srand( time( NULL ) );
	sPing = InstInt_Init(&sPingReplyCtx);

#define NB_PONG 20
	for (i = 0; i < NB_PONG; i++)
	{
		pRsp->L7ConcentId[5] = i;
		pRsp->L7ModemId = (i%4);
		pRsp->L7RssiUpstream  = rand() % 255 ;
		gsPongMsg.u8Rssi = rand() % 255 ;
		gsPongMsg.i16TxFreqOffset = 0;
		InstInt_Add(&sPingReplyCtx, &gsPongMsg);
	}

	ret = InstInt_End(&sPingReplyCtx);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(NB_PONG, ret, "Nb pong");
}

