#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(WizeCoreApp_wizeapi);
#include "wize_api.h"
#include "ses_dispatcher.h"

// Mock
#include "mock_wize_app.h"
//#include "mock_ses_dispatcher.h"

// Mock Mgr
#include "mock_net_mgr.h"
#include "mock_time_mgr.h"

// Mock Samples
#include "mock_time_evt.h"
#include "mock_parameters.h"
#include "mock_logger.h"
#include "mock_crypto.h"

// Mock FreeRTOS
#include "mock_queue.h"
#include "mock_task.h"
#include "mock_portmacro.h"

/******************************************************************************/
extern uint8_t gLoggerLevel;

void _time_wakeup_enable(void) {}
void _time_wakeup_reload(void) {}
void _time_wakeup_force(void) {}
void _time_update_set_handler(pfTimeEvt_HandlerCB_t const pfCb) {}

/******************************************************************************/
uint8_t aParamValue[20];
uint8_t param_size = 1;
uint8_t _Param_Access_Stub_(uint8_t u8_Id, uint8_t* p_Data, uint8_t u8_Dir, int cmock_num_calls)
{
	TEST_ASSERT_NOT_NULL_MESSAGE(p_Data, "ParamAcess Ptr");
	memcpy(p_Data, aParamValue, param_size);
	return 1;
}

/******************************************************************************/
static void _fill_ann_(void);

static void _fill_ann_(void)
{
	admin_cmd_anndownload_t *pReq = (admin_cmd_anndownload_t*)(sAdmCtx.aRecvBuff);
	pReq->L7CommandId = ADM_ANNDOWNLOAD;
	*(uint16_t*)pReq->L7SwVersionIni = __htons( (uint16_t)(0x0000) );
	*(uint16_t*)pReq->L7SwVersionTarget = __htons( (uint16_t)(0x0001) );
	*(uint16_t*)pReq->L7MField = __htons((uint16_t)(0x7810));
	*(uint16_t*)pReq->L7DcHwId = __htons((uint16_t)(0x0010));
	*(uint16_t*)pReq->L7BlocksCount = __htons( (uint16_t)(100) );
	pReq->L7ChannelId = 120;
	pReq->L7ModulationId = PHY_WM2400;
	memset(pReq->L7Klog, 1, 16);

	time_t t;
	time(&t);
	t -= EPOCH_UNIX_TO_OURS;
	t = ( (t / 86400) + 1) *86400;
	*(uint32_t*)pReq->L7DaysProg = __htonl(t);

	pReq->L7DayRepeat = 3;
	pReq->L7DeltaSec = 5;

	// -----
	sAdmCtx.sCmdMsg.u8KeyId = KEY_CHG_ID;
	sAdmCtx.sCmdMsg.u8Size = sizeof(admin_cmd_anndownload_t);

	((admin_rsp_t*)(sAdmCtx.aSendBuff))->L7ResponseId = ADM_ANNDOWNLOAD;
	((admin_rsp_t*)(sAdmCtx.aSendBuff))->L7ErrorCode = ADM_NONE;
}

/******************************************************************************/

TEST_SETUP(WizeCoreApp_wizeapi)
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
	Param_GetAddOf_IgnoreAndReturn(0);

	// Key
	Crypto_WriteKey_IgnoreAndReturn(0);

	// FreeRTOS
	vPortYield_Ignore();
	vQueueAddToRegistry_Ignore();

	// Task creation and notify
	xTaskGetCurrentTaskHandle_IgnoreAndReturn((void*)0xFECA);
	xTaskCreateStatic_IgnoreAndReturn((void*)0xCAFE);
	xTaskGenericNotifyWait_IgnoreAndReturn(pdTRUE);
	xTaskGenericNotify_IgnoreAndReturn(pdTRUE);
	vTaskGenericNotifyGiveFromISR_Ignore();

	// Queues, semaphore, mutex
	xQueueGenericCreateStatic_IgnoreAndReturn((void*)0x1000); // xSemaphoreCreateBinaryStatic
	xQueueGenericSend_IgnoreAndReturn(1);
	xQueueSemaphoreTake_IgnoreAndReturn(1);

	// Mgr
	NetMgr_Setup_Ignore();
	TimeMgr_Main_IgnoreAndReturn(0);

	// --
	WizeApi_SesMgr_Setup(NULL, &sInstCtx, &sAdmCtx, &sDwnCtx);
}

