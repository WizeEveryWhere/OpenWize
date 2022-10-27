#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

TEST_GROUP(Samples_TimeEvt);

#include "time_evt.h"

#include "mock_queue.h"
#include "mock_task.h"
#include "mock_portmacro.h"

/*

1/ generate FreeRTOS_config.h
set( configSUPPORT_DYNAMIC_ALLOCATION         1)
set( configUSE_FreeRTOS_HEAP                  4)
set( configTOTAL_HEAP_SIZE                    5*1024)
set( configUSE_MALLOC_FAILED_HOOK             0)
#set( configENABLE_BACKWARD_COMPATIBILITY      0)
set( configUSE_MALLOC_FAILED_HOOK              1)
set( configUSE_PORT_OPTIMISED_TASK_SELECTION   0)


########

set(configASSERT
"extern void vAssertCalled( const char * const pcFileName, unsigned long ulLine );\n")
set(configASSERT
"${configASSERT}#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ )\n")

set(configPRINTF
"extern void vLoggingPrintf( const char * pcFormatString, ... );\n")
set(configPRINTF
"${configPRINTF}#define configPRINTF( x ) vLoggingPrintf(x)\n")


/*
GDB :
Program with Native FreeRTOS must be compiled and linked without "-static" gcc option.
Otherwise you'll not be able to debug the program.

FreeRTOS mock requirement :
	sys_mutex_create_xxx
	- -> xSemaphoreCreateMutexStatic -> xQueueCreateMutexStatic
	- -> vQueueAddToRegistry

	sys_mutex_acquire
	- -> xSemaphoreTake -> xQueueSemaphoreTake

	sys_mutex_acquire_isr
	- -> xSemaphoreTakeFromISR -> xQueueReceiveFromISR

	sys_mutex_release
	- -> xSemaphoreGive -> xQueueGenericSend

	sys_mutex_release_isr
	- -> xSemaphoreGiveFromISR -> xQueueGiveFromISR

	sys_flag_set_isr
	- -> xTaskNotifyFromISR -> xTaskGenericNotifyFromISR
	- -> portYIELD_FROM_ISR -> vPortYield

*/

/******************************************************************************/


/******************************************************************************/
typedef enum {
	EVT_A    = 0x01,
	EVT_B    = 0x02,
	EVT_C    = 0x04,
	EVT_D    = 0x08,
	EVT_STOP = 0x10,
	EVT_NONE = (uint32_t)(-1),
} evt_e;

#define NB_PROG_EVT 5

void *gu32TaskHandler[NB_PROG_EVT] = {1, 2, 3, 4, 5};
uint32_t gu32NotifyVal[NB_PROG_EVT];

uint64_t gu64SimuTime;
pfTimeEvt_HandlerCB_t gpfTimeEvtHandler;

void simu_time_init(void);
void simu_time_inc(uint32_t u32IncMsTime);
void simu_it(uint32_t u32IncMsTime);

void simu_time_init(void)
{
	int i;
	for (i = 0; i < NB_PROG_EVT; i++)
	{
		gu32NotifyVal[i] = EVT_NONE;
	}
	gu64SimuTime = 0;
}

void simu_time_inc(uint32_t u32IncMsTime)
{
	gu64SimuTime += u32IncMsTime;
}

void simu_it(uint32_t u32IncMsTime)
{
	gu64SimuTime += u32IncMsTime;
	if(gpfTimeEvtHandler)
	{
		gpfTimeEvtHandler();
	}
}

/******************************************************************************/
void _timer_set_handler(const uint8_t u8TimerId, pfTimeEvt_HandlerCB_t const pfCb);
void _timer_start(const uint8_t u8TimerId, uint64_t u64Value);
void _timer_stop(const uint8_t u8TimerId);
void _get_current_time_ms(uint64_t *u64MsTime);
void _set_current_time(time_t t);

inline
void _timer_set_handler(const uint8_t u8TimerId, pfTimeEvt_HandlerCB_t const pfCb)
{
	gpfTimeEvtHandler = pfCb;
}

inline void _timer_start(const uint8_t u8TimerId, uint64_t u64Value) { }

inline void _timer_stop(const uint8_t u8TimerId) { }

inline
void _get_current_time_ms(uint64_t *u64MsTime)
{
    *u64MsTime = gu64SimuTime;
}

inline
void _set_current_time(time_t t)
{
	gu64SimuTime = t*1000;
}

/******************************************************************************/

BaseType_t __xTaskGenericNotifyFromISR_Stub__(
		TaskHandle_t xTaskToNotify,
		UBaseType_t uxIndexToNotify,
		uint32_t ulValue,
		eNotifyAction eAction,
		uint32_t* pulPreviousNotificationValue,
		BaseType_t* pxHigherPriorityTaskWoken, int cmock_num_calls)
{

	int i;
	for (i = 0; i < NB_PROG_EVT; i++)
	{
		if(gu32TaskHandler[i] == xTaskToNotify)
		{
			gu32NotifyVal[i] = ulValue;
			break;
		}
	}
	return pdPASS;
}

