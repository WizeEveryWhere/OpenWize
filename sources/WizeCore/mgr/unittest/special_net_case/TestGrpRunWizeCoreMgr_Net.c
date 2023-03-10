#include "unity_fixture.h"

uint8_t gLoggerLevel;

TEST_GROUP_RUNNER(WizeCoreMgr_net)
{
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Init_NullPtr);
    /*
     * Note :
     * The test_NetMgr_Uninit_NullPtr test must be call before the very first
     * call to WizeNet_Setup which is "subbed" to _WizeNet_Setup_stub_. This last
     * will set the pNetdev->pCtx and pNetdev->pPhydev to non NULL pointer
     */
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Uninit_NullPtr);

    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Init_TimerInitFail);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Init_NetInitFail);

    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Uninit_NetUninitFail);

    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Open_NetMgrInitFail);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Open_AcquireFail);

    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Close_NotOwner);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Close_LockFail);

    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_SetUpDwLinkIoctl_NotOwner);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_SetUpDwLinkIoctl_LockFail);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_SetUpDwLinkIoctl_WizeNetIoctlFail);

    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Send_NullPtr);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Send_BadType);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Send_NotOwner);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Send_LockFail);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Send_RetryFail);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Send_TimerStartFail);

    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Listen_NullPtr);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Listen_BadType);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Listen_NotOwner);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Listen_LockFail);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Listen_RetryFail);
    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_Listen_TimerStartFail);

    RUN_TEST_CASE(WizeCoreMgr_net, test_NetMgr_ListenReady_NotOwnerNorCaller);
}

