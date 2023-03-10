#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(WizeCoreMgr_net);
#include "net_mgr.h"

// Mock WizeCore
#include "mock_net_api_private.h"

// Mock Samples
#include "mock_time_evt.h"
#include "mock_logger.h"

// Mock FreeRTOS
#include "mock_queue.h"
#include "mock_task.h"
#include "mock_portmacro.h"

/******************************************************************************/
extern uint8_t gLoggerLevel;

struct phydev_s sPhyDev;
wize_net_t sWizeNet;

/******************************************************************************/
#define OPEN_AND_CLOSE_TESTED 4
static uint32_t open_close_tested = 0;

#define IS_OPEN_CLOSE_TESTED() ( open_close_tested >= OPEN_AND_CLOSE_TESTED )
#define OPEN_CLOSE_TEST() open_close_tested++

static net_listen_type_e eListenType;
static uint32_t u32Tmo;
static net_msg_t xNetMsg;
static uint8_t aData[256] = "This is my data to send";

netdev_err_type_e eNetDevErrType;
int32_t _WizeNet_Send_err_stub_(netdev_t* pNetdev, net_msg_t *pNetMsg, int cmock_num_calls)
{
	pNetdev->eErrType = eNetDevErrType;
	return ( (eNetDevErrType == NETDEV_ERROR_NONE)?(NETDEV_STATUS_OK):(NETDEV_STATUS_ERROR) );
}

int32_t _WizeNet_Listen_err_stub_(netdev_t* pNetdev, int cmock_num_calls)
{
	pNetdev->eErrType = eNetDevErrType;
	return ( (eNetDevErrType == NETDEV_ERROR_NONE)?(NETDEV_STATUS_OK):(NETDEV_STATUS_ERROR) );
}

int32_t _WizeNet_Setup_stub_(netdev_t* pNetdev, wize_net_t* pWizeCtx, phydev_t *pPhydev, int cmock_num_calls)
{
	int32_t i32Ret = NETDEV_STATUS_ERROR;
    if (pNetdev && pWizeCtx && pPhydev)
    {
    	pNetdev->pCtx = pWizeCtx;
    	pNetdev->pPhydev = pPhydev;
    	pNetdev->eState = NETDEV_STATE_UNKWON;
    	i32Ret = NETDEV_STATUS_OK;
    }
	return i32Ret;
}

static void init_net_msg(void)
{
	eListenType = NET_LISTEN_TYPE_ONE;
	u32Tmo = 5;
	xNetMsg.pData = aData;
	xNetMsg.u8Size = (uint8_t)(sizeof(aData));
	xNetMsg.u8Type = APP_DATA;
}

/******************************************************************************/

TEST_SETUP(WizeCoreMgr_net)
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
	xQueueCreateMutexStatic_IgnoreAndReturn((void*)0x2000);
	xQueueGenericSend_IgnoreAndReturn(1);
	xQueueSemaphoreTake_IgnoreAndReturn(1);

	// WizeCore net

	//WizeNet_Setup_IgnoreAndReturn(NETDEV_STATUS_OK);
	WizeNet_Setup_Stub(_WizeNet_Setup_stub_);

	WizeNet_Init_IgnoreAndReturn(NETDEV_STATUS_OK);
	WizeNet_Uninit_IgnoreAndReturn(NETDEV_STATUS_OK);
	WizeNet_Send_IgnoreAndReturn(NETDEV_STATUS_OK);
	WizeNet_Recv_IgnoreAndReturn(NETDEV_STATUS_OK);
	WizeNet_Listen_IgnoreAndReturn(NETDEV_STATUS_OK);
	WizeNet_Ioctl_IgnoreAndReturn(NETDEV_STATUS_OK);

	eNetDevErrType = NETDEV_ERROR_NONE;

	// Open and Close are tested
	if ( IS_OPEN_CLOSE_TESTED() )
	{
		NetMgr_Setup(&sPhyDev, &sWizeNet);
		NetMgr_Open((void*)0xCAFE);
	}
}

