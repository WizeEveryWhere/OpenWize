#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(WizeCoreApp_sesdisp);
#include "ses_dispatcher.h"

#include "inst_mgr.h"
#include "adm_mgr.h"
#include "dwn_mgr.h"

// Mock mgr
#include "mock_net_mgr.h"

// Mock Samples
#include "mock_time_evt.h"
#include "mock_parameters.h"
#include "mock_logger.h"

/******************************************************************************/
extern uint8_t gLoggerLevel;

static struct adm_mgr_ctx_s sAdmCtx;
static struct inst_mgr_ctx_s sInstCtx;
static struct dwn_mgr_ctx_s sDwnCtx;
static struct ses_disp_ctx_s sSesDispCtx;

/******************************************************************************/
enum {
	SETUP_SUCCESS = 0x01,
	INIT_SUCCESS = 0x02
};

uint8_t gPassTests = 0;

static void _config_dwn_(void);

static void _config_dwn_(void)
{
	time_t currentEpoch;
	// --------------------------------------
	sDwnCtx.u8DeltaSec = 5;
	sDwnCtx.u16BlocksCount = 1000;
	sDwnCtx.u8RxLength = 1;
	sDwnCtx.u8DayRepeat = 3;
	time(&currentEpoch);
	sDwnCtx.u32DaysProg = currentEpoch + 86400;

	sSesDispCtx.u32InstDurationMs = 5000;
	sSesDispCtx.u32DataDurationMs = 500;
	sSesDispCtx.u32CmdDurationMs = 1500;
	sSesDispCtx.u32RspDurationMs = 1500;
}

/******************************************************************************/

TEST_SETUP(WizeCoreApp_sesdisp)
{
	gLoggerLevel = LOG_LV_QUIET;

	// Logger
	Logger_Put_Ignore();
	Logger_Post_Ignore();
	Logger_Frame_Ignore();

	// TimeEvt
	TimeEvt_TimerInit_IgnoreAndReturn(0);
	TimeEvt_TimerStart_IgnoreAndReturn(0);
	TimeEvt_TimerStop_Ignore();
	TimeEvt_UpdateTime_Ignore();

	// Net Mgr
	NetMgr_Open_IgnoreAndReturn(NET_STATUS_OK);
	NetMgr_Close_IgnoreAndReturn(NET_STATUS_OK);
	NetMgr_Ioctl_IgnoreAndReturn(NET_STATUS_OK);
	NetMgr_Send_IgnoreAndReturn(NET_STATUS_OK);
	NetMgr_Listen_IgnoreAndReturn(NET_STATUS_OK);
	NetMgr_ListenReady_IgnoreAndReturn(NET_STATUS_OK);
	NetMgr_SetDwlink_IgnoreAndReturn(NET_STATUS_OK);
	NetMgr_SetUplink_IgnoreAndReturn(NET_STATUS_OK);

	// Parameters
	Param_Access_IgnoreAndReturn(1);

	// -----
	sSesDispCtx.sSesCtx[SES_INST].pPrivate = &(sInstCtx);
	sSesDispCtx.sSesCtx[SES_ADM].pPrivate = &(sAdmCtx);
	sSesDispCtx.sSesCtx[SES_DWN].pPrivate = &(sDwnCtx);

	if (gPassTests & SETUP_SUCCESS)
	{
		SesDisp_Setup(&sSesDispCtx);
		if (gPassTests & INIT_SUCCESS)
		{
			SesDisp_Init(&sSesDispCtx, 1);
			_config_dwn_();
		}
	}
}

TEST_TEAR_DOWN(WizeCoreApp_sesdisp)
{

}

/******************************************************************************/
// Test initialization, enable and disable

TEST(WizeCoreApp_sesdisp, test_SesDisp_Setup)
{
	SesDisp_Setup(&sSesDispCtx);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_DISP_STATE_DISABLE, sSesDispCtx.eState, "FSM State");
	gPassTests |= SETUP_SUCCESS;
}