/******************************************************************************/
/******************************************************************************/

TEST_SETUP(Samples_TimeEvt)
{
	gu64SimuTime = 0;
	gpfTimeEvtHandler = NULL;

	vPortYield_Ignore();

	vQueueAddToRegistry_Ignore();
	xQueueCreateMutexStatic_IgnoreAndReturn(0x1000);

	xQueueSemaphoreTake_IgnoreAndReturn(1);
	xQueueGenericSend_IgnoreAndReturn(1);
	xQueueReceiveFromISR_IgnoreAndReturn(1);
	xQueueGiveFromISR_IgnoreAndReturn(1);

	//xTaskGenericNotifyFromISR_IgnoreAndReturn(1);
	xTaskGenericNotifyFromISR_Stub(__xTaskGenericNotifyFromISR_Stub__);

	simu_time_init();
	TimeEvt_Setup();
}

TEST_TEAR_DOWN(Samples_TimeEvt)
{

}


TEST(Samples_TimeEvt, test_TimeEvt_TimerInit_failed)
{
	uint8_t ret;
	time_evt_t sTimeEvt;

	ret = TimeEvt_TimerInit(NULL, 0x00010001, TIMEEVT_CFG_ONESHOT);
	TEST_ASSERT_EQUAL(1, ret);

	ret = TimeEvt_TimerInit(&(sTimeEvt), NULL, TIMEEVT_CFG_ONESHOT);
	TEST_ASSERT_EQUAL(1, ret);
}

TEST(Samples_TimeEvt, test_TimeEvt_TimerStart_failed)
{
	uint8_t ret;
	time_evt_t sTimeEvt;

	// ------
	ret = TimeEvt_TimerInit(&(sTimeEvt), 0x00010001, TIMEEVT_CFG_ONESHOT);
	TEST_ASSERT_EQUAL(0, ret);

	// NULL pointer
	ret = TimeEvt_TimerStart(NULL, 10, 0, EVT_A);
	TEST_ASSERT_EQUAL(1, ret);

	// relative timer is negative
	ret = TimeEvt_TimerStart(&(sTimeEvt), 0, -2, EVT_A);
	TEST_ASSERT_EQUAL(1, ret);

	// relative timer is 0
	ret = TimeEvt_TimerStart(&(sTimeEvt), 0, 0, EVT_A);
	TEST_ASSERT_EQUAL(1, ret);

	// timer seems to be already start
	sTimeEvt.pNext = 0x1;
	ret = TimeEvt_TimerStart(&(sTimeEvt), 0, 2, EVT_A);
	TEST_ASSERT_EQUAL(1, ret);

	// ------
	ret = TimeEvt_TimerInit(&(sTimeEvt), 0x00010001, TIMEEVT_CFG_ABSOLUTE);
	TEST_ASSERT_EQUAL(0, ret);

	// absolute timer in the past
	simu_time_inc(2000);
	ret = TimeEvt_TimerStart(&(sTimeEvt), 1, 0, EVT_A);
	TEST_ASSERT_EQUAL(1, ret);
}

TEST(Samples_TimeEvt, test_TimeEvt_CheckIncDecTime)
{
	uint8_t ret;
#define NB_TIMER 3
	time_evt_t sTimeEvt[NB_TIMER];
	evt_e    aEvent[NB_TIMER]  = {EVT_A, EVT_B, EVT_C};
	uint32_t aElapse[NB_TIMER] = {   10,    12,     20};
	time_evt_cfg_e aTimerType[NB_TIMER] = {TIMEEVT_CFG_ONESHOT, TIMEEVT_CFG_ONESHOT, TIMEEVT_CFG_ABSOLUTE};

	uint8_t i;
	for(i = 0; i < NB_TIMER; i++ )
	{
		gu32TaskHandler[i] = 0x00010001 + 2*i;
		ret = TimeEvt_TimerInit(&(sTimeEvt[i]), gu32TaskHandler[i], aTimerType[i]);
		TEST_ASSERT_EQUAL(0, ret);

		ret = TimeEvt_TimerStart(&(sTimeEvt[i]), aElapse[i], 0, aEvent[i]);
		TEST_ASSERT_EQUAL(0, ret);
	}

	uint8_t inc, sum;
	sum = 0;
	inc = 0;
	do
	{
		sum += inc;
		simu_it(inc);
		for(i = 0; i < NB_TIMER; i++ )
		{
			TEST_ASSERT_TRUE(sTimeEvt[i].u64Value == (sTimeEvt[i].u64InitVal - sum));
		}
		inc++;
	} while(inc < 5);
}

