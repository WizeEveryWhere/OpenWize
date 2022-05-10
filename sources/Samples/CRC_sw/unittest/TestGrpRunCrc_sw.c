#include "unity_fixture.h"

TEST_GROUP_RUNNER(Samples_CRC_sw)
{
    RUN_TEST_CASE(Samples_CRC_sw, test_CRC_Compute_Success);
    RUN_TEST_CASE(Samples_CRC_sw, test_CRC_Compute_NullPointer);
    RUN_TEST_CASE(Samples_CRC_sw, test_CRC_Check_Success);
    RUN_TEST_CASE(Samples_CRC_sw, test_CRC_Check_Fail);
}