TEST(WizeCoreApp_sesdisp, test_SesDisp_InitDisable)
{
	uint8_t i;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;
	SesDisp_Init(pCtx, 0);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_DISP_STATE_DISABLE, pCtx->eState, "FSM State");
	for (i = 0; i < SES_NB; i++)
	{
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_STATE_DISABLE, pCtx->sSesCtx[i].eState, "ini call");
	}
}

TEST(WizeCoreApp_sesdisp, test_SesDisp_InitEnable)
{
	uint8_t i;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;
	SesDisp_Init(pCtx, 1);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_DISP_STATE_ENABLE, pCtx->eState, "FSM State");
	for (i = 0; i < SES_NB; i++)
	{
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_STATE_IDLE, pCtx->sSesCtx[i].eState, "ini call");
	}
	gPassTests |= INIT_SUCCESS;
}

/******************************************************************************/
// Test that only one session of the same type is able to be executed

TEST(WizeCoreApp_sesdisp, test_SesDisp_RejectSame_INSTSession)
{
	uint32_t ret, expected_msk;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	ret = SesDisp_Fsm(pCtx, SES_EVT_INST_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "Return");
	expected_msk = SES_EVT_ADM_OPEN | SES_EVT_INST_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");

	// case 1 : try to open an already running session
	ret = SesDisp_Fsm(pCtx, SES_EVT_INST_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_INST_ERROR, ret, "Return");
}

TEST(WizeCoreApp_sesdisp, test_SesDisp_RejectSame_ADMSession)
{
	uint32_t ret, expected_msk;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	ret = SesDisp_Fsm(pCtx, SES_EVT_ADM_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "Return");
	expected_msk = SES_EVT_ADM_OPEN | SES_EVT_INST_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");

	// case 1 : try to open an already running session
	ret = SesDisp_Fsm(pCtx, SES_EVT_ADM_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_ADM_ERROR, ret, "Return");
}

TEST(WizeCoreApp_sesdisp, test_SesDisp_RejectSame_DWNSession)
{
	uint32_t ret, expected_msk;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	ret = SesDisp_Fsm(pCtx, SES_EVT_DWN_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "Return");
	expected_msk = SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");

	// case 1 : try to open an already running session
	ret = SesDisp_Fsm(pCtx, SES_EVT_DWN_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_DWN_ERROR, ret, "Return");
}

/******************************************************************************/
// Test that mutual exclusion between sessions

TEST(WizeCoreApp_sesdisp, test_SesDisp_RejectMutEx_ADMDuringINST)
{
	uint32_t ret, expected_msk;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	ret = SesDisp_Fsm(pCtx, SES_EVT_INST_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "Return");
	expected_msk = SES_EVT_ADM_OPEN | SES_EVT_INST_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");

	// case 2 : try to open a mutual exclusive session
	//   2.1 : try open ADM during INST
	ret = SesDisp_Fsm(pCtx, SES_EVT_ADM_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_ADM_ERROR, ret, "Return");
}

TEST(WizeCoreApp_sesdisp, test_SesDisp_RejectMutEx_INSTDuringADM)
{
	uint32_t ret, expected_msk;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	ret = SesDisp_Fsm(pCtx, SES_EVT_ADM_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "Return");
	expected_msk = SES_EVT_ADM_OPEN | SES_EVT_INST_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");

	// case 2 : try to open a mutual exclusive session
	//   2.2 : try open INST during ADM (warning this is not the EXECPING)
	ret = SesDisp_Fsm(pCtx, SES_EVT_INST_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_INST_ERROR, ret, "Return");
}

/******************************************************************************/
// Test that other sessions are rejected during a running DWN session

