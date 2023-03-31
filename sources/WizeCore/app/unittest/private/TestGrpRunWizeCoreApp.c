#include "unity_fixture.h"

uint8_t gLoggerLevel;

TEST_GROUP_RUNNER(WizeCoreApp_sesdisp)
{
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_Setup);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_InitDisable);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_InitEnable);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_RejectSame_INSTSession);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_RejectSame_ADMSession);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_RejectSame_DWNSession);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_RejectMutEx_ADMDuringINST);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_RejectMutEx_INSTDuringADM);

	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_RejectNoTime_INSTDuringDWNListening);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_RejectNoTime_INSTDuringDWNWaiting);

	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_RejectNoTime_ADMDuringDWNListening);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_RejectNoTime_ADMallDuringDWNWaiting);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_RejectNoTime_ADMcmdrspDuringDWNWaiting);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_RejectNoTime_ADMrspDuringDWNWaiting);

	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_NetMgrError);

	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_ActiveINSTGetNetEvent);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_ActiveADMGetNetEvent);
	RUN_TEST_CASE(WizeCoreApp_sesdisp, test_SesDisp_ActiveDWNGetNetEvent);

}

TEST_GROUP_RUNNER(WizeCoreApp_wizeapp)
{
	RUN_TEST_CASE(WizeCoreApp_wizeapp, test_WizeApp_Init);
	RUN_TEST_CASE(WizeCoreApp_wizeapp, test_WizeApp_CommonXXX);
	RUN_TEST_CASE(WizeCoreApp_wizeapp, test_WizeApp_TimeXXX);

	RUN_TEST_CASE(WizeCoreApp_wizeapp, test_WizeApp_AnnReadyNoPendRSP);
	RUN_TEST_CASE(WizeCoreApp_wizeapp, test_WizeApp_AnnReadyNotAnn);
	RUN_TEST_CASE(WizeCoreApp_wizeapp, test_WizeApp_AnnReadyIdMismatch);
	RUN_TEST_CASE(WizeCoreApp_wizeapp, test_WizeApp_AnnReadyHasErrCode);
}
