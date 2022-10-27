#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

TEST_GROUP(Samples_TimeEvtInt);

#include "time_evt.h"

/******************************************************************************/
/******************************************************************************/

extern struct time_evt_ctx_s sTimeEvtCtx;
extern void _reset(struct time_evt_ctx_s *pCtx);
extern void _insert(struct time_evt_ctx_s *pCtx, struct time_evt_s *pNew);
extern void _remove(struct time_evt_ctx_s *pCtx, struct time_evt_s *pDel);
extern void _update(struct time_evt_ctx_s *pCtx, uint64_t u64Now);
extern void* _current(struct time_evt_ctx_s *pCtx);

static void _insert_all_(time_evt_t *pTimeEvt, uint64_t *pValue, uint32_t u32NbElement);
static void _check_ordering_(uint32_t u32NbElement, uint64_t u64LowerVal, uint64_t u64GreaterVal);
static uint8_t _search_value_(uint64_t u64value);

/******************************************************************************/
TEST_SETUP(Samples_TimeEvtInt)
{
}

TEST_TEAR_DOWN(Samples_TimeEvtInt)
{
}

TEST(Samples_TimeEvtInt, test_TimeEvtInt_EventCtx_InsertTest)
{
#define NB_EVENT 10
	uint8_t u8NbEvent = NB_EVENT;
	struct time_evt_ctx_s *pCtx = &sTimeEvtCtx;
	time_evt_t aTimeEvt[NB_EVENT];
	uint64_t aValue[NB_EVENT] = {
			100, 102, 98, 55, 4,
			 18,  32, 78,  0, 86400
	};
	TEST_ASSERT_NOT_NULL(pCtx);
	_reset(pCtx);
	/***********************************************/
	// insert all
	_insert_all_(aTimeEvt, aValue, u8NbEvent);
	// check that ordering is respected
	_check_ordering_(NB_EVENT, 0, 86400);

	/***********************************************/
}

TEST(Samples_TimeEvtInt, test_TimeEvtInt_EventCtx_RemoveTest)
{
#define NB_EVENT 10
	uint8_t i;
	uint8_t u8NbEvent = NB_EVENT;
	struct time_evt_ctx_s *pCtx = &sTimeEvtCtx;
	time_evt_t aTimeEvt[NB_EVENT];
	uint64_t aValue[NB_EVENT] = {
			100, 102, 98, 55, 4,
			 18,  32, 78,  0, 86400
	};
	TEST_ASSERT_NOT_NULL(pCtx);
	_reset(pCtx);
	/***********************************************/
	// insert all
	_insert_all_(aTimeEvt, aValue, u8NbEvent);
	// check that ordering is respected
	_check_ordering_(NB_EVENT, 0, 86400);

	/***********************************************/
	// check remove
	time_evt_t *pTimeEvt;
	// remove one
	u8NbEvent--;
	i = 3;
	pTimeEvt = &aTimeEvt[i];
	_remove(pCtx, pTimeEvt);
	// check ordering is still preserved
	_check_ordering_(u8NbEvent, 0, 86400);
	// check that remove value is effective
	TEST_ASSERT_FALSE(_search_value_(aValue[i]));

	// remove first
	u8NbEvent--;
	i = 8;
	pTimeEvt = &aTimeEvt[i];
	_remove(pCtx, pTimeEvt);
	// check ordering is still preserved
	_check_ordering_(u8NbEvent, 4, 86400);
	// check that remove value is effective
	TEST_ASSERT_FALSE(_search_value_(aValue[i]));

	// remove last
	u8NbEvent--;
	i = 9;
	pTimeEvt = &aTimeEvt[i];
	_remove(pCtx, pTimeEvt);
	// check ordering is still preserved
	_check_ordering_(u8NbEvent, 4, 102);
	// check that remove value is effective
	TEST_ASSERT_FALSE(_search_value_(aValue[i]));

	/***********************************************/
}

