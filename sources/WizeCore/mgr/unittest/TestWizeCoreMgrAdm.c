#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(WizeCoreMgr_adm);
#include "adm_mgr.h"

// Mock Netmgr
#include "mock_net_mgr.h"

// Mock Samples
#include "mock_time_evt.h"
#include "mock_logger.h"

/******************************************************************************/
extern uint8_t gLoggerLevel;

static struct adm_mgr_ctx_s sAdmCtx;
static struct ses_ctx_s sCtx = {.pPrivate = &sAdmCtx};

/******************************************************************************/

TEST_SETUP(WizeCoreMgr_adm)
{
	gLoggerLevel = LOG_LV_QUIET;

	Logger_Put_Ignore();
	Logger_Post_Ignore();
	Logger_Frame_Ignore();

	TimeEvt_TimerInit_IgnoreAndReturn(0);
	TimeEvt_TimerStop_Ignore();
	TimeEvt_TimerStart_IgnoreAndReturn(0);

	NetMgr_Send_IgnoreAndReturn(NET_STATUS_OK);
	NetMgr_Listen_IgnoreAndReturn(NET_STATUS_OK);

	AdmMgr_Setup(&sCtx);
}
TEST_TEAR_DOWN(WizeCoreMgr_adm)
{

}

/******************************************************************************/

TEST(WizeCoreMgr_adm, test_AdmMgr_Init)
{
	struct ses_ctx_s *pCtx = &sCtx;

	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_STATE_DISABLE, pCtx->eState, "FSM State");
	pCtx->ini(pCtx, 1);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_STATE_IDLE, pCtx->eState, "FSM State");
}

TEST(WizeCoreMgr_adm, test_AdmMgr_FsmIdleState_SendError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Fail to Send
	NetMgr_Send_IgnoreAndReturn(NET_STATUS_ERROR);
	u32Evt = SES_EVT_ADM_OPEN;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_ADM_ERROR;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_adm, test_AdmMgr_FsmWaitingRXState_ByPassCmd)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_WAITING_RX_DELAY;
	((struct adm_mgr_ctx_s*)pCtx->pPrivate)->u8ByPassCmd = 1;

	// ByPass CCmd
	u32Evt = SES_EVT_ADM_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_ADM_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_adm, test_AdmMgr_FsmWaitingRXState_ListenError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_WAITING_RX_DELAY;
	((struct adm_mgr_ctx_s*)pCtx->pPrivate)->u8ByPassCmd = 0;

	// Fail to Listen
	NetMgr_Listen_IgnoreAndReturn(NET_STATUS_ERROR);
	u32Evt = SES_EVT_ADM_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_ADM_ERROR | SES_FLG_ADM_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_adm, test_AdmMgr_FsmListeningState_SendImmTimerStartError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_LISTENING;
	((struct adm_mgr_ctx_s*)pCtx->pPrivate)->u8ExchRespDelay = 0;

	// Fail to Start Timer
	TimeEvt_TimerStart_IgnoreAndReturn(1);
	u32Evt = SES_EVT_RECV_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_ADM_ERROR | SES_FLG_ADM_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_adm, test_AdmMgr_FsmListeningState_TimerStartError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_LISTENING;
	((struct adm_mgr_ctx_s*)pCtx->pPrivate)->u8ExchRespDelay = 1;

	// Fail to Start Timer
	TimeEvt_TimerStart_IgnoreAndReturn(1);
	u32Evt = SES_EVT_RECV_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_ADM_ERROR | SES_FLG_ADM_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_adm, test_AdmMgr_FsmListeningState_Timeout)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_LISTENING;

	// Timeout
	u32Evt = SES_EVT_TIMEOUT;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_ADM_TIMEOUT | SES_FLG_ADM_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_adm, test_AdmMgr_FsmWaintingTXState_SendError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_WAITING_TX_DELAY;
	((struct adm_mgr_ctx_s*)pCtx->pPrivate)->u8Pending = ADM_RSP_READY;

	// Fail to Send
	NetMgr_Send_IgnoreAndReturn(NET_STATUS_ERROR);
	u32Evt = SES_EVT_ADM_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_ADM_ERROR | SES_FLG_ADM_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_adm, test_AdmMgr_FsmWaintingTXState_SendOutOfDate)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_WAITING_TX_DELAY;
	((struct adm_mgr_ctx_s*)pCtx->pPrivate)->u8Pending = ADM_RSP_PEND;

	// Fail to Send
	NetMgr_Send_IgnoreAndReturn(NET_STATUS_ERROR);
	u32Evt = SES_EVT_ADM_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_ADM_OUT_DATE | SES_FLG_ADM_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_adm, test_AdmMgr_FsmSendingState_ByPassCmd)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_SENDING;
	((struct adm_mgr_ctx_s*)pCtx->pPrivate)->u8Pending = ADM_RSP_NONE;
	((struct adm_mgr_ctx_s*)pCtx->pPrivate)->u8ByPassCmd = 1;

	// ByPass Cmd
	u32Evt = SES_EVT_SEND_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_DATA_SENT | SES_FLG_ADM_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_adm, test_AdmMgr_FsmSendingState_TimerStartError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_SENDING;
	((struct adm_mgr_ctx_s*)pCtx->pPrivate)->u8Pending = ADM_RSP_NONE;
	((struct adm_mgr_ctx_s*)pCtx->pPrivate)->u8ByPassCmd = 0;

	// Fail Start Timer
	TimeEvt_TimerStart_IgnoreAndReturn(1);
	u32Evt = SES_EVT_SEND_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_DATA_SENT | SES_FLG_ADM_ERROR | SES_FLG_ADM_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_adm, test_AdmMgr_FsmSendingState_TimeOut)
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
	u32ExpectedFlg = SES_FLG_ADM_TIMEOUT | SES_FLG_ADM_ERROR | SES_FLG_ADM_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}


