#include "unity_fixture.h"


TEST_GROUP_RUNNER(WizeCoreAppInt_inst)
{
	RUN_TEST_CASE(WizeCoreAppInt_inst, test_InstInt_Init);
	RUN_TEST_CASE(WizeCoreAppInt_inst, test_InstInt_Add);
	RUN_TEST_CASE(WizeCoreAppInt_inst, test_InstInt_End);
}

TEST_GROUP_RUNNER(WizeCoreAppInt_adm)
{
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_UnknownCmd);

	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_ReadInvalidParam);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_ReadLenExceed);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_ReadAcessDeniedWO);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_ReadAcessDeniedNA);

	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_WriteInvalidParam);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_WriteAcessDeniedRO);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_WriteAcessDeniedNA);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_WriteNotConform);

	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_WriteKeyBadFrmLen);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_WriteKeyWrongCiphKey);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_WriteKeyBadId);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_WriteKeyBadIdByPassKeyId);

	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBadFrmLen);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnLocalUpadtePending);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnMFieldMismatch);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBadKeyId);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBadChannel);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBadModulation);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnDayRepeatOutOfScope);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnDeltaSecOutOfScope);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBcastDayOutOfScopeLower);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBcastDayOutOfScopeUpper);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBcastWindowDurationOutOfScope);

	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBcastWindowOutOfScopeMntBeg);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBcastWindowOutOfScopeMntEnd);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBcastWindowOutOfScopeMnt);

	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBadNbOfBlock);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBadTargetHW);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBadInitFW);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBadTargetFWLower);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_AnnBadTargetFWEqual);

	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_PreCmdUNKOWN);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_PreCmdREAD_PARAM);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_PreCmdWRITE_PARAM);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_PreCmdEXECINSTPING);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_PreCmdWRITE_KEY);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_PreCmdANNDOWNLOAD);
	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_PreCmdAlreadyTreat);

	RUN_TEST_CASE(WizeCoreAppInt_adm, test_AdmInt_PostCmd);

}