TEST(WizeCoreApp_sesdisp, test_SesDisp_RejectNoTime_INSTDuringDWNListening)
{
	uint32_t ret, expected_msk;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	ret = SesDisp_Fsm(pCtx, SES_EVT_DWN_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "Return");
	expected_msk = SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");
	// case 3 : try to open INST or ADM during DWN session
	//   3.1 : INST
	//   3.1.1 : DWN session state is listening
	pCtx->sSesCtx[SES_DWN].eState = SES_STATE_LISTENING;
	ret = SesDisp_Fsm(pCtx, SES_EVT_INST_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_INST_ERROR, ret, "Return");
	expected_msk = SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");
}

TEST(WizeCoreApp_sesdisp, test_SesDisp_RejectNoTime_INSTDuringDWNWaiting)
{
	uint32_t ret, expected_msk;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	ret = SesDisp_Fsm(pCtx, SES_EVT_DWN_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "Return");
	expected_msk = SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");
	// case 3 : try to open INST or ADM during DWN session
	//   3.1 : INST
	//   3.1.2 : DWN session state is waiting
	pCtx->sSesCtx[SES_DWN].sTimeEvt.u64Value = pCtx->u32InstDurationMs - 1;

	pCtx->sSesCtx[SES_DWN].eState = SES_STATE_WAITING_RX_DELAY;
	ret = SesDisp_Fsm(pCtx, SES_EVT_INST_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_INST_ERROR, ret, "Return");
	expected_msk = SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");
}

TEST(WizeCoreApp_sesdisp, test_SesDisp_RejectNoTime_ADMDuringDWNListening)
{
	uint32_t ret, expected_msk;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	ret = SesDisp_Fsm(pCtx, SES_EVT_DWN_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "Return");
	expected_msk = SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");

	// case 3 : try to open INST or ADM during DWN session
	//   3.2 : ADM :
	//   3.2.1 : DWN session state is listening
	//   3.2.1.1 : All rejected
	pCtx->sSesCtx[SES_DWN].eState = SES_STATE_LISTENING;
	ret = SesDisp_Fsm(pCtx, SES_EVT_ADM_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_ADM_ERROR, ret, "Return");
	expected_msk = SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");
}

TEST(WizeCoreApp_sesdisp, test_SesDisp_RejectNoTime_ADMallDuringDWNWaiting)
{
	uint32_t ret, expected_msk;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	ret = SesDisp_Fsm(pCtx, SES_EVT_DWN_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "Return");
	expected_msk = SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");

	// case 3 : try to open INST or ADM during DWN session
	//   3.2 : ADM :
	//   3.2.2 :DWN session state is waiting
	pCtx->sSesCtx[SES_DWN].eState = SES_STATE_WAITING_RX_DELAY;

	// 3.2.2.1 : DATA + CMD + RSP rejected
	pCtx->sSesCtx[SES_DWN].sTimeEvt.u64Value = pCtx->u32DataDurationMs -1;
	ret = SesDisp_Fsm(pCtx, SES_EVT_ADM_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_ADM_ERROR, ret, "Return");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, sAdmCtx.u8ByPassCmd, "CMD enable");
	expected_msk = SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");
}

TEST(WizeCoreApp_sesdisp, test_SesDisp_RejectNoTime_ADMcmdrspDuringDWNWaiting)
{
	uint32_t ret, expected_msk;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	ret = SesDisp_Fsm(pCtx, SES_EVT_DWN_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "Return");
	expected_msk = SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");

	// case 3 : try to open INST or ADM during DWN session
	//   3.2 : ADM :
	//   3.2.2 :DWN session state is waiting
	pCtx->sSesCtx[SES_DWN].eState = SES_STATE_WAITING_RX_DELAY;

	//   3.2.2.2 : CMD + RSP rejected (note : CMD can be use only to ....)
	pCtx->sSesCtx[SES_DWN].sTimeEvt.u64Value = pCtx->u32DataDurationMs + pCtx->u32CmdDurationMs -1;
	ret = SesDisp_Fsm(pCtx, SES_EVT_ADM_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "Return");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, sAdmCtx.u8ByPassCmd, "CMD disable");
	expected_msk = SES_EVT_ADM_DELAY_EXPIRED | SES_EVT_ADM_OPEN | SES_EVT_INST_OPEN | SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");
}

