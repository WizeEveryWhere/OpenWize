#include "unity_fixture.h"

TEST_GROUP_RUNNER(Samples_Decomp)
{
	RUN_TEST_CASE(Samples_Decomp, test_Decomp_InStream_EOF);
	RUN_TEST_CASE(Samples_Decomp, test_Decomp_OutStream_EOF);

	RUN_TEST_CASE(Samples_Decomp, test_Decomp_Allocator_Fail);
	RUN_TEST_CASE(Samples_Decomp, test_Decomp_AllocateDictSize_Fail);
	RUN_TEST_CASE(Samples_Decomp, test_Decomp_AllocateCntSize_Fail);
	RUN_TEST_CASE(Samples_Decomp, test_Decomp_TooLargeFile_Fail);

	RUN_TEST_CASE(Samples_Decomp, test_Decomp_Decode_Fail);
    RUN_TEST_CASE(Samples_Decomp, test_Decomp_Small_Success);
}
