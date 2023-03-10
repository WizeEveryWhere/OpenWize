#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(WizeCoreMgr_dwn);
#include "dwn_mgr.h"

// Mock NetMgr
#include "mock_net_mgr.h"

// Mock Samples
#include "mock_time_evt.h"
#include "mock_logger.h"

/******************************************************************************/
extern uint8_t gLoggerLevel;

static struct dwn_mgr_ctx_s sDwnCtx;
static struct ses_ctx_s sCtx = {.pPrivate = &sDwnCtx};

/******************************************************************************/

TEST_SETUP(WizeCoreMgr_dwn)
{
	gLoggerLevel = LOG_LV_QUIET;

	Logger_Put_Ignore();
	Logger_Post_Ignore();
	Logger_Frame_Ignore();

	TimeEvt_TimerInit_IgnoreAndReturn(0);
	TimeEvt_TimerStop_Ignore();
	TimeEvt_TimerStart_IgnoreAndReturn(0);

	NetMgr_Listen_IgnoreAndReturn(NET_STATUS_OK);
	NetMgr_SetDwlink_IgnoreAndReturn(NET_STATUS_OK);

	DwnMgr_Setup(&sCtx);

	time_t currentEpoch;
	// --------------------------------------
	sDwnCtx.u8DeltaSec = 5;
	sDwnCtx.u16BlocksCount = 1000;
	sDwnCtx.u8RxLength = 1;
	sDwnCtx.u8DayRepeat = 3;
	time(&currentEpoch);
	sDwnCtx.u32DaysProg = currentEpoch + 86400;
}

TEST_TEAR_DOWN(WizeCoreMgr_dwn)
{

}

/******************************************************************************/
TEST(WizeCoreMgr_dwn, test_DwnMgr_Init)
{
	struct ses_ctx_s *pCtx = &sCtx;

	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_STATE_DISABLE, pCtx->eState, "FSM State");
	pCtx->ini(pCtx, 1);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_STATE_IDLE, pCtx->eState, "FSM State");
}

