#include "unity_fixture.h"

TEST_GROUP_RUNNER(Samples_TimeEvtInt)
{
	RUN_TEST_CASE(Samples_TimeEvtInt, test_TimeEvtInt_EventCtx_InsertTest);
	RUN_TEST_CASE(Samples_TimeEvtInt, test_TimeEvtInt_EventCtx_RemoveTest);
	RUN_TEST_CASE(Samples_TimeEvtInt, test_TimeEvtInt_EventCtx_UpdateTest);
}

TEST_GROUP_RUNNER(Samples_TimeEvt)
{
	RUN_TEST_CASE(Samples_TimeEvt, test_TimeEvt_TimerInit_failed);
	RUN_TEST_CASE(Samples_TimeEvt, test_TimeEvt_TimerStart_failed);

	RUN_TEST_CASE(Samples_TimeEvt, test_TimeEvt_CheckIncDecTime);
	RUN_TEST_CASE(Samples_TimeEvt, test_TimeEvt_CheckPeriodicReArm);
	RUN_TEST_CASE(Samples_TimeEvt, test_TimeEvt_CheckUpdateTime);
	RUN_TEST_CASE(Samples_TimeEvt, test_TimeEvt_CheckLongTreatTime);

	RUN_TEST_CASE(Samples_TimeEvt, test_TimeEvt_OneTimer_OneShot);
}
