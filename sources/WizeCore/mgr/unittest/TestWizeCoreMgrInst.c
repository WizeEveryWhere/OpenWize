#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(WizeCoreMgr_inst);
#include "inst_mgr.h"

// Mock NetMgr
#include "mock_net_mgr.h"

// Mock Samples
#include "mock_time_evt.h"
#include "mock_logger.h"

/******************************************************************************/
/*
 *
 *    transition      |                         state
 *                    | DISABLE | IDLE    |   SENDING    |        W_RX      | LISTENING
 * -------------------------------------------------------------------------------------
 * INST_OPEN          |    X    | SENDING |      X       |         X        |    X
 * INST_CANCEL        |  IDLE   |  IDLE   |     IDLE     |        IDLE      |   IDLE
 * INST_READY         |    X    |   X     |      X       |         X        |  X / IDLE
 * SEND_DONE          |    X    |   X     | W_RX / IDLE  |         X        |    X
 * TIMEOUT            |    X    |   X     |     IDLE     |         X        |   IDLE
 * INST_DELAY_EXPIRED |    X    |   X     |      X       | LISTENING / IDLE |    X
 * RECV_DONE          |    X    |   X     |      X       |         X        |    X
 */

/******************************************************************************/
extern uint8_t gLoggerLevel;

static struct inst_mgr_ctx_s sInstCtx;
static struct ses_ctx_s sCtx = {.pPrivate = &sInstCtx};

/******************************************************************************/

TEST_SETUP(WizeCoreMgr_inst)
{
	gLoggerLevel = LOG_LV_QUIET;

	Logger_Put_Ignore();
	Logger_Post_Ignore();
	Logger_Frame_Ignore();

	TimeEvt_TimerInit_IgnoreAndReturn(0);
	TimeEvt_TimerStop_Ignore();
	TimeEvt_TimerStart_IgnoreAndReturn(0);

	NetMgr_ListenReady_IgnoreAndReturn(NET_STATUS_OK);
	NetMgr_Send_IgnoreAndReturn(NET_STATUS_OK);
	NetMgr_Listen_IgnoreAndReturn(NET_STATUS_OK);

	InstMgr_Setup(&sCtx);
}
TEST_TEAR_DOWN(WizeCoreMgr_inst)
{
}

/******************************************************************************/
TEST(WizeCoreMgr_inst, test_InstMgr_Init)
{
	struct ses_ctx_s *pCtx = &sCtx;

	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_STATE_DISABLE, pCtx->eState, "FSM State");
	pCtx->ini(pCtx, 1);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_STATE_IDLE, pCtx->eState, "FSM State");
}

TEST(WizeCoreMgr_inst, test_InstMgr_FsmIdleState_SendError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Fail to Send
	NetMgr_Send_IgnoreAndReturn(NET_STATUS_ERROR);
	u32Evt = SES_EVT_INST_OPEN;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_INST_ERROR;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_inst, test_InstMgr_FsmSendingState_TimerStartError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_SENDING;

	// Fail to Start Timer
	TimeEvt_TimerStart_IgnoreAndReturn(1);
	u32Evt = SES_EVT_SEND_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg =  SES_FLG_INST_ERROR | SES_FLG_INST_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_inst, test_InstMgr_FsmSendingState_TimeOut)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_SENDING;

	// Send Timeout
	u32Evt = SES_EVT_TIMEOUT;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg =  SES_FLG_INST_TIMEOUT | SES_FLG_INST_ERROR  | SES_FLG_INST_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_inst, test_InstMgr_FsmWaitingState_ListenError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_WAITING_RX_DELAY;

	// Fail to Listen
	NetMgr_Listen_IgnoreAndReturn(NET_STATUS_ERROR);
	u32Evt = SES_EVT_INST_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg =  SES_FLG_INST_ERROR | SES_FLG_INST_COMPLETE;;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_inst, test_InstMgr_FsmWaitingState_ListenReadyError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_LISTENING;

	// Fail to Listen
	NetMgr_ListenReady_IgnoreAndReturn(NET_STATUS_ERROR);
	u32Evt = SES_EVT_INST_READY;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg =  SES_FLG_INST_ERROR | SES_FLG_INST_COMPLETE;;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_inst, test_InstMgr_FsmListeningState_Timeout)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_LISTENING;

	// Listen Timeout
	u32Evt = SES_EVT_TIMEOUT;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg =  SES_FLG_INST_TIMEOUT | SES_FLG_INST_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_inst, test_InstMgr_FsmListeningState_LostMsg)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_LISTENING;
	((struct inst_mgr_ctx_s*)pCtx->pPrivate)->u8Pending = 1;

	// Received message is lost or miss
	u32Evt = SES_EVT_RECV_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_LISTENING;
	u32ExpectedFlg =  SES_FLG_INST_OUT_DATE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_inst, test_InstMgr_FsmSendingState_SendOk)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_SENDING;

	// Send ok
	u32Evt = SES_EVT_SEND_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_WAITING_RX_DELAY;
	u32ExpectedFlg =  SES_FLG_PING_SENT;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_inst, test_InstMgr_Fsm_ProcessCanceled)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	const uint32_t u32AvailableStates[] = {
			SES_STATE_IDLE,
			SES_STATE_SENDING,
			SES_STATE_WAITING_RX_DELAY,
			SES_STATE_LISTENING
	};
	// Loop on each state
	uint8_t i;
	for( i = 0; i < sizeof(u32AvailableStates); i++)
	{
	// Initial state
		pCtx->eState = u32AvailableStates[i];

		// Cancel the session
		u32Evt = SES_EVT_INST_CANCEL;
		u32Flg = pCtx->fsm(pCtx, u32Evt);
		u32ExpectedState = SES_STATE_IDLE;
		u32ExpectedFlg =  SES_FLG_INST_COMPLETE;
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
	}
}

TEST(WizeCoreMgr_inst, test_InstMgr_Fsm_ProcessOk)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	struct inst_mgr_ctx_s *pPrivate = (struct inst_mgr_ctx_s*)pCtx->pPrivate;
	pCtx->ini(pCtx, 1);

	// Open
	u32Evt = SES_EVT_INST_OPEN;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_SENDING;
	u32ExpectedFlg =  SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");

	// Send Ok
	u32Evt = SES_EVT_SEND_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_WAITING_RX_DELAY;
	u32ExpectedFlg =  SES_FLG_PING_SENT;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");

	// Wait Ok
	u32Evt = SES_EVT_INST_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_LISTENING;
	u32ExpectedFlg =  SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");

	// Loop on Listen
	uint8_t i;
	for(i = 0; i < 5; i++)
	{
		// Listen Ok
		u32Evt = SES_EVT_RECV_DONE;
		u32Flg = pCtx->fsm(pCtx, u32Evt);
		u32ExpectedState = SES_STATE_LISTENING;
		u32ExpectedFlg =  SES_FLG_PONG_RECV;
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
		TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, pPrivate->u8Pending, "Pending bit");

		// Release msg buffer
		u32Evt = SES_EVT_INST_READY;
		u32Flg = pCtx->fsm(pCtx, u32Evt);
		u32ExpectedState = SES_STATE_LISTENING;
		u32ExpectedFlg =  SES_FLG_NONE;
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
		TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, pPrivate->u8Pending, "Pending bit");
	}

	// Listen Timeout
	u32Evt = SES_EVT_TIMEOUT;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg =  SES_FLG_INST_TIMEOUT | SES_FLG_INST_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}
