#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(WizeCoreApp_wizeapp);
#include "wize_app.h"

#include "ses_dispatcher.h"
#include "inst_mgr.h"
#include "adm_mgr.h"
#include "dwn_mgr.h"

// Mock API
#include "mock_wize_api.h"

// Mock internal
#include "mock_inst_internal.h"
#include "mock_adm_internal.h"

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
static struct ping_reply_ctx_s *pPingReplyCtx;

/******************************************************************************/
uint8_t init_test = 0;

/******************************************************************************/
TEST_SETUP(WizeCoreApp_wizeapp)
{
	// Logger
	Logger_Put_Ignore();
	Logger_Post_Ignore();
	Logger_Frame_Ignore();

	// TimeEvt
	TimeEvt_TimerInit_IgnoreAndReturn(0);
	TimeEvt_TimerStart_IgnoreAndReturn(0);
	TimeEvt_TimerStop_Ignore();
	TimeEvt_UpdateTime_Ignore();

	// Parameters
	Param_Access_IgnoreAndReturn(1);

	// API
	WizeApi_ExecPing_IgnoreAndReturn(WIZE_API_SUCCESS);
	WizeApi_Send_IgnoreAndReturn(WIZE_API_SUCCESS);
	WizeApi_Download_IgnoreAndReturn(WIZE_API_SUCCESS);
	WizeApi_Notify_Ignore();

	// Internal
	AdmInt_PreCmd_IgnoreAndReturn(1);
	AdmInt_PostCmd_IgnoreAndReturn(0);

	inst_ping_t ping_val = {0, 0, 0, 0};
	InstInt_Init_IgnoreAndReturn(ping_val);
	InstInt_Add_Ignore();
	InstInt_End_IgnoreAndReturn(0);

	// ----
	sSesDispCtx.sSesCtx[SES_INST].pPrivate = &(sInstCtx);
	sSesDispCtx.sSesCtx[SES_ADM].pPrivate = &(sAdmCtx);
	sSesDispCtx.sSesCtx[SES_DWN].pPrivate = &(sDwnCtx);
	SesDisp_Setup(&sSesDispCtx);
	SesDisp_Init(&sSesDispCtx, 1);

	if (init_test)
	{
		WizeApp_Init(
			&sAdmCtx.sCmdMsg,
			&sAdmCtx.sRspMsg,
			&sInstCtx.sRspMsg,
			&sDwnCtx.sRecvMsg
		);
	}
}

TEST_TEAR_DOWN(WizeCoreApp_wizeapp)
{
}

TEST(WizeCoreApp_wizeapp, test_WizeApp_Init)
{
	uint32_t ret;

	AdmInt_PreCmd_IgnoreAndReturn(1);
	AdmInt_PostCmd_IgnoreAndReturn(ADM_WRITE_PARAM);

	WizeApp_Init(NULL, &sAdmCtx.sRspMsg, &sInstCtx.sRspMsg, &sDwnCtx.sRecvMsg);
	ret = WizeApp_Common(SES_FLG_CMD_RECV);
	ret = WizeApp_Common(SES_FLG_RSP_SENT);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, ret, "Return");


	WizeApp_Init(&sAdmCtx.sCmdMsg, NULL, &sInstCtx.sRspMsg, &sDwnCtx.sRecvMsg);
	ret = WizeApp_Common(SES_FLG_CMD_RECV);
	ret = WizeApp_Common(SES_FLG_RSP_SENT);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, ret, "Return");

	WizeApp_Init(&sAdmCtx.sCmdMsg, &sAdmCtx.sRspMsg, NULL, &sDwnCtx.sRecvMsg);
	ret = WizeApp_Common(SES_FLG_CMD_RECV);
	ret = WizeApp_Common(SES_FLG_RSP_SENT);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, ret, "Return");

	WizeApp_Init(&sAdmCtx.sCmdMsg, &sAdmCtx.sRspMsg, &sInstCtx.sRspMsg, NULL);
	ret = WizeApp_Common(SES_FLG_CMD_RECV);
	ret = WizeApp_Common(SES_FLG_RSP_SENT);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, ret, "Return");

	WizeApp_Init(&sAdmCtx.sCmdMsg, &sAdmCtx.sRspMsg, &sInstCtx.sRspMsg, &sDwnCtx.sRecvMsg);
	ret = WizeApp_Common(SES_FLG_CMD_RECV);
	ret = WizeApp_Common(SES_FLG_RSP_SENT);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(ADM_WRITE_PARAM, ret, "Return");

	init_test = 1;
}

