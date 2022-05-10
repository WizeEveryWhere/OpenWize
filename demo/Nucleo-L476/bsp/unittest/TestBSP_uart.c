#include "unity_fixture.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

TEST_GROUP(NucleoL476_bspUART);

#include "bsp.h"
#include "platform.h"


TEST_SETUP(NucleoL476_bspUART)
{

}

TEST_TEAR_DOWN(NucleoL476_bspUART)
{

}

/*******************************************************************************/
#define BUF_SZ 515

typedef struct
{
	uint16_t u16Len;
	uint8_t pBuf[BUF_SZ];
} fake_device_t;

fake_device_t sFakeDevice;
uint32_t u32RcvFlag;
static void _rcv_cb_(void *p_CbParam,  uint32_t evt)
{
	u32RcvFlag = evt;
}


TEST(NucleoL476_bspUART, test_BSP_Recv_Address_Success)
{

}


TEST(NucleoL476_bspUART, test_BSP_Recv_w_Address_Success)
{
	uint8_t eRet;

	uint16_t c = hex2ascii(0xF5);

	eRet = BSP_Uart_SetCallback(UART_ID_PHY, _rcv_cb_, NULL);
    TEST_ASSERT_EQUAL_MESSAGE(DEV_SUCCESS, eRet, "SetCallback failed");

    eRet = BSP_Uart_Init( UART_ID_PHY, '\r', UART_MODE_EOB, 0);
    TEST_ASSERT_EQUAL_MESSAGE(DEV_SUCCESS, eRet, "Init failed");


	eRet = BSP_Uart_Receive(UART_ID_PHY, sFakeDevice.pBuf, BUF_SZ);
	TEST_ASSERT_EQUAL_MESSAGE(DEV_SUCCESS, eRet, "Receive failed");

	uint16_t i;
	//for (i = 0; i < 3; i++)
	while(1)
	{
		u32RcvFlag = UART_EVT_NONE;
		while (!u32RcvFlag);
		if (u32RcvFlag != UART_EVT_RX_HCPLT)
		{
			break;
		}
	}
	TEST_ASSERT_NOT_EQUAL_MESSAGE(UART_EVT_RX_ABT, u32RcvFlag, "Receive Aborted");
	TEST_ASSERT_EQUAL_MESSAGE(UART_EVT_RX_CPLT, u32RcvFlag, "Receive Complete");

	sFakeDevice.u16Len = ascii2hex( __ntohs( *((uint16_t*)(sFakeDevice.pBuf)) ) );
	printf("Recv : [");
	for (i = 0; i < sFakeDevice.u16Len; i++)
	{
		printf("%c, ", sFakeDevice.pBuf[i]);
		if ( ( (i+1)%30) == 0 )
		{
			printf("\n");
		}
	}
	printf("]\n");
}

TEST(NucleoL476_bspUART, test_BSP_Send_w_Address_Success)
{
	uint8_t eRet;
	eRet = BSP_Uart_SetCallback(UART_ID_PHY, _rcv_cb_, NULL);
    TEST_ASSERT_EQUAL_MESSAGE(DEV_SUCCESS, eRet, "SetCallback failed");

    eRet = BSP_Uart_Init( UART_ID_PHY, '\r', UART_MODE_EOB, 0);
    TEST_ASSERT_EQUAL_MESSAGE(DEV_SUCCESS, eRet, "Init failed");

    uint16_t i;
    i = 2;
    sFakeDevice.u16Len = 6;
    memcpy(&(sFakeDevice.pBuf[i]), "HelloF", sFakeDevice.u16Len);
    i += sFakeDevice.u16Len;
    *(uint16_t*)(&sFakeDevice.pBuf[i]) = __htons( hex2ascii(0xFF) );
    i += 2;
    sFakeDevice.pBuf[i++] = '\r';
    sFakeDevice.u16Len = i;


    *(uint16_t*)(&sFakeDevice.pBuf[0]) = __htons( hex2ascii( (uint8_t)(i-2)) );

    eRet = BSP_Uart_Transmit(UART_ID_PHY, sFakeDevice.pBuf, sFakeDevice.u16Len);
    TEST_ASSERT_EQUAL_MESSAGE(DEV_SUCCESS, eRet, "Transmit failed");

	for (i = 0; i < 3; i++)
	{
		u32RcvFlag = UART_EVT_NONE;
		while (!u32RcvFlag);
		if (u32RcvFlag != UART_EVT_RX_HCPLT)
		{
			break;
		}
	}
	TEST_ASSERT_EQUAL_MESSAGE(UART_EVT_TX_CPLT, u32RcvFlag, "Transmit Complete");
}
