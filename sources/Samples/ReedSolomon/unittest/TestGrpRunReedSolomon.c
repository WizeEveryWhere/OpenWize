#include "unity_fixture.h"

TEST_GROUP_RUNNER(Samples_ReedSolomon)
{
    RUN_TEST_CASE(Samples_ReedSolomon, test_RS_GenerateTablesCheck);

    RUN_TEST_CASE(Samples_ReedSolomon, test_RS_Decode_1ErrorMessage);
    RUN_TEST_CASE(Samples_ReedSolomon, test_RS_Decode_1ErrorParity);

    RUN_TEST_CASE(Samples_ReedSolomon, test_RS_Decode_16ErrorMessage);
    RUN_TEST_CASE(Samples_ReedSolomon, test_RS_Decode_16ErrorParity);
    RUN_TEST_CASE(Samples_ReedSolomon, test_RS_Decode_16ErrorMessageAndParity);

    RUN_TEST_CASE(Samples_ReedSolomon, test_RS_Decode_17ErrorMessageAndParity);

    RUN_TEST_CASE(Samples_ReedSolomon, test_RS_Decode_PseudoRealFrame);
    RUN_TEST_CASE(Samples_ReedSolomon, test_RS_Decode_PseudoRealFrameWithError);

}