TEST(WizeCoreApp_sesdisp, test_SesDisp_RejectNoTime_ADMrspDuringDWNWaiting)
{
	uint32_t ret, expected_msk;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	ret = SesDisp_Fsm(pCtx, SES_EVT_DWN_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "Return");
	expected_msk = SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");

	// case 3 : try to open INST or ADM during DWN session
	//   3.2 : ADM :
	//   3.2.2 :DWN session state is waiting
	pCtx->sSesCtx[SES_DWN].eState = SES_STATE_WAITING_RX_DELAY;

	//   3.2.2.3 : RSP rejected
	pCtx->sSesCtx[SES_DWN].sTimeEvt.u64Value = pCtx->u32DataDurationMs + pCtx->u32CmdDurationMs + pCtx->u32RspDurationMs -1;
	ret = SesDisp_Fsm(pCtx, SES_EVT_ADM_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_NONE, ret, "RSP disable");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, sAdmCtx.u8ByPassCmd, "CMD enable");
	expected_msk = SES_EVT_ADM_READY | SES_EVT_ADM_OPEN | SES_EVT_INST_OPEN | SES_EVT_DWN_OPEN;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected_msk, pCtx->forbidden_msk, "Forbid mask");
}

/******************************************************************************/
// Test that the case where error occurs NetMgr (Open and Ioctl)

TEST(WizeCoreApp_sesdisp, test_SesDisp_NetMgrError)
{
	uint32_t ret;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	NetMgr_Ioctl_IgnoreAndReturn(NET_STATUS_ERROR);
	ret = SesDisp_Fsm(pCtx, SES_EVT_INST_OPEN);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(SES_FLG_UNKNOWN_ERROR, ret, "Return");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, pCtx->u8ActiveSes, "u8ActiveSes");
	TEST_ASSERT_NULL_MESSAGE(pCtx->pActive, "pActive");
}

/******************************************************************************/
// Test that the right session gets the Net event

TEST(WizeCoreApp_sesdisp, test_SesDisp_ActiveINSTGetNetEvent)
{
	uint32_t ret, expected;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;
	ret = SesDisp_Fsm(pCtx, SES_EVT_INST_OPEN);
	expected = SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(&(pCtx->sSesCtx[SES_INST]), pCtx->pActive, "pActive");

	ret = SesDisp_Fsm(pCtx, SES_EVT_SEND_DONE);
	expected = SES_FLG_PING_SENT;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_NULL_MESSAGE(pCtx->pActive, "pActive");

	ret = SesDisp_Fsm(pCtx, SES_EVT_INST_DELAY_EXPIRED);
	expected = SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(&(pCtx->sSesCtx[SES_INST]), pCtx->pActive, "pActive");

	ret = SesDisp_Fsm(pCtx, SES_EVT_RECV_DONE);
	expected = SES_FLG_PONG_RECV;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(&(pCtx->sSesCtx[SES_INST]), pCtx->pActive, "pActive");

	ret = SesDisp_Fsm(pCtx, SES_EVT_TIMEOUT);
	expected = (SES_FLG_INST_TIMEOUT | SES_FLG_INST_COMPLETE);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_NULL_MESSAGE(pCtx->pActive, "pActive");
}

