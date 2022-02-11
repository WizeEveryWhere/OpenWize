#include "unity_fixture.h"

TEST_GROUP_RUNNER(Samples_Parameters)
{
    RUN_TEST_CASE(Samples_Parameters, test_Param_GetSize_Success);
    RUN_TEST_CASE(Samples_Parameters, test_Param_GetLocAccess_Success);
    RUN_TEST_CASE(Samples_Parameters, test_Param_GetRemAccess_Success);
    RUN_TEST_CASE(Samples_Parameters, test_Param_GetEffect_Success);
    RUN_TEST_CASE(Samples_Parameters, test_Param_GetRestrId_Success);

    RUN_TEST_CASE(Samples_Parameters, test_LocalReadAccess_Success);
    RUN_TEST_CASE(Samples_Parameters, test_LocalWriteAccess_Success);

    RUN_TEST_CASE(Samples_Parameters, test_LocalReadAccess_Forbidden);
    RUN_TEST_CASE(Samples_Parameters, test_LocalWriteAccess_Forbidden);

    RUN_TEST_CASE(Samples_Parameters, test_RemoteReadAccess_Success);
    RUN_TEST_CASE(Samples_Parameters, test_RemoteWriteAccess_Success);

    RUN_TEST_CASE(Samples_Parameters, test_RemoteReadAccess_Forbidden);
    RUN_TEST_CASE(Samples_Parameters, test_RemoteWriteAccess_Forbidden);

    RUN_TEST_CASE(Samples_Parameters, test_DirectReadAccess_Success);
    RUN_TEST_CASE(Samples_Parameters, test_DirectWriteAccess_Success);

    RUN_TEST_CASE(Samples_Parameters, test_Param_CheckConformityRange_Success);
    RUN_TEST_CASE(Samples_Parameters, test_Param_CheckConformityRange_Forbidden);

    RUN_TEST_CASE(Samples_Parameters, test_Param_CheckConformityEnum_Success);
    RUN_TEST_CASE(Samples_Parameters, test_Param_CheckConformityEnum_Forbidden);

    RUN_TEST_CASE(Samples_Parameters, test_Param_CheckConformityModulo_Success);
    RUN_TEST_CASE(Samples_Parameters, test_Param_CheckConformityModulo_Forbidden);
}
