#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(WizeCore_instmgr);
#include "inst_mgr.h"

#include "mock_.h"

rtos
	xSemaphoreCreateMutexStatic

logger
	LOG_INF
	LOG_DBG
	LOG_WRN

net_mgr
	NetMgr_ListenReady
	NetMgr_Send
	NetMgr_Listen

time_evt
	TimeEvt_TimerInit
	TimeEvt_TimerStop
	TimeEvt_TimerStart

/******************************************************************************/


/******************************************************************************/

TEST_SETUP(WizeCore_instmgr)
{

}
TEST_TEAR_DOWN(WizeCore_instmgr)
{

}

/******************************************************************************/

TEST(WizeCore_instmgr, test_InstMgr_xxx)
{

}
