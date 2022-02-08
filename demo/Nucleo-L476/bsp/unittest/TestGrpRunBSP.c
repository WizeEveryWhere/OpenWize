#include "unity_fixture.h"

TEST_GROUP_RUNNER(NucleoL476_bspLP)
{
    RUN_TEST_CASE(NucleoL476_bspLP, test_BSP_Sleep_Mode_Success);
    RUN_TEST_CASE(NucleoL476_bspLP, test_BSP_Stop_2_Mode_Success);
    RUN_TEST_CASE(NucleoL476_bspLP, test_BSP_Standby_Mode_Success);
    RUN_TEST_CASE(NucleoL476_bspLP, test_BSP_Shutdown_Mode_Success);
}

TEST_GROUP_RUNNER(NucleoL476_bspRTC)
{
    RUN_TEST_CASE(NucleoL476_bspRTC, test_BSP_RTC_Time_Test);
    RUN_TEST_CASE(NucleoL476_bspRTC, test_BSP_RTC_WakeUpTimer_Test);
    RUN_TEST_CASE(NucleoL476_bspRTC, test_BSP_RTC_UpdateTime_Test);
}

TEST_GROUP_RUNNER(NucleoL476_bspUART)
{
    RUN_TEST_CASE(NucleoL476_bspUART, test_BSP_Recv_Address_Success);
    RUN_TEST_CASE(NucleoL476_bspUART, test_BSP_Recv_w_Address_Success);
    RUN_TEST_CASE(NucleoL476_bspUART, test_BSP_Send_w_Address_Success);
}