TEST(WizeCoreMgr_dwn, test_DwnMgr_FsmIdleState_NextBlkOffsetError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	struct dwn_mgr_ctx_s *pPrvCtx = (struct dwn_mgr_ctx_s*)pCtx->pPrivate;
	pCtx->ini(pCtx, 1);

	time_t currentEpoch;
	// --------------------------------------
	pPrvCtx->u8DeltaSec = 5;
	pPrvCtx->u16BlocksCount = 1000;
	pPrvCtx->u8RxLength = 1;
	// Set all days have been missed
	time(&currentEpoch);
	pPrvCtx->u8DayRepeat = 1;
	pPrvCtx->u32DaysProg = currentEpoch - 86400;

	// Error on the Date of Next Block
	u32Evt = SES_EVT_DWN_OPEN;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_DWN_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");

	// --------------------------------------
	pPrvCtx->u8DeltaSec = 5;
	pPrvCtx->u16BlocksCount = 1000;
	pPrvCtx->u8RxLength = 1;
	// One day is remained, but no more block in that day
	time(&currentEpoch);
	pPrvCtx->u8DayRepeat = 1;
	pPrvCtx->u32DaysProg = currentEpoch - (pPrvCtx->u16BlocksCount * (pPrvCtx->u8DeltaSec) );

	// Error on the Date of Next Block
	u32Evt = SES_EVT_DWN_OPEN;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg = SES_FLG_DWN_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_dwn, test_DwnMgr_FsmIdleState_TimerStartError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Fail to Start Timer
	TimeEvt_TimerStart_IgnoreAndReturn(1);
	u32Evt = SES_EVT_DWN_OPEN;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg =  SES_FLG_DWN_ERROR | SES_FLG_DWN_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_dwn, test_DwnMgr_FsmWaitingState_TimerStartError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_WAITING;

	// Fail to Start Timer
	TimeEvt_TimerStart_IgnoreAndReturn(1);
	u32Evt = SES_EVT_DWN_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg =  SES_FLG_DWN_ERROR | SES_FLG_DWN_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_dwn, test_DwnMgr_FsmWaitingRXState_SetDownlinkError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	struct dwn_mgr_ctx_s *pPrvCtx = (struct dwn_mgr_ctx_s*)pCtx->pPrivate;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_WAITING_RX_DELAY;
	pPrvCtx->_u16BlocksCount = 10;

	// Fail to SetDownlink
	NetMgr_SetDwlink_IgnoreAndReturn(NET_STATUS_ERROR);
	u32Evt = SES_EVT_DWN_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_WAITING_RX_DELAY;
	u32ExpectedFlg =  SES_FLG_DWN_ERROR;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_dwn, test_DwnMgr_FsmWaitingRXState_ListenError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	struct dwn_mgr_ctx_s *pPrvCtx = (struct dwn_mgr_ctx_s*)pCtx->pPrivate;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_WAITING_RX_DELAY;
	pPrvCtx->_u16BlocksCount = 10;

	// Fail to Listen
	NetMgr_Listen_IgnoreAndReturn(NET_STATUS_ERROR);
	u32Evt = SES_EVT_DWN_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_WAITING_RX_DELAY;
	u32ExpectedFlg =  SES_FLG_DWN_ERROR;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_dwn, test_DwnMgr_FsmWaitingRXState_TimerStartError)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	struct dwn_mgr_ctx_s *pPrvCtx = (struct dwn_mgr_ctx_s*)pCtx->pPrivate;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_WAITING_RX_DELAY;
	pPrvCtx->_u16BlocksCount = 0;
	pPrvCtx->_u8DayCount = 1;

	// Fail to Start Timer
	TimeEvt_TimerStart_IgnoreAndReturn(1);
	u32Evt = SES_EVT_DWN_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg =  SES_FLG_DWN_ERROR | SES_FLG_DWN_COMPLETE;;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_dwn, test_DwnMgr_FsmWaitingRXState_SessionDone)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	struct dwn_mgr_ctx_s *pPrvCtx = (struct dwn_mgr_ctx_s*)pCtx->pPrivate;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_WAITING_RX_DELAY;
	pPrvCtx->_u16BlocksCount = 0;
	pPrvCtx->_u8DayCount = 0;

	// Session done (case SES_STATE_WAITING_RX_DELAY state)
	u32Evt = SES_EVT_DWN_DELAY_EXPIRED;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg =  SES_FLG_DWN_COMPLETE;;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");

	// Initial state
	pCtx->eState = SES_STATE_LISTENING;
	pPrvCtx->_u16BlocksCount = 0;
	pPrvCtx->_u8DayCount = 0;

	// Session done (case SES_STATE_WAITING_RX_DELAY state)
	u32Evt = SES_EVT_RECV_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_IDLE;
	u32ExpectedFlg =  SES_FLG_BLK_RECV | SES_FLG_DWN_COMPLETE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_BITS_HIGH_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_dwn, test_DwnMgr_FsmListeningState_OutOfDate)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	struct dwn_mgr_ctx_s *pPrvCtx = (struct dwn_mgr_ctx_s*)pCtx->pPrivate;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_LISTENING;
	pPrvCtx->u8Pending = 1;
	pPrvCtx->_u16BlocksCount = 1;
	pPrvCtx->_u8DayCount = 1;

	// Session done
	u32Evt = SES_EVT_RECV_DONE;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_WAITING_RX_DELAY;
	u32ExpectedFlg =  SES_FLG_DWN_OUT_DATE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_BITS_HIGH_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_dwn, test_DwnMgr_FsmListeningState_Timeout)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	struct dwn_mgr_ctx_s *pPrvCtx = (struct dwn_mgr_ctx_s*)pCtx->pPrivate;
	pCtx->ini(pCtx, 1);

	// Initial state
	pCtx->eState = SES_STATE_LISTENING;
	pPrvCtx->u8Pending = 1;
	pPrvCtx->_u16BlocksCount = 1;
	pPrvCtx->_u8DayCount = 1;

	// Session done
	u32Evt = SES_EVT_TIMEOUT;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_WAITING_RX_DELAY;
	u32ExpectedFlg = SES_FLG_DWN_TIMEOUT;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_BITS_HIGH_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
}

