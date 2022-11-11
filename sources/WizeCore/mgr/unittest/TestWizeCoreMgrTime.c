#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

TEST_GROUP(WizeCoreMgr_time);
#include "time_mgr.h"

#include <machine/endian.h>

// Mock Samples
#include "mock_time_evt.h"

/******************************************************************************/
#define EPOCH_UNIX_TO_OURS 1356998400U
time_upd_ctx_t sTimeCtx;
struct time_upd_s sTimeUpdCtx;
uint32_t CurEpoch;
uint16_t CurOffset;
uint16_t CurDrift;

/******************************************************************************/

TEST_SETUP(WizeCoreMgr_time)
{
	TimeEvt_UpdateTime_Ignore();

	sTimeCtx.pTimeUpd = &sTimeUpdCtx;
	sTimeCtx.pCurEpoch  = &CurEpoch;
	sTimeCtx.pCurOffset = &CurOffset;
	sTimeCtx.pCurDrift  = &CurDrift;
	sTimeCtx.u32OffsetToUnix = EPOCH_UNIX_TO_OURS;

	sTimeUpdCtx.value = 0;
	sTimeUpdCtx.drift_.drift = 0;
	sTimeUpdCtx.days_cnt = 0;
	sTimeUpdCtx.state_.state = 0;

	CurEpoch = 0;
	CurOffset = 0;
	CurDrift = 0;
}

TEST_TEAR_DOWN(WizeCoreMgr_time)
{

}

/******************************************************************************/

TEST(WizeCoreMgr_time, test_TimeMgr_CurValChanged)
{
	uint32_t eRet = 0;
	struct time_upd_s *pCtx = &sTimeUpdCtx;

	time_t t;
	time( &t );
	t -=  EPOCH_UNIX_TO_OURS;
	t = __htonl(t);

	// Drift changed
	CurDrift = __htons(0x0105);
	eRet = TimeMgr_Main(&sTimeCtx, 0);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(TIME_FLG_DRIFT_CHANGE, eRet, "Return");
	TEST_ASSERT_EQUAL_INT8_MESSAGE(0x01, pCtx->drift_.value, "Value");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x05, pCtx->drift_.period, "Period");
	TEST_ASSERT_EQUAL_UINT8_MESSAGE(0x05, pCtx->days_cnt, "Period");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(t, CurEpoch, "Current Epoch");

	// Offset changed
	CurOffset = __htons(0x0005);
	eRet = TimeMgr_Main(&sTimeCtx, 0);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(TIME_FLG_OFFSET_CHANGE, eRet, "Return");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0x0005, pCtx->value, "Value");
	TEST_ASSERT_TRUE_MESSAGE( (pCtx->state_.epoch_pend == 0) , "Pending bit");
	TEST_ASSERT_TRUE_MESSAGE( (pCtx->state_.offset_pend == 1) , "Pending bit");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(t, CurEpoch, "Current Epoch");

	// Epoch changed
	CurEpoch = __htonl(0x0005);
	eRet = TimeMgr_Main(&sTimeCtx, 0);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(TIME_FLG_CLOCK_CHANGE, eRet, "Return");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0x0005, pCtx->value, "Value");
	TEST_ASSERT_TRUE_MESSAGE( (pCtx->state_.epoch_pend == 1) , "Pending bit");
	TEST_ASSERT_TRUE_MESSAGE( (pCtx->state_.offset_pend == 0) , "Pending bit");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(t, CurEpoch, "Current Epoch");
}

TEST(WizeCoreMgr_time, test_TimeMgr_DayPassNoCorrection)
{
	uint32_t eRet = 0;

	TimeEvt_UpdateTime_Expect(0);

	eRet = TimeMgr_Main(&sTimeCtx, 1);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(TIME_FLG_NONE, eRet, "Return");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, CurEpoch, "Current Epoch");
}

TEST(WizeCoreMgr_time, test_TimeMgr_DayPassClockCorrection)
{
	uint32_t eRet = 0;
	struct time_upd_s *pCtx = &sTimeUpdCtx;

	time_t t = 0x1024;

	pCtx->value = (uint32_t)t;
	pCtx->state_.epoch_pend = 1;

	t += EPOCH_UNIX_TO_OURS;
	TimeEvt_UpdateTime_Expect(t);

	eRet = TimeMgr_Main(&sTimeCtx, 1);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(TIME_FLG_CLOCK_ADJ, eRet, "Return");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, pCtx->value, "Value");
	TEST_ASSERT_TRUE_MESSAGE( (pCtx->state_.epoch_pend == 0) , "Pending bit");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, CurEpoch, "Current Epoch");
}

TEST(WizeCoreMgr_time, test_TimeMgr_DayPassOffsetCorrection)
{
	uint32_t eRet = 0;
	int16_t offset = 10;
	struct time_upd_s *pCtx = &sTimeUpdCtx;

	time_t t;
	time( &t );
	t += offset;

	TimeEvt_UpdateTime_Expect(t);

	pCtx->value = (uint32_t)(offset);
	pCtx->state_.offset_pend = 1;

	eRet = TimeMgr_Main(&sTimeCtx, 1);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(TIME_FLG_OFFSET_ADJ, eRet, "Return");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, pCtx->value, "Value");
	TEST_ASSERT_TRUE_MESSAGE( (pCtx->state_.offset_pend == 0) , "Pending bit");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, CurEpoch, "Current Epoch");
}

TEST(WizeCoreMgr_time, test_TimeMgr_DayPassDriftCorrection)
{
	uint32_t eRet = 0;
	uint8_t days_cnt = 10;
	int8_t drift_val = 1;
	struct time_upd_s *pCtx = &sTimeUpdCtx;

	time_t t;
	uint8_t i;

	pCtx->days_cnt = days_cnt;
	pCtx->drift_.period = days_cnt;
	pCtx->drift_.value = drift_val;

	for (i = days_cnt - 1; i > 0; i--)
	{
		TimeEvt_UpdateTime_Expect(0);
		eRet = TimeMgr_Main(&sTimeCtx, 1);
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(TIME_FLG_NONE, eRet, "Return");
		TEST_ASSERT_EQUAL_UINT32_MESSAGE(i, pCtx->days_cnt, "Count");
	}

	time( &t );
	t += drift_val;
	TimeEvt_UpdateTime_Expect(t);
	eRet = TimeMgr_Main(&sTimeCtx, 1);
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(TIME_FLG_DRIFT_ADJ, eRet, "Return");
	TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, CurEpoch, "Current Epoch");
}