TEST_TEAR_DOWN(WizeCoreMgr_net)
{
	// Open and Close are tested
	if ( IS_OPEN_CLOSE_TESTED() )
	{
		NetMgr_Close();
	}
}

/******************************************************************************/
TEST(WizeCoreMgr_net, test_NetMgr_Init_NullPtr)
{
	int32_t ret;
	// ---
	ret = NetMgr_Init();
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);
}

TEST(WizeCoreMgr_net, test_NetMgr_Init_TimerInitFail)
{
	int32_t ret;
	// ---

	//TimeEvt_TimerInit_ExpectAndReturn(NULL, (void*)0xCAFE, TIMEEVT_CFG_ONESHOT, 1);
	//TimeEvt_TimerInit_IgnoreArg_pTimeEvt();

	TimeEvt_TimerInit_IgnoreAndReturn(1);

	NetMgr_Setup(&sPhyDev, &sWizeNet);
	ret = NetMgr_Init();
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);
}

TEST(WizeCoreMgr_net, test_NetMgr_Init_NetInitFail)
{
	int32_t ret;
	// ---
	WizeNet_Init_IgnoreAndReturn(NETDEV_STATUS_ERROR);
	NetMgr_Setup(&sPhyDev, &sWizeNet);
	ret = NetMgr_Init();
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);
}

/******************************************************************************/
TEST(WizeCoreMgr_net, test_NetMgr_Uninit_NullPtr)
{
	int32_t ret;
	// ---
	ret = NetMgr_Uninit();
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);
}

TEST(WizeCoreMgr_net, test_NetMgr_Uninit_NetUninitFail)
{
	int32_t ret;
	// ---
	WizeNet_Uninit_IgnoreAndReturn(NETDEV_STATUS_ERROR);
	NetMgr_Setup(&sPhyDev, &sWizeNet);
	ret = NetMgr_Uninit();
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);
}

/******************************************************************************/
TEST(WizeCoreMgr_net, test_NetMgr_Open_NetMgrInitFail)
{
	int32_t ret;
	// ---
	NetMgr_Setup(&sPhyDev, &sWizeNet);
	// NetMgr_Init fail
	WizeNet_Init_IgnoreAndReturn(NETDEV_STATUS_ERROR);
	ret = NetMgr_Open((void*)0xCAFE);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);
	// ---
	OPEN_CLOSE_TEST();
}

TEST(WizeCoreMgr_net, test_NetMgr_Open_AcquireFail)
{
	int32_t ret;
	// ---
	NetMgr_Setup(&sPhyDev, &sWizeNet);
	// Fail to take semaphore
	xQueueSemaphoreTake_IgnoreAndReturn(0);
	ret = NetMgr_Open((void*)0xCAFE);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);
	// ---
	OPEN_CLOSE_TEST();
}
/******************************************************************************/
TEST(WizeCoreMgr_net, test_NetMgr_Close_NotOwner)
{
	int32_t ret;
	// ---
	NetMgr_Setup(&sPhyDev, &sWizeNet);
	ret = NetMgr_Open((void*)0xCAFE);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_OK, ret);

	// Not the owner
	xTaskGetCurrentTaskHandle_IgnoreAndReturn((void*)0xAAAA);
	ret = NetMgr_Close();
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);
	// ---
	OPEN_CLOSE_TEST();
}

TEST(WizeCoreMgr_net, test_NetMgr_Close_LockFail)
{
	int32_t ret;
	// ---
	NetMgr_Setup(&sPhyDev, &sWizeNet);
	ret = NetMgr_Open((void*)0xCAFE);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_OK, ret);

	// Fail to lock NetDev
	xQueueSemaphoreTake_IgnoreAndReturn(0);
	ret = NetMgr_Close();
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);
	// ---
	OPEN_CLOSE_TEST();
}

