#include "unity_fixture.h"

TEST_GROUP_RUNNER(WizeCore_net)
{
    RUN_TEST_CASE(WizeCore_net, test_NetApi_Setup);
    RUN_TEST_CASE(WizeCore_net, test_NetApi_Init);
    RUN_TEST_CASE(WizeCore_net, test_NetApi_Uinit);
    RUN_TEST_CASE(WizeCore_net, test_NetApi_Ioctl);
    RUN_TEST_CASE(WizeCore_net, test_NetApi_Send);
    RUN_TEST_CASE(WizeCore_net, test_NetApi_Recv);
    RUN_TEST_CASE(WizeCore_net, test_NetApi_Listen);
    RUN_TEST_CASE(WizeCore_net, test_NetApi_CallBack);
}
