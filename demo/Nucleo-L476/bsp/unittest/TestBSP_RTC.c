#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

TEST_GROUP(NucleoL476_bspRTC);

#include "bsp.h"

#ifndef EPOCH_UNIX_TO_OURS
#define EPOCH_UNIX_TO_OURS 1356998400U // second between Unix Epoch and 01/01/2013-00:00:00
#endif

TEST_SETUP(NucleoL476_bspRTC)
{
}

TEST_TEAR_DOWN(NucleoL476_bspRTC)
{
}

TEST(NucleoL476_bspRTC, test_BSP_RTC_Time_Test)
{
	time_t t1, t2;
	t1 = EPOCH_UNIX_TO_OURS;

	BSP_Rtc_TimeWrite(t1);
	BSP_Rtc_TimeRead(&t2);
	TEST_ASSERT_EQUAL_MESSAGE(t1, t2, "TimeRead doesn't match TimeWrite");

	dayligth_sav_e dayligth_sav;

	dayligth_sav = SUMMER_TIME_CHANGE;
    /*
     * Note : WARNING, Update Daylight for summer add 1 hour
     * If, before calling Update Daylight function :
     * - current time = 22:00:00, then updated time = 23:00:00 (expected 23:00:00)
     * - current time = 23:00:00, then updated time = 24:00:00 (expected 00:00:00)
     * - current time = 00:00:00, then updated time = 01:00:00 (expected 01:00:00)
     * - current time = 01:00:00, then updated time = 02:00:00 (expected 02:00:00)
     */
    // set current time to 00:00:00
	t1 = EPOCH_UNIX_TO_OURS;
	BSP_Rtc_TimeWrite(t1);
	BSP_Rtc_TimeRead(&t1);
    BSP_Rtc_TimeUpdateDayligth(dayligth_sav);
    msleep(12);
    BSP_Rtc_TimeRead(&t2);
    TEST_ASSERT_EQUAL_MESSAGE((t1+3600), t2, "Summer Time Update at 00:00:00");

    // set current time to 22:00:00
	t1 = EPOCH_UNIX_TO_OURS - 2*3600 ;
	BSP_Rtc_TimeWrite(t1);
	BSP_Rtc_TimeRead(&t1);
    BSP_Rtc_TimeUpdateDayligth(dayligth_sav);
    msleep(12);
    BSP_Rtc_TimeRead(&t2);
    TEST_ASSERT_EQUAL_MESSAGE((t1+3600), t2, "Summer Time Update at 22:00:00");

    // set current time to 01:00:00
	t1 = EPOCH_UNIX_TO_OURS + 3600 ;
	BSP_Rtc_TimeWrite(t1);
	BSP_Rtc_TimeRead(&t1);
    BSP_Rtc_TimeUpdateDayligth(dayligth_sav);
    msleep(12);
    BSP_Rtc_TimeRead(&t2);
    TEST_ASSERT_EQUAL_MESSAGE((t1+3600), t2, "Summer Time Update at 01:00:00");

    // set current time to 23:00:00
	t1 = EPOCH_UNIX_TO_OURS - 3600 ;
	BSP_Rtc_TimeWrite(t1);
	BSP_Rtc_TimeRead(&t1);
    BSP_Rtc_TimeUpdateDayligth(dayligth_sav);
    msleep(12);
    BSP_Rtc_TimeRead(&t2);
    TEST_ASSERT_EQUAL_MESSAGE((t1+3600), t2, "Summer Time Update at 23:00:00");



	dayligth_sav = WINTER_TIME_CHANGE;
    /*
     * Note : WARNING, Update Daylight for winter subtract 1 hour
     * If, before calling Update Daylight function :
     * - current time = 23:00:00, then updated time = 22:00:00 (expected 22:00:00)
     * - current time = 00:00:00, then updated time = 00:00:00 (expected 23:00:00)
     * - current time = 01:00:00, then updated time = 12:00:00 (expected 00:00:00)
     * - current time = 02:00:00, then updated time = 01:00:00 (expected 01:00:00)
     */
    // set current time to 02:00:00
	t1 = EPOCH_UNIX_TO_OURS + 2*3600 ;
	BSP_Rtc_TimeWrite(t1);
	BSP_Rtc_TimeRead(&t1);
    BSP_Rtc_TimeUpdateDayligth(dayligth_sav);
    msleep(12);
    BSP_Rtc_TimeRead(&t2);
    TEST_ASSERT_EQUAL_MESSAGE((t1-3600), t2, "Winter Time Update at 02:00:00");

    // set current time to 23:00:00
	t1 = EPOCH_UNIX_TO_OURS - 3600 ;
	BSP_Rtc_TimeWrite(t1);
	BSP_Rtc_TimeRead(&t1);
    BSP_Rtc_TimeUpdateDayligth(dayligth_sav);
    msleep(12);
    BSP_Rtc_TimeRead(&t2);
    TEST_ASSERT_EQUAL_MESSAGE((t1-3600), t2, "Winter Time Update at 23:00:00");

    /*
    // set current time to 00:00:00
	t1 = EPOCH_UNIX_TO_OURS;
	BSP_Rtc_TimeWrite(t1);
	BSP_Rtc_TimeRead(&t1);
    BSP_Rtc_TimeUpdateDayligth(dayligth_sav);
    msleep(12);
    BSP_Rtc_TimeRead(&t2);
    TEST_ASSERT_EQUAL_MESSAGE((t1-3600), t2, "Winter Time Update at 00:00:00");

    // set current time to 01:00:00
	t1 = EPOCH_UNIX_TO_OURS + 3600 ;
	BSP_Rtc_TimeWrite(t1);
	BSP_Rtc_TimeRead(&t1);
    BSP_Rtc_TimeUpdateDayligth(dayligth_sav);
    msleep(12);
    BSP_Rtc_TimeRead(&t2);
    TEST_ASSERT_EQUAL_MESSAGE((t1-3600), t2, "Winter Time Update at 01:00:00");
    */
}