TEST_TEAR_DOWN(WizeCoreApp_wizeapi)
{

}

/******************************************************************************/
/******************************************************************************/
TEST(WizeCoreApp_wizeapi, test_WizeApi_ApiSendLocked)
{
	uint8_t ret;
	uint8_t aData[10];
	uint8_t u8Size = 0;
	uint8_t bPrio = 0;

	xQueueSemaphoreTake_IgnoreAndReturn(0);
	ret = WizeApi_Send(aData, u8Size, bPrio);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WIZE_API_ACCESS_TIMEOUT, ret, "Return");
}

TEST(WizeCoreApp_wizeapi, test_WizeApi_ApiSendInvalid)
{
	uint8_t ret;
	uint8_t aData[10];
	uint8_t u8Size = 5;
	uint8_t bPrio = 0;

	Param_Access_StopIgnore();
	Param_Access_Stub(_Param_Access_Stub_);
	aParamValue[0] = 100;

	ret = WizeApi_Send(NULL, u8Size, bPrio);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WIZE_API_INVALID_PARAM, ret, "Return");

	ret = WizeApi_Send(aData, 255, bPrio);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WIZE_API_INVALID_PARAM, ret, "Return");
}

TEST(WizeCoreApp_wizeapi, test_WizeApi_ApiSendSuccess)
{
	uint8_t ret;
	uint8_t aData[10];
	uint8_t u8Size = 5;
	uint8_t bPrio = 0;

	xTaskGenericNotify_ExpectAndReturn( (void*)0xCAFE, 0, SES_EVT_ADM_OPEN, eSetBits, NULL, pdTRUE);

	ret = WizeApi_Send(aData, u8Size, bPrio);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WIZE_API_SUCCESS, ret, "Return");
}

/******************************************************************************/
TEST(WizeCoreApp_wizeapi, test_WizeApi_ApiExecPingLocked)
{
	uint8_t ret;
	uint8_t aData[10];
	uint8_t u8Size = 0;

	xQueueSemaphoreTake_IgnoreAndReturn(0);
	ret = WizeApi_ExecPing(aData, u8Size);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WIZE_API_ACCESS_TIMEOUT, ret, "Return");
}

TEST(WizeCoreApp_wizeapi, test_WizeApi_ApiExecPingInvalid)
{
	uint8_t ret;
	uint8_t aData[10];
	uint8_t u8Size = sizeof(inst_ping_t);

	ret = WizeApi_ExecPing(NULL, u8Size);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WIZE_API_INVALID_PARAM, ret, "Return");

	ret = WizeApi_ExecPing(aData, u8Size + 2);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WIZE_API_INVALID_PARAM, ret, "Return");
}

TEST(WizeCoreApp_wizeapi, test_WizeApi_ApiExecPingSuccess)
{
	uint8_t ret;
	uint8_t aData[10];
	uint8_t u8Size = sizeof(inst_ping_t);

	xTaskGenericNotify_ExpectAndReturn( (void*)0xCAFE, 0, SES_EVT_INST_OPEN, eSetBits, NULL, pdTRUE);

	ret = WizeApi_ExecPing(aData, u8Size);
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WIZE_API_SUCCESS, ret, "Return");
}

/******************************************************************************/
TEST(WizeCoreApp_wizeapi, test_WizeApi_ApiDownloadLocked)
{
	uint8_t ret;

	xQueueSemaphoreTake_IgnoreAndReturn(0);
	ret = WizeApi_Download();
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WIZE_API_ACCESS_TIMEOUT, ret, "Return");
}

TEST(WizeCoreApp_wizeapi, test_WizeApi_ApiDownloadInvalid)
{
	uint8_t ret;

	ret = WizeApi_Download();
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WIZE_API_INVALID_PARAM, ret, "Return");

	ret = WizeApi_Download();
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WIZE_API_INVALID_PARAM, ret, "Return");
}

TEST(WizeCoreApp_wizeapi, test_WizeApi_ApiDownloadSuccess)
{
	uint8_t ret;

	xTaskGenericNotify_ExpectAndReturn( (void*)0xCAFE, 0, SES_EVT_DWN_OPEN, eSetBits, NULL, pdTRUE);

	_fill_ann_();

	ret = WizeApi_Download();
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(WIZE_API_SUCCESS, ret, "Return");
}

/******************************************************************************/
/******************************************************************************/