TEST(WizeCoreApp_sesdisp, test_SesDisp_ActiveADMGetNetEvent)
{
	uint32_t ret, expected;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;

	ret = SesDisp_Fsm(pCtx, SES_EVT_ADM_OPEN);
	expected = SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(&(pCtx->sSesCtx[SES_ADM]), pCtx->pActive, "pActive");

	ret = SesDisp_Fsm(pCtx, SES_EVT_SEND_DONE);
	expected = SES_FLG_DATA_SENT;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_NULL_MESSAGE(pCtx->pActive, "pActive");

	ret = SesDisp_Fsm(pCtx, SES_EVT_ADM_DELAY_EXPIRED);
	expected = SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(&(pCtx->sSesCtx[SES_ADM]), pCtx->pActive, "pActive");

	ret = SesDisp_Fsm(pCtx, SES_EVT_RECV_DONE);
	expected = SES_FLG_CMD_RECV;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_NULL_MESSAGE(pCtx->pActive, "pActive");

	ret = SesDisp_Fsm(pCtx, SES_EVT_ADM_READY);
	expected = SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_NULL_MESSAGE(pCtx->pActive, "pActive");

	ret = SesDisp_Fsm(pCtx, SES_EVT_ADM_DELAY_EXPIRED);
	expected = SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_EQUAL_PTR_MESSAGE(&(pCtx->sSesCtx[SES_ADM]), pCtx->pActive, "pActive");

	ret = SesDisp_Fsm(pCtx, SES_EVT_SEND_DONE);
	expected = (SES_FLG_RSP_SENT | SES_FLG_ADM_COMPLETE);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_NULL_MESSAGE(pCtx->pActive, "pActive");
}

TEST(WizeCoreApp_sesdisp, test_SesDisp_ActiveDWNGetNetEvent)
{
	uint32_t ret, expected;
	struct ses_disp_ctx_s *pCtx = &sSesDispCtx;
	uint32_t i, j;

	sDwnCtx.u16BlocksCount = 10;
	sDwnCtx.u8DayRepeat = 2;

	ret = SesDisp_Fsm(pCtx, SES_EVT_DWN_OPEN);
	expected = SES_FLG_NONE;
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
	TEST_ASSERT_NULL_MESSAGE(pCtx->pActive, "pActive");

	// loop on each day
	for (i = sDwnCtx.u8DayRepeat; i > 0 ; i--)
	{
		// loop on each block
		for (j = sDwnCtx.u16BlocksCount; j > 0; j--)
		{
			// waiting for listening window
			ret = SesDisp_Fsm(pCtx, SES_EVT_DWN_DELAY_EXPIRED);
			expected = SES_FLG_NONE;
			TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
			TEST_ASSERT_EQUAL_PTR_MESSAGE(&(pCtx->sSesCtx[SES_DWN]), pCtx->pActive, "pActive");

			// listening
			if ( j == 3)
			{
				// timeout
				ret = SesDisp_Fsm(pCtx, SES_EVT_TIMEOUT);
				expected = SES_FLG_DWN_TIMEOUT;
			}
			else if ( j == 5)
			{
				// out of date
				ret = SesDisp_Fsm(pCtx, SES_EVT_RECV_DONE);
				expected = SES_FLG_DWN_OUT_DATE;
			}
			else
			{
				// receive block
				ret = SesDisp_Fsm(pCtx, SES_EVT_RECV_DONE);
				expected = SES_FLG_BLK_RECV;
			}
			TEST_ASSERT_BITS_HIGH_MESSAGE(expected, ret, "Return");
			TEST_ASSERT_NULL_MESSAGE(pCtx->pActive, "pActive");

			if ( (i == 1) && (j == 1) )
			{
				// no more day, session done
				expected =  SES_FLG_DWN_COMPLETE;
				TEST_ASSERT_BITS_HIGH_MESSAGE(expected, ret, "Return");
				TEST_ASSERT_NULL_MESSAGE(pCtx->pActive, "pActive");
			}

			// release buffer
			if ( j != 6)
			{
				ret = SesDisp_Fsm(pCtx, SES_EVT_DWN_READY);
				expected = SES_FLG_NONE;
				TEST_ASSERT_EQUAL_UINT32_MESSAGE(expected, ret, "Return");
				TEST_ASSERT_NULL_MESSAGE(pCtx->pActive, "pActive");
			}
			// else // next block (5) will be "out of date"
		}
	}
}

/******************************************************************************/