uint32_t gEvt;

static void event_cb(void)
{
	gEvt = 1;
}

TEST(NucleoL476_bspRTC, test_BSP_RTC_WakeUpTimer_Test)
{
	uint32_t lower, prev;
	int32_t diff;
	struct timeval tp;
	time_t t1;
	uint8_t div_s, div_a, count;
	t1 = EPOCH_UNIX_TO_OURS-1;
	BSP_Rtc_TimeWrite(t1);
	/*
	 * With RTC Clock = LSI (32khz)
	 *  24h =>      86400 seconds => div_a = 127; div_s = 255;
	 *  24h => approx. 96 seconds => div_a =  34; div_s =   0;
	 *  24h => approx. 72 seconds => div_a =  25; div_s =   0;
	 *  24h => approx. 48 seconds => div_a =  16; div_s =   0;
	 *  24h => approx. 24 seconds => div_a =   7; div_s =   0;
	 *  24h => approx. 12 seconds => div_a =   3; div_s =   0;
	 *
	 */
	div_s = 0;
	div_a = 3;
	BSP_Rtc_PrescalerSetup(div_s, div_a);
	BSP_Rtc_WakeUpTimerSetCallback(event_cb);
	prev = (uint32_t)(t1 & 0xFFFFFFFF );
	count = 4;
	BSP_Rtc_WakeUpTimerEnable();
	while (count)
	{
		gEvt = 0;
		while(gEvt == 0);
		BSP_Rtc_TimeReadMicro(&tp);
		lower = (uint32_t)(tp.tv_sec & 0xFFFFFFFF );
		printf("wkup : %lu,%lu (%lu)\n", lower, tp.tv_usec, (lower - prev) );
		// the first occurrence is relative to our time the setting t1
		if (count < 4) {
			diff = lower - prev - 86400;
			TEST_ASSERT_TRUE_MESSAGE( (diff < 5), "Wkup doesn't match");
			//TEST_ASSERT_EQUAL_MESSAGE( (prev + 86400), (lower & 0xFFFFFFFC), "Wkup doesn't match");
		}
		prev = lower;
		count--;
	}
}

TEST(NucleoL476_bspRTC, test_BSP_RTC_UpdateTime_Test)
{
	printf("TODO\n");
	TEST_ASSERT_TRUE_MESSAGE( (0), "UpdateTime test is not implemented");
}