TEST(WizeCoreMgr_adm, test_AdmMgr_Fsm_ProcessCanceled)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	const uint32_t u32AvailableStates[] = {
			SES_STATE_IDLE,
			SES_STATE_WAITING_RX_DELAY,
			SES_STATE_LISTENING,
			SES_STATE_WAITING_TX_DELAY,
			SES_STATE_SENDING,
	};
	// Loop on each state
	uint8_t i;
	for( i = 0; i < sizeof(u32AvailableStates); i++)
	{
	// Initial state
		pCtx->eState = u32AvailableStates[i];

		// Cancel the session
		u32Evt = SES_EVT_ADM_CANCEL;
		u32Flg = pCtx->fsm(pCtx, u32Evt);
		u32ExpectedState = SES_STATE_IDLE;
		u32ExpectedFlg =  SES_FLG_ADM_COMPLETE;
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
	}
}

TEST(WizeCoreMgr_adm, test_AdmMgr_Fsm_ProcessOk)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Open
	u32Evt = SES_EVT_ADM_OPEN;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_SENDING;
	u32ExpectedFlg =  SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");

	// Send Ok
	u32Evt = SES_EVT_SEND_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_WAITING_RX_DELAY;
	u32ExpectedFlg =  SES_FLG_DATA_SENT;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");

	// Wait Ok
	u32Evt = SES_EVT_ADM_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_LISTENING;
	u32ExpectedFlg =  SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");

	// Listen
	u32Evt = SES_EVT_RECV_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_WAITING_TX_DELAY;
	u32ExpectedFlg =  SES_FLG_CMD_RECV;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");

	// Release buffer
	u32Evt = SES_EVT_ADM_READY;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	TEST_ASSERT_TRUE_MESSAGE(
		(((struct adm_mgr_ctx_s*)pCtx->pPrivate)->u8Pending & ADM_RSP_READY), "ADM_RSP_READY");

	// Wait ok
	u32Evt = SES_EVT_ADM_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_SENDING;
	u32ExpectedFlg =  SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");

	// Send Ok
	u32Evt = SES_EVT_SEND_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_RSP_SENT | SES_FLG_ADM_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");

}