/******************************************************************************/
TEST(WizeCoreMgr_net, test_NetMgr_SetUpDwLinkIoctl_NotOwner)
{
#ifdef NET_MGR_OWNER_IS_CALLER
	int32_t ret;
	// ---
	// Not the owner
	phy_chan_e eCh = PHY_CH100;
	phy_mod_e eMod = PHY_WM2400;
	uint32_t eCtl = NETDEV_CTL_SET_DWLINK_CH;
	uint32_t _args = (uint32_t)eCh;

	xTaskGetCurrentTaskHandle_IgnoreAndReturn((void*)0xAAAA);

	ret = NetMgr_SetUplink(eCh, eMod);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);

	ret = NetMgr_SetDwlink(eCh, eMod);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);

	ret = NetMgr_Ioctl(eCtl, _args);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);

#endif
}

TEST(WizeCoreMgr_net, test_NetMgr_SetUpDwLinkIoctl_LockFail)
{
	int32_t ret;
	// ---
	// Fail to lock
	phy_chan_e eCh = PHY_CH100;
	phy_mod_e eMod = PHY_WM2400;
	uint32_t eCtl = NETDEV_CTL_SET_DWLINK_CH;
	uint32_t _args = (uint32_t)eCh;

	xQueueSemaphoreTake_IgnoreAndReturn(0);

	ret = NetMgr_SetUplink(eCh, eMod);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);

	ret = NetMgr_SetDwlink(eCh, eMod);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);

	ret = NetMgr_Ioctl(eCtl, _args);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);
}

TEST(WizeCoreMgr_net, test_NetMgr_SetUpDwLinkIoctl_WizeNetIoctlFail)
{
	int32_t ret;
	// ---
	// WizeNet_Ioctl fail
	phy_chan_e eCh = PHY_CH100;
	phy_mod_e eMod = PHY_WM2400;
	uint32_t eCtl = NETDEV_CTL_SET_DWLINK_CH;
	uint32_t _args = (uint32_t)eCh;

	WizeNet_Ioctl_IgnoreAndReturn(NETDEV_STATUS_ERROR);

	ret = NetMgr_SetUplink(eCh, eMod);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);

	ret = NetMgr_SetDwlink(eCh, eMod);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);

	ret = NetMgr_Ioctl(eCtl, _args);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);
}

/******************************************************************************/

TEST(WizeCoreMgr_net, test_NetMgr_Send_NullPtr)
{
	int32_t ret;
	// ---
	init_net_msg();

	// Null ptr
	ret = NetMgr_Send(NULL, u32Tmo);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);

	xNetMsg.pData = NULL;
	ret = NetMgr_Send(&xNetMsg, u32Tmo);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);
}

TEST(WizeCoreMgr_net, test_NetMgr_Send_BadType)
{
	int32_t ret;
	// ---
	init_net_msg();

	// Bad type
	xNetMsg.u8Type = APP_TYPE_NB;
	ret = NetMgr_Send(&xNetMsg, u32Tmo);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);
}

TEST(WizeCoreMgr_net, test_NetMgr_Send_NotOwner)
{
#ifdef NET_MGR_OWNER_IS_CALLER
	int32_t ret;
	// ---
	init_net_msg();
	// Not owner
	xTaskGetCurrentTaskHandle_IgnoreAndReturn((void*)0xAAAA);

	ret = NetMgr_Send(&xNetMsg, u32Tmo);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);
#endif
}

TEST(WizeCoreMgr_net, test_NetMgr_Send_LockFail)
{
	int32_t ret;
	// ---
	init_net_msg();
	// Lock fail
	xQueueSemaphoreTake_IgnoreAndReturn(0);
	ret = NetMgr_Send(&xNetMsg, u32Tmo);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);
}

TEST(WizeCoreMgr_net, test_NetMgr_Send_RetryFail)
{
	int32_t ret;
	// ---
	init_net_msg();

	WizeNet_Send_StopIgnore();
	WizeNet_Send_Stub(_WizeNet_Send_err_stub_);

	// ReTry fail due to NETDEV_ERROR_PROTO
	eNetDevErrType = NETDEV_ERROR_PROTO;
	ret = NetMgr_Send(&xNetMsg, u32Tmo);
	TEST_ASSERT_EQUAL_INT32(NETDEV_STATUS_ERROR, ret);

	// ReTry fail due to NETDEV_ERROR_PHY
	eNetDevErrType = NETDEV_ERROR_PHY;
	ret = NetMgr_Send(&xNetMsg, u32Tmo);
	TEST_ASSERT_EQUAL_INT32(NETDEV_STATUS_ERROR, ret);
}

