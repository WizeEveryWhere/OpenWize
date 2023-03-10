#include "unity_fixture.h"

uint8_t gLoggerLevel;

TEST_GROUP_RUNNER(WizeCoreMgr_time)
{
    RUN_TEST_CASE(WizeCoreMgr_time, test_TimeMgr_CurValChanged);
    RUN_TEST_CASE(WizeCoreMgr_time, test_TimeMgr_DayPassNoCorrection);
    RUN_TEST_CASE(WizeCoreMgr_time, test_TimeMgr_DayPassClockCorrection);
    RUN_TEST_CASE(WizeCoreMgr_time, test_TimeMgr_DayPassOffsetCorrection);
    RUN_TEST_CASE(WizeCoreMgr_time, test_TimeMgr_DayPassDriftCorrection);
}


TEST_GROUP_RUNNER(WizeCoreMgr_inst)
{
    RUN_TEST_CASE(WizeCoreMgr_inst, test_InstMgr_Init);
    RUN_TEST_CASE(WizeCoreMgr_inst, test_InstMgr_FsmIdleState_SendError);
    RUN_TEST_CASE(WizeCoreMgr_inst, test_InstMgr_FsmSendingState_TimerStartError);
    RUN_TEST_CASE(WizeCoreMgr_inst, test_InstMgr_FsmSendingState_TimeOut);
    RUN_TEST_CASE(WizeCoreMgr_inst, test_InstMgr_FsmWaitingState_ListenError);
    RUN_TEST_CASE(WizeCoreMgr_inst, test_InstMgr_FsmWaitingState_ListenReadyError);
    RUN_TEST_CASE(WizeCoreMgr_inst, test_InstMgr_FsmListeningState_Timeout);
    RUN_TEST_CASE(WizeCoreMgr_inst, test_InstMgr_FsmListeningState_LostMsg);
    RUN_TEST_CASE(WizeCoreMgr_inst, test_InstMgr_FsmSendingState_SendOk);
    RUN_TEST_CASE(WizeCoreMgr_inst, test_InstMgr_Fsm_ProcessCanceled);
    RUN_TEST_CASE(WizeCoreMgr_inst, test_InstMgr_Fsm_ProcessOk);
}

TEST_GROUP_RUNNER(WizeCoreMgr_adm)
{
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_Init);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_FsmIdleState_SendError);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_FsmWaitingRXState_ByPassCmd);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_FsmWaitingRXState_ListenError);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_FsmListeningState_SendImmTimerStartError);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_FsmListeningState_TimerStartError);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_FsmListeningState_Timeout);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_FsmWaintingTXState_SendError);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_FsmWaintingTXState_SendOutOfDate);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_FsmSendingState_ByPassCmd);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_FsmSendingState_TimerStartError);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_FsmSendingState_TimeOut);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_Fsm_ProcessCanceled);
    RUN_TEST_CASE(WizeCoreMgr_adm, test_AdmMgr_Fsm_ProcessOk);
}

TEST_GROUP_RUNNER(WizeCoreMgr_dwn)
{
	RUN_TEST_CASE(WizeCoreMgr_dwn, test_DwnMgr_Init);
	RUN_TEST_CASE(WizeCoreMgr_dwn, test_DwnMgr_FsmIdleState_NextBlkOffsetError);
	RUN_TEST_CASE(WizeCoreMgr_dwn, test_DwnMgr_FsmIdleState_TimerStartError);
	RUN_TEST_CASE(WizeCoreMgr_dwn, test_DwnMgr_FsmWaitingState_TimerStartError);
	RUN_TEST_CASE(WizeCoreMgr_dwn, test_DwnMgr_FsmWaitingRXState_SetDownlinkError);
	RUN_TEST_CASE(WizeCoreMgr_dwn, test_DwnMgr_FsmWaitingRXState_ListenError);
	RUN_TEST_CASE(WizeCoreMgr_dwn, test_DwnMgr_FsmWaitingRXState_TimerStartError);
	RUN_TEST_CASE(WizeCoreMgr_dwn, test_DwnMgr_FsmWaitingRXState_SessionDone);
	RUN_TEST_CASE(WizeCoreMgr_dwn, test_DwnMgr_FsmListeningState_OutOfDate);
	RUN_TEST_CASE(WizeCoreMgr_dwn, test_DwnMgr_FsmListeningState_Timeout);
	RUN_TEST_CASE(WizeCoreMgr_dwn, test_DwnMgr_Fsm_ProcessCanceled);
	RUN_TEST_CASE(WizeCoreMgr_dwn, test_DwnMgr_Fsm_ProcessOk);
}