TEST(Samples_TimeEvt, test_TimeEvt_CheckPeriodicReArm)
{
	uint8_t ret;
	uint8_t i;
	uint32_t u32TmrSec = 10;
	uint32_t u32SimTimeMs;
	time_evt_t sTimeEvt;

	gu32TaskHandler[0] = 0x00010001;

	ret = TimeEvt_TimerInit(&sTimeEvt, gu32TaskHandler[0], TIMEEVT_CFG_PERIODIC);
	TEST_ASSERT_EQUAL(0, ret);

	ret = TimeEvt_TimerStart(&sTimeEvt, u32TmrSec, 0, EVT_A);
	TEST_ASSERT_EQUAL(0, ret);

	u32SimTimeMs = u32TmrSec*1000;
	for(i = 0; i < 5; i++ )
	{
		simu_it(u32SimTimeMs);
		TEST_ASSERT_TRUE(gu32NotifyVal[0] == EVT_A);
		TEST_ASSERT_TRUE(sTimeEvt.u64Value == sTimeEvt.u64InitVal);
		gu32NotifyVal[0] = EVT_NONE;
	}
}

TEST(Samples_TimeEvt, test_TimeEvt_CheckUpdateTime)
{
	uint8_t ret;
#define NB_TIMER 3
	time_evt_t sTimeEvt[NB_TIMER];
	evt_e    aEvent[NB_TIMER]  = {EVT_A, EVT_B, EVT_C};
	uint32_t aElapse[NB_TIMER] = {   10,    12,     20};
	time_evt_cfg_e aTimerType[NB_TIMER] = {TIMEEVT_CFG_ONESHOT, TIMEEVT_CFG_ONESHOT, TIMEEVT_CFG_ABSOLUTE};

	uint8_t i;
	for(i = 0; i < NB_TIMER; i++ )
	{
		gu32TaskHandler[i] = 0x00010001 + 2*i;
		ret = TimeEvt_TimerInit(&(sTimeEvt[i]), gu32TaskHandler[i], aTimerType[i]);
		TEST_ASSERT_EQUAL(0, ret);

		ret = TimeEvt_TimerStart(&(sTimeEvt[i]), aElapse[i], 0, aEvent[i]);
		TEST_ASSERT_EQUAL(0, ret);
	}

	time_t t = 15;
	TimeEvt_UpdateTime(t);

	TEST_ASSERT_TRUE(gu64SimuTime == (t*1000));
	TEST_ASSERT_TRUE(sTimeEvt[2].u64Value == (sTimeEvt[2].u64InitVal - (t*1000)));
}

TEST(Samples_TimeEvt, test_TimeEvt_CheckLongTreatTime)
{
	uint8_t ret;
#define NB_TIMER 3
	time_evt_t sTimeEvt[NB_TIMER];
	evt_e    aEvent[NB_TIMER]  = {EVT_A, EVT_B, EVT_C};
	uint32_t aElapse[NB_TIMER] = {   10,    12,     20};
	time_evt_cfg_e aTimerType[NB_TIMER] = {TIMEEVT_CFG_ONESHOT, TIMEEVT_CFG_ONESHOT, TIMEEVT_CFG_ABSOLUTE};

	uint8_t i;
	for(i = 0; i < NB_TIMER; i++ )
	{
		gu32TaskHandler[i] = 0x00010001 + 2*i;
		ret = TimeEvt_TimerInit(&(sTimeEvt[i]), gu32TaskHandler[i], aTimerType[i]);
		TEST_ASSERT_EQUAL(0, ret);

		ret = TimeEvt_TimerStart(&(sTimeEvt[i]), aElapse[i], 0, aEvent[i]);
		TEST_ASSERT_EQUAL(0, ret);
	}

	simu_it(25*1000);
	for(i = 0; i < NB_TIMER; i++ )
	{
		TEST_ASSERT_TRUE(gu32NotifyVal[i] == aEvent[i]);
		TEST_ASSERT_TRUE(sTimeEvt[i].u64Value == (0));
		TEST_ASSERT_TRUE(sTimeEvt[i].pNext == NULL);
	}

}

TEST(Samples_TimeEvt, test_TimeEvt_OneTimer_OneShot)
{
	uint8_t ret;
	int16_t i16TmrDeltaMs = 0;
	uint32_t u32TmrSec = 10;
	uint32_t u32SimTimeMs;
	time_evt_t sTimeEvt;

	gu32TaskHandler[0] = 0x00010001;

	ret = TimeEvt_TimerInit(&sTimeEvt, gu32TaskHandler[0], TIMEEVT_CFG_ONESHOT);
	TEST_ASSERT_TRUE(ret == 0);

	ret = TimeEvt_TimerStart(&sTimeEvt, u32TmrSec, i16TmrDeltaMs, EVT_A);
	TEST_ASSERT_TRUE(ret == 0);

	u32SimTimeMs = u32TmrSec*1000;
	u32SimTimeMs += (int32_t)i16TmrDeltaMs;
	simu_it(u32SimTimeMs);
	TEST_ASSERT_TRUE(gu32NotifyVal[0] == EVT_A);
}

/******************************************************************************/