TEST(Samples_TimeEvtInt, test_TimeEvtInt_EventCtx_UpdateTest)
{
#define NB_EVENT 10
	uint8_t i;
	uint8_t u8NbEvent = NB_EVENT;
	struct time_evt_ctx_s *pCtx = &sTimeEvtCtx;
	time_evt_t aTimeEvt[NB_EVENT];
	uint64_t aValue[NB_EVENT] = {
			100, 102, 98, 55, 4,
			 18,  32, 78,  0, 86400
	};
	TEST_ASSERT_NOT_NULL(pCtx);
	_reset(pCtx);
	/***********************************************/
	// insert all
	_insert_all_(aTimeEvt, aValue, u8NbEvent);
	// check that ordering is respected
	_check_ordering_(NB_EVENT, 0, 86400);

	/***********************************************/
	// check update
	uint64_t u64Now;
	uint64_t u64Expected;

	void _check_expected_(uint64_t u64AdjustVal)
	{
		for (i = 0; i < u8NbEvent; i++)
		{

#ifdef HAS_TIMEEVT_ABS
			u64Expected = (aValue[i] > u64AdjustVal)?(aValue[i]):(0);
#else
			u64Expected = (aValue[i] >= u64AdjustVal)?(aValue[i] - u64AdjustVal):(0);
#endif

			TEST_ASSERT_EQUAL(u64Expected, aTimeEvt[i].u64Value);
		}
	}

	// update with 0
	u64Now = 0;
	_update(pCtx, u64Now);
	_check_expected_(u64Now);

	// update with 4
	u64Now = 4;
	_update(pCtx, u64Now);
	_check_expected_(u64Now);

	// update with 80
	u64Now = 80;
	_update(pCtx, u64Now);
	_check_expected_(u64Now);

	// update with 105
	u64Now = 105;
	_update(pCtx, u64Now);
	_check_expected_(u64Now);
}

/******************************************************************************/
/******************************************************************************/

static void _insert_all_(time_evt_t *pTimeEvt, uint64_t *pValue, uint32_t u32NbElement)
{
	uint32_t i = u32NbElement;
	struct time_evt_ctx_s *pCtx = &sTimeEvtCtx;
	TEST_ASSERT_NOT_NULL(pTimeEvt);
	TEST_ASSERT_NOT_NULL(pValue);

	// init time_evt_t table
	for (i = 0; i < u32NbElement; i++)
	{
		pTimeEvt[i].pNext = NULL;
		pTimeEvt[i].u64Value = pValue[i];
		pTimeEvt[i].u64InitVal = pValue[i];
		pTimeEvt[i].pvTaskHandle = NULL;
	}
	// insert
	for (i = 0; i < u32NbElement; i++)
	{
		_insert(pCtx, &pTimeEvt[i]);
	}
}

static void _check_ordering_(uint32_t u32NbElement, uint64_t u64LowerVal, uint64_t u64GreaterVal)
{
	struct time_evt_ctx_s *pCtx = &sTimeEvtCtx;
	time_evt_t *pTimeEvt;
	uint32_t i = u32NbElement;
	uint8_t bIsFirst = 1;
	uint8_t bIsLast = 0;
	// check that ordering is respected
	pTimeEvt = (time_evt_t*)(_current(pCtx));
	while(pTimeEvt != NULL)
	{
		i--;
		if(pTimeEvt->pNext)
		{
			TEST_ASSERT_TRUE(pTimeEvt->u64Value <= pTimeEvt->pNext->u64Value);
		}
		else
		{
			TEST_ASSERT_EQUAL(0, i);
			bIsLast = 1;
		}

		if(bIsFirst)
		{
			bIsFirst = 0;
			TEST_ASSERT_EQUAL(u64LowerVal, pTimeEvt->u64Value);
		}
		if(bIsLast)
		{
			TEST_ASSERT_EQUAL(u64GreaterVal, pTimeEvt->u64Value);
		}

		pTimeEvt = pTimeEvt->pNext;
	}
}

static uint8_t _search_value_(uint64_t u64value)
{
	struct time_evt_ctx_s *pCtx = &sTimeEvtCtx;
	time_evt_t *pTimeEvt;
	// check that ordering is respected
	pTimeEvt = (time_evt_t*)(_current(pCtx));
	while(pTimeEvt != NULL)
	{
		if(pTimeEvt->pNext)
		{
			if(pTimeEvt->u64Value == u64value)
			{
				return 1;
			}
		}
		pTimeEvt = pTimeEvt->pNext;
	}
	return 0;
}
