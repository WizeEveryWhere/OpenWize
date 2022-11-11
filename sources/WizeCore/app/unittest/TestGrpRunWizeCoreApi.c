#include "unity_fixture.h"

uint8_t gLoggerLevel;

TEST_GROUP_RUNNER(WizeCoreApp_wizeapi)
{
	RUN_TEST_CASE(WizeCoreApp_wizeapi, test_WizeApi_ApiSendLocked);
	RUN_TEST_CASE(WizeCoreApp_wizeapi, test_WizeApi_ApiSendInvalid);
	RUN_TEST_CASE(WizeCoreApp_wizeapi, test_WizeApi_ApiSendSuccess);
	RUN_TEST_CASE(WizeCoreApp_wizeapi, test_WizeApi_ApiExecPingLocked);
	RUN_TEST_CASE(WizeCoreApp_wizeapi, test_WizeApi_ApiExecPingInvalid);
	RUN_TEST_CASE(WizeCoreApp_wizeapi, test_WizeApi_ApiExecPingSuccess);
	RUN_TEST_CASE(WizeCoreApp_wizeapi, test_WizeApi_ApiDownloadLocked);
	RUN_TEST_CASE(WizeCoreApp_wizeapi, test_WizeApi_ApiDownloadInvalid);
	RUN_TEST_CASE(WizeCoreApp_wizeapi, test_WizeApi_ApiDownloadSuccess);
}