TEST(WizeCoreMgr_dwn, test_DwnMgr_Fsm_ProcessCanceled)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;
	pCtx->ini(pCtx, 1);

	const uint32_t u32AvailableStates[] = {
			SES_STATE_IDLE,
			SES_STATE_WAITING,
			SES_STATE_WAITING_RX_DELAY,
			SES_STATE_LISTENING
	};
	// Loop on each state
	uint8_t i;
	for( i = 0; i < 4; i++)
	{
	// Initial state
		pCtx->eState = u32AvailableStates[i];

		// Cancel the session
		u32Evt = SES_EVT_DWN_CANCEL;
		u32Flg = pCtx->fsm(pCtx, u32Evt);
		u32ExpectedState = SES_STATE_IDLE;
		u32ExpectedFlg =  SES_FLG_DWN_COMPLETE;
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
		TEST_ASSERT_BITS_HIGH_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");
	}
}

TEST(WizeCoreMgr_dwn, test_DwnMgr_Fsm_ProcessOk)
{
	uint32_t u32Evt, u32Flg;
	uint32_t u32ExpectedState, u32ExpectedFlg;
	struct ses_ctx_s *pCtx = &sCtx;

	pCtx->ini(pCtx, 1);
	sDwnCtx.u16BlocksCount = 5;

	// Open
	u32Evt = SES_EVT_DWN_OPEN;
	u32Flg = pCtx->fsm(pCtx, u32Evt);
	u32ExpectedState = SES_STATE_WAITING;
	u32ExpectedFlg =  SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, "FSM State");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, "Flag");

	// --------------------------------------
	uint32_t i, j;
	uint32_t day, blk;
	char fsm_str[30];
	char flag_str[30];

	// loop on each day
	for (i = sDwnCtx.u8DayRepeat; i > 0 ; i--)
	{
		day = sDwnCtx.u8DayRepeat - i +1;

		// loop on each block
		for (j = sDwnCtx.u16BlocksCount; j > 0; j--)
		{
			blk = sDwnCtx.u16BlocksCount - j +1;
			sprintf(fsm_str, "FSM state; Day %d; Blk %d", day, blk);
			sprintf(flag_str, "Flag; Day %d; Blk %d", day, blk);

			// waiting for listening window
			u32Evt = SES_EVT_DWN_DELAY_EXPIRED;
			u32Flg = pCtx->fsm(pCtx, u32Evt);
			u32ExpectedState = SES_STATE_LISTENING;
			u32ExpectedFlg =  SES_FLG_NONE;
			TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, fsm_str);
			TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, flag_str);

			// listening
			u32Evt = SES_EVT_RECV_DONE;
			u32Flg = pCtx->fsm(pCtx, u32Evt);
			u32ExpectedFlg = SES_FLG_BLK_RECV;

			if( (i == 1) && (j == 1) )
			{
				// no more day, session done
				u32ExpectedState = SES_STATE_IDLE;
				u32ExpectedFlg |= SES_FLG_DWN_COMPLETE;
			}
			else
			{
				if(j == 1)
				{
					u32ExpectedState = SES_STATE_WAITING;
				}
				else
				{
					u32ExpectedState = SES_STATE_WAITING_RX_DELAY;
				}
			}

			TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedState, pCtx->eState, fsm_str);
			TEST_ASSERT_BITS_HIGH_MESSAGE(u32ExpectedFlg, u32Flg, flag_str);

			// release buffer
			u32Evt = SES_EVT_DWN_READY;
			u32Flg = pCtx->fsm(pCtx, u32Evt);
			u32ExpectedFlg =  SES_FLG_NONE;
			TEST_ASSERT_EQUAL_UINT32_MESSAGE(u32ExpectedFlg, u32Flg, flag_str);
		}
	}
}
