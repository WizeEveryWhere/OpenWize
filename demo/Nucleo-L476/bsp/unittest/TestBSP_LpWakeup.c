#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

TEST_GROUP(NucleoL476_bspLP);

#include "bsp.h"

TEST_SETUP(NucleoL476_bspLP)
{

}

TEST_TEAR_DOWN(NucleoL476_bspLP)
{

}
/*
void setUp(void)
{
}

void tearDown(void)
{
}
*/
/*******************************************************************************/
TEST(NucleoL476_bspLP, test_BSP_Sleep_Mode_Success)
{
	time_t t1, t2;
	time(&t1);
	BSP_LowPower_Enter(LP_SLEEP_MODE);
	time(&t2);
	printf("Wake-Up from %s mode after %d seconds \n", "Sleep", t2 - t1);
}

TEST(NucleoL476_bspLP, test_BSP_Stop_2_Mode_Success)
{

	time_t t1, t2;
	time(&t1);
	BSP_LowPower_Enter(LP_STOP2_MODE);
	time(&t2);
	printf("Wake-Up from %s mode after %d seconds \n", "Stop2", t2 - t1);
}

TEST(NucleoL476_bspLP, test_BSP_Standby_Mode_Success)
{
	printf("Wake-Up from %s mode unavailable \n", "Standby");
}

TEST(NucleoL476_bspLP, test_BSP_Shutdown_Mode_Success)
{
	printf("Wake-Up from %s mode unavailable \n", "Shutdown");
}

