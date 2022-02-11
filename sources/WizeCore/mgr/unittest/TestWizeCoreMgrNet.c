#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(WizeCore_netmgr);
#include "net_mgr.h"
/*
#include "mock_.h"
rtos
	xTaskCreateStatic
	xEventGroupCreateStatic
	xSemaphoreCreateBinaryStatic
	xSemaphoreCreateMutexStatic
	xSemaphoreTake
	xSemaphoreGive
	xTaskGetCurrentTaskHandle
	xTaskNotify
	xTaskNotifyWait
	xTaskNotifyFromISR
	portYIELD_FROM_ISR

logger
	LOG_DBG
	LOG_ERR
	LOG_WRN
	LOG_FRM_IN
	LOG_FRM_OUT

net_api
	WizeNet_Init
	WizeNet_Uninit
	WizeNet_Ioctl
	WizeNet_Recv
	WizeNet_Send
	WizeNet_Listen


time_evt
	TimeEvt_TimerInit
	TimeEvt_TimerStop
	TimeEvt_TimerStart

*/
/******************************************************************************/


/******************************************************************************/

TEST_SETUP(WizeCore_netmgr)
{

}
TEST_TEAR_DOWN(WizeCore_netmgr)
{

}

/******************************************************************************/

TEST(WizeCore_netmgr, test_NetMgr_xxx)
{

}