TEST(WizeCoreApp_wizeapp, test_WizeApp_CommonXXX)
{
	uint32_t ulEvent;
	uint32_t ulRet, ulExpected;

	ulEvent = SES_EVT_DAY_PASSED;
	ulRet = WizeApp_Common(ulEvent);

	TEST_IGNORE_MESSAGE("No test available : TODO");



	//TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, ulRet, "Return");


	// Test ADM CMD/RSP pending/release


	// Test effective CMD after RSP


	// Test EXEC_PING into ADM



	// Test DWN


	// Test back full Power

	// Test Periodic PING



}

TEST(WizeCoreApp_wizeapp, test_WizeApp_TimeXXX)
{
	//TEST_ASSERT_TRUE_MESSAGE(0, "No test available : TODO");
    TEST_IGNORE_MESSAGE("No test available : TODO");

}

TEST(WizeCoreApp_wizeapp, test_WizeApp_AnnReadyNoPendRSP)
{
	uint32_t ret;
	admin_rsp_err_t *pRsp = (admin_rsp_err_t *)(sAdmCtx.sRspMsg.pData);

	pRsp->L7ErrorCode = 0;
	pRsp->L7ErrorParam = 0;

	// No pending RSP
	WizeApp_AnnReady(1, 1);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, pRsp->L7ErrorCode, "Err Code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, pRsp->L7ErrorParam, "Err Param");
}

TEST(WizeCoreApp_wizeapp, test_WizeApp_AnnReadyNotAnn)
{
	uint32_t ret;
	admin_rsp_err_t *pRsp = (admin_rsp_err_t *)(sAdmCtx.sRspMsg.pData);

	pRsp->L7ErrorCode = 0;
	pRsp->L7ErrorParam = 0;

	// Pending RSP
	AdmInt_PreCmd_IgnoreAndReturn(0);

	// RSP is not ADM_ANNDOWNLOAD
	sAdmCtx.sCmdMsg.pData[0] = ADM_EXECINSTPING;
	ret = WizeApp_Common(SES_FLG_CMD_RECV);

	pRsp->L7ResponseId = ADM_EXECINSTPING;
	WizeApp_AnnReady(1, 1);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, pRsp->L7ErrorCode, "Err Code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, pRsp->L7ErrorParam, "Err Param");
}

TEST(WizeCoreApp_wizeapp, test_WizeApp_AnnReadyIdMismatch)
{
	uint32_t ret;
	admin_rsp_err_t *pRsp = (admin_rsp_err_t *)(sAdmCtx.sRspMsg.pData);

	pRsp->L7ErrorCode = 0;
	pRsp->L7ErrorParam = 0;

	// Pending RSP
	AdmInt_PreCmd_IgnoreAndReturn(0);

	// CMD an RSP Id mismatch
	sAdmCtx.sCmdMsg.pData[0] = ADM_EXECINSTPING;
	pRsp->L7ResponseId = ADM_ANNDOWNLOAD;
	ret = WizeApp_Common(SES_FLG_CMD_RECV);
	WizeApp_AnnReady(1, 1);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, pRsp->L7ErrorCode, "Err Code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0, pRsp->L7ErrorParam, "Err Param");
}

TEST(WizeCoreApp_wizeapp, test_WizeApp_AnnReadyHasErrCode)
{
	uint32_t ret;
	admin_rsp_err_t *pRsp = (admin_rsp_err_t *)(sAdmCtx.sRspMsg.pData);

	pRsp->L7ErrorCode = 0;
	pRsp->L7ErrorParam = 0;

	// Pending RSP
	AdmInt_PreCmd_IgnoreAndReturn(0);

	// Error Code is set
	sAdmCtx.sCmdMsg.pData[0] = ADM_ANNDOWNLOAD;
	pRsp->L7ResponseId = ADM_ANNDOWNLOAD;
	ret = WizeApp_Common(SES_FLG_CMD_RECV);

	WizeApi_Notify_Expect(SES_EVT_ADM_READY);
	WizeApp_AnnReady(1, 1);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, pRsp->L7ErrorCode, "Err Code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, pRsp->L7ErrorParam, "Err Param");

	// Check that no RSP is pending
	WizeApi_Notify_Ignore();
	WizeApp_AnnReady(2, 2);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, pRsp->L7ErrorCode, "Err Code");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(1, pRsp->L7ErrorParam, "Err Param");
}