TEST(WizeCoreMgr_net, test_NetMgr_Send_TimerStartFail)
{
	int32_t ret;
	// ---
	init_net_msg();

	// TimerStart fail
	TimeEvt_TimerStart_IgnoreAndReturn(1);
	ret = NetMgr_Send(&xNetMsg, u32Tmo);
	TEST_ASSERT_EQUAL_INT32(NETDEV_STATUS_ERROR, ret);
}

/******************************************************************************/

TEST(WizeCoreMgr_net, test_NetMgr_Listen_NullPtr)
{
	int32_t ret;
	// ---
	init_net_msg();

	// Null ptr
	ret = NetMgr_Listen(NULL, u32Tmo, eListenType);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);

	xNetMsg.pData = NULL;
	ret = NetMgr_Listen(&xNetMsg, u32Tmo, eListenType);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);
}

TEST(WizeCoreMgr_net, test_NetMgr_Listen_BadType)
{
	int32_t ret;
	// ---
	init_net_msg();

	// Bad type
	xNetMsg.u8Type = APP_TYPE_NB;
	ret = NetMgr_Listen(&xNetMsg, u32Tmo, eListenType);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_ERROR, ret);
}

TEST(WizeCoreMgr_net, test_NetMgr_Listen_NotOwner)
{
#ifdef NET_MGR_OWNER_IS_CALLER
	int32_t ret;
	// ---
	init_net_msg();
	// Not owner
	xTaskGetCurrentTaskHandle_IgnoreAndReturn((void*)0xAAAA);

	ret = NetMgr_Listen(&xNetMsg, u32Tmo, eListenType);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);
#endif
}

TEST(WizeCoreMgr_net, test_NetMgr_Listen_LockFail)
{
	int32_t ret;
	// ---
	init_net_msg();
	// Lock fail
	xQueueSemaphoreTake_IgnoreAndReturn(0);
	ret = NetMgr_Listen(&xNetMsg, u32Tmo, eListenType);
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);
}

TEST(WizeCoreMgr_net, test_NetMgr_Listen_RetryFail)
{
	int32_t ret;
	// ---
	init_net_msg();

	WizeNet_Listen_StopIgnore();
	WizeNet_Listen_Stub(_WizeNet_Listen_err_stub_);

	// ReTry fail due to NETDEV_ERROR_PROTO
	eNetDevErrType = NETDEV_ERROR_PROTO;
	ret = NetMgr_Listen(&xNetMsg, u32Tmo, eListenType);
	TEST_ASSERT_EQUAL_INT32(NETDEV_STATUS_ERROR, ret);

	// ReTry fail due to NETDEV_ERROR_PHY
	eNetDevErrType = NETDEV_ERROR_PHY;
	ret = NetMgr_Listen(&xNetMsg, u32Tmo, eListenType);
	TEST_ASSERT_EQUAL_INT32(NETDEV_STATUS_ERROR, ret);
}

TEST(WizeCoreMgr_net, test_NetMgr_Listen_TimerStartFail)
{
	int32_t ret;
	// ---
	init_net_msg();

	// TimerStart fail
	TimeEvt_TimerStart_IgnoreAndReturn(1);
	ret = NetMgr_Listen(&xNetMsg, u32Tmo, eListenType);
	TEST_ASSERT_EQUAL_INT32(NETDEV_STATUS_ERROR, ret);
}

/******************************************************************************/
TEST(WizeCoreMgr_net, test_NetMgr_ListenReady_NotOwnerNorCaller)
{
	int32_t ret;
	// ---
	init_net_msg();

	// Not owner nor Caller
	xTaskGetCurrentTaskHandle_IgnoreAndReturn((void*)0xAAAA);

	ret = NetMgr_ListenReady();
	TEST_ASSERT_EQUAL_INT32(NET_STATUS_BUSY, ret);
}
