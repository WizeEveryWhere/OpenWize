#include "unity_fixture.h"

TEST_GROUP_RUNNER(WizeCore_proto)
{
	// Test on call to Wize_ProtoBuild
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_NullPtr);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_BadSize);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_AppTypeMismatch);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_CryptoFailed);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_HKencFailed);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_HKmacFailed);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_CrcFailed);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_L6AppGiven);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_L6AppDefault);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_L6Cpt);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_Cfield);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_CheckLField);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Build_CheckOtherContent);

    // Test on call to Wize_ProtoExtract
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Extract_NullPtr);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_Extract_BadSize);

    // Test on call to Wize_ProtoExtract with download frame
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractDwn_RSDecodeFailed);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractDwn_CRCComputeFailed);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractDwn_CRCCheckErr);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractDwn_BadL2DownID);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractDwn_BadL6DownVer);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractDwn_HklogError);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractDwn_HklogMismatch);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractDwn_UncipherError);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractDwn_CheckOtherContent);

    // Test on call to Wize_ProtoExtract with exchange frame
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_AFieldMismatch);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_MFieldMismatch);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_CRCComputeFailed);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_CRCCheckError);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_CiFieldMismatch);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_L6VersMismatch);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_L6NetwIdMismatch);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_CFieldMismatch);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_HKencFailed);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_HKencMismatch);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_HKmacFailed);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_HKmacMismatch);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_UnciphFailed);
    // not possible case : RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_UnknownKid);
    // noting more RUN_TEST_CASE(WizeCore_proto, test_Proto_ExtractExch_CheckOtherContent);

    // Test on call to Wize_ProtoStatsxx
    RUN_TEST_CASE(WizeCore_proto, test_Proto_StatRxUpdate_Success);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_StatsTxUpdate_Success);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_StatsRxClear_Success);
    RUN_TEST_CASE(WizeCore_proto, test_Proto_StatsTxClear_Success);
}
