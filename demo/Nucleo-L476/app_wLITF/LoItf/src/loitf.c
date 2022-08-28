
/**
  * @file: loitf.c
  * @brief: // TODO This file ...
  * 
  * @details
  *
  * @copyright 2019, GRDF, Inc.  All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted (subject to the limitations in the disclaimer
  * below) provided that the following conditions are met:
  *    - Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *    - Redistributions in binary form must reproduce the above copyright 
  *      notice, this list of conditions and the following disclaimer in the 
  *      documentation and/or other materials provided with the distribution.
  *    - Neither the name of GRDF, Inc. nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  *
  * @par Revision history
  *
  * @par 1.0.0 : 2022/08/16 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup app_wLITF
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "loitf.h"

#include "rtos_macro.h"
#include "logger.h"
#include <bsp.h>

#include <time.h>
#include "time_evt.h"

#include <platform.h>

#include "wize_api.h"
#include "proto.h"
#include "parameters.h"
#include "parameters_lan_ids.h"

#include "crypto.h"

#include "perf_utils.h"

static void LoItf_MainTask(void const * argument);
static void _loitf_evt_(void *p_CbParam,  uint32_t evt);

#define LOITF_TASK_NAME loitf
#define LOITF_TASK_FCT LoItf_MainTask
#define LOITF_STACK_SIZE 300
#define LOITF_PRIORITY (UBaseType_t)(tskIDLE_PRIORITY+1)

SYS_TASK_CREATE_DEF(loitf, LOITF_STACK_SIZE, LOITF_PRIORITY);

#ifndef LOITF_TMO_EVT
	//#define LOITF_TMO_EVT 0xFFFFFFFF
	#define LOITF_TMO_EVT 5000
#endif

#ifndef LOITF_TRANS_SUFFIX
	#define LOITF_TRANS_SUFFIX "\r\n"
#endif

typedef enum
{
	LOITF_EVT_SENT = 0x01,
	LOITF_EVT_RECV = 0x02,
	LOITF_EVT_TMO  = 0x03,
	LOITF_EVT_WKUP  = 0x04,
} loitf_evt_e;

typedef enum
{
	LOITF_STATE_IDLE  = 0x00,
	LOITF_STATE_BUSY  = 0x01,
	LOITF_STATE_SLEEP = 0x10,
	LOITF_STATE_WKUP  = 0x20,
} loitf_state_e;

typedef enum
{
	LOITF_ERR_NONE              = 0x00,
	LOITF_ERR_INVALID_CMD       = 0x01,
	LOITF_ERR_INVALID_PARAM_ID  = 0x02,
	LOITF_ERR_CONFORM_PARAM_VAL = 0x03,
	LOITF_ERR_ACCESS            = 0x04,
	LOITF_INF_SLEEP,
	LOITF_INF_BUSY,
	LOITF_INF_WAKEUP,
} loitf_error_e;

const char * const loitf_error_msgs[] =
{
    [LOITF_ERR_NONE]              = "ERR_NONE",
	[LOITF_ERR_INVALID_CMD]       = "ERR_CMD",
	[LOITF_ERR_INVALID_PARAM_ID]  = "ERR_ID",
	[LOITF_ERR_CONFORM_PARAM_VAL] = "ERR_VAL",
	[LOITF_ERR_ACCESS]            = "ERR_ACCESS",
	[LOITF_INF_SLEEP]             = "SLEEP",
	[LOITF_INF_BUSY]              = "BUSY",
	[LOITF_INF_WAKEUP]            = "WAKEUP",
};

#define BUF_INPUT_SZ 511
#define BUF_OUTPUT_SZ 511

#define BUF_WORK_SZ 255

struct loitf_s
{
	void *hTask;
	uint8_t pBufIn[BUF_INPUT_SZ];
	uint8_t pBufOut[BUF_OUTPUT_SZ];

	uint8_t pBufWorkIn[BUF_WORK_SZ];

	int16_t i16InSz;
	int16_t i16OutSz;

	loitf_state_e eState;

	loitf_error_e eError;

	uint8_t mode;
};

static struct loitf_s sLoItf;

/*!
  * @brief Function convert an ascii string into hexa string.
  *
  * @param [in] pDest  Converted string
  * @param [in] pSrc   String to convert
  * @param [in] src_sz The number of hexa value in pSrc
  *
  * @return The number of bytes in pDest (-1 in fault case)
  */
static int32_t str_ascii2hex(uint8_t *pDest, uint16_t *pSrc, size_t src_sz)
{
	size_t i;
	if ( pSrc && pDest && src_sz)
	{
		for (i = 0; i < src_sz; i++)
		{
			pDest[i] = ascii2hex( __ntohs((pSrc[i])) );
		}
		return (int32_t)i;
	}
	else
	{
		return -1;
	}
}

/*!
  * @brief Function convert an hexa string into ascii string.
  *
  * @param [in] pDest  Converted string
  * @param [in] pSrc   String to convert
  * @param [in] src_sz The number of hexa value in pSrc
  *
  * @return The number of bytes in pDest (-1 in fault case)
  */
static int32_t str_hex2ascii(uint16_t *pDest, uint8_t *pSrc, size_t src_sz)
{
	size_t i;
	if ( pSrc && pDest && src_sz)
	{
		for (i = 0; i < src_sz; i++)
		{
			pDest[i] = __htons(hex2ascii(pSrc[i]));
		}
		return (int32_t)i;
	}
	else
	{
		return -1;
	}
}


void LoItf_Setup(void)
{
	sLoItf.hTask = SYS_TASK_CREATE_CALL(loitf, LOITF_TASK_FCT, NULL);
	assert(sLoItf.hTask);
	BSP_Uart_SetCallback(UART_ID_COM, _loitf_evt_, NULL);
	//BSP_Uart_Init(UART_ID_COM, '\r', UART_MODE_EOB, 576000); // 5000ms
	BSP_Uart_Init(UART_ID_COM, '\r', UART_MODE_EOB, 0); // 5000ms
}

extern boot_state_t gBootState;


static void LoItf_OnReceived(void)
{
	uint8_t *pData = sLoItf.pBufIn;
	while (*pData == '\n' && (pData < (sLoItf.pBufIn + BUF_INPUT_SZ)) )
	{
		pData++;
	}

	uint8_t cmd = pData[0];

	// Action Param  pData[1-n]
	if (cmd == 'B')
	{
		LOG_DBG("ITF Reboot\n");
		if (pData[1] == '0')
		{
			BSP_Boot_Reboot(0);
		}
		else if (pData[1] == '1')
		{
			BSP_Boot_Reboot(1);
		}
	}
	else if (cmd == 'M')
	{
		uint32_t *pSRAM2 = (uint32_t*)0x10000000;
		*pSRAM2 = 0x1;
		register int16_t temp = (int16_t)(pData[1]) - 48 ;
		if ( ( temp >= 0) && (temp < LP_NB_MODE) )
		{
			sLoItf.mode = (lp_mode_e)temp;
			sLoItf.i16OutSz = sprintf((char*)sLoItf.pBufOut, "M=%d\r\n", temp);
		}
		else
		{
			sLoItf.eError = LOITF_ERR_CONFORM_PARAM_VAL;
		}
	}
	else if (cmd == 'I')
	{
		LOG_DBG("ITF Install\n");
		WizeApi_ExecPing();
		//if ( WIZE_API_SUCCESS == WizeApi_ExecPing())
		{
			uint8_t temp;
			Param_Access(PING_NBFOUND, &temp, 0);
			sLoItf.i16OutSz = sprintf((char*)sLoItf.pBufOut, "I=%d\r\n", temp);
		}
	}
	else if (cmd == 'D')
	{
		LOG_DBG("ITF Data\n");
		uint8_t u8Size = strlen( (char*)&(pData[1]) ) -1 ;
		if ( u8Size%2 )
		{
			pData[u8Size+1] = '0';
			u8Size++;
		}
		u8Size = u8Size >> 1;
		if ( str_ascii2hex(sLoItf.pBufWorkIn, (uint16_t*)&(pData[1]), u8Size) > 1)
		{
			WizeApi_Send(sLoItf.pBufWorkIn, u8Size, APP_DATA);
			{
				sLoItf.i16OutSz = sprintf((char*)sLoItf.pBufOut, "D[%02hx]=%d bytes\r\n", sLoItf.pBufWorkIn[0], u8Size-1);
			}
		}
		else
		{
			sLoItf.eError = LOITF_ERR_CONFORM_PARAM_VAL;
		}
	}
	else if (cmd == 'K')
	{
		uint8_t u8Size = strlen( (char*)&(pData[1]) ) -1 ;
		u8Size = u8Size >> 1;
		if (u8Size < 17 || u8Size > 33)
		{
			sLoItf.eError = LOITF_ERR_CONFORM_PARAM_VAL;
		}
		else
		{
			str_ascii2hex(sLoItf.pBufWorkIn, (uint16_t*)&(pData[1]), u8Size);
			if ( Crypto_WriteKey(&(sLoItf.pBufWorkIn[1]), sLoItf.pBufWorkIn[0]) != CRYPTO_OK)
			{
				sLoItf.eError = LOITF_ERR_INVALID_PARAM_ID;
			}
			else
			{
				sLoItf.i16OutSz = sprintf((char*)sLoItf.pBufOut, "K%02hx\r\n", sLoItf.pBufWorkIn[0]);
			}
		}
	}
	else if (cmd == 'W')
	{
		uint8_t u8Size = strlen( (char*)&(pData[1]) ) -1 ;
		u8Size = u8Size >> 1;
		str_ascii2hex(sLoItf.pBufWorkIn, (uint16_t*)&(pData[1]), u8Size);

		if ( Param_IsValidId(sLoItf.pBufWorkIn[0]) == 0)
		{
			sLoItf.eError = LOITF_ERR_INVALID_PARAM_ID;
		}
		else
		{
			if ( ( Param_GetSize(sLoItf.pBufWorkIn[0]) < u8Size ) &&
				 ( Param_CheckConformity(sLoItf.pBufWorkIn[0], &(sLoItf.pBufWorkIn[1]) ) != 0)
				)
			{
				if (Param_LocalAccess(sLoItf.pBufWorkIn[0], &(sLoItf.pBufWorkIn[1]), 1) == 0)
				{
					sLoItf.eError = LOITF_ERR_ACCESS;
				}
				else
				{
					sLoItf.i16OutSz = sprintf((char*)sLoItf.pBufOut, "W%02hx\r\n", sLoItf.pBufWorkIn[0]);
				}
			}
			else
			{
				sLoItf.eError = LOITF_ERR_CONFORM_PARAM_VAL;
			}
		}
	}
	else if (cmd == 'R')
	{
		uint8_t u8Size = strlen( (char*)&(pData[1]) ) -1 ;
		u8Size = u8Size >> 1;
		str_ascii2hex(sLoItf.pBufWorkIn, (uint16_t*)&(pData[1]), u8Size);

		if ( Param_IsValidId(sLoItf.pBufWorkIn[0]) == 0)
		{
			sLoItf.eError = LOITF_ERR_INVALID_PARAM_ID;
		}
		else
		{
			if (Param_LocalAccess(sLoItf.pBufWorkIn[0], &(sLoItf.pBufWorkIn[1]), 0) )
			{
				sLoItf.i16OutSz = sprintf((char*)sLoItf.pBufOut, "R%02hx=", sLoItf.pBufWorkIn[0]);
				u8Size = str_hex2ascii( &(sLoItf.pBufOut[sLoItf.i16OutSz]), &(sLoItf.pBufWorkIn[1]), Param_GetSize(sLoItf.pBufWorkIn[0]) );
				if (u8Size < 0)
				{
					sLoItf.eError = LOITF_ERR_CONFORM_PARAM_VAL;
				}
				else
				{
					sLoItf.i16OutSz += u8Size;
					sLoItf.pBufOut[sLoItf.i16OutSz++] = '\r';
					sLoItf.pBufOut[sLoItf.i16OutSz++] = '\n';
				}
			}
			else
			{
				sLoItf.eError = LOITF_ERR_ACCESS;
			}
		}
	}
	else
	{
		sLoItf.eError = LOITF_ERR_INVALID_CMD;
		memset(sLoItf.pBufIn, 0, 20);
	}

	if (sLoItf.eError)
	{
		sLoItf.i16OutSz = sprintf((char*)sLoItf.pBufOut, "%s\r\n", loitf_error_msgs[sLoItf.eError]);
		if(sLoItf.i16OutSz <= 0)
		{
			sLoItf.i16OutSz = 0;
		}
		sLoItf.eError = LOITF_ERR_NONE;
	}
	pData[0] = 0;
}

static void LoItf_MainTask(void const * argument)
{
	uint32_t ulEvent;
	uint32_t u32LoItfTmo = pdMS_TO_TICKS(LOITF_TMO_EVT);



	uint32_t u32CycCnt;
	uint8_t bTxPending = 0;
	uint8_t bRxPending = 0;
	uint8_t bTmoPending = 0;


	sLoItf.mode = LP_STDBY_MODE;
	sLoItf.eError = LOITF_ERR_NONE;
	sLoItf.eState = LOITF_STATE_IDLE;
	sLoItf.i16InSz = 0;
	sLoItf.i16OutSz = 0;

	LOG_DBG("ITF Start\n");

	__cycle_counter_enable__(1);

	sLoItf.i16OutSz = sprintf((char*)sLoItf.pBufOut, "B=0x%lx\r\n", gBootState.state);
	if(sLoItf.i16OutSz > 0)
	{
		BSP_Uart_Transmit(UART_ID_COM, sLoItf.pBufOut, sLoItf.i16OutSz);
	}

	while(1)
	{
		if ( sLoItf.eState == LOITF_STATE_SLEEP )
		{
			LOG_DBG("ITF LP enter mode %d\n", sLoItf.mode);

			BSP_Uart_Transmit(UART_ID_COM, "SLEEP"LOITF_TRANS_SUFFIX, (uint16_t)7);
			xTaskNotifyWait(0, ULONG_MAX, &ulEvent, u32LoItfTmo );
			BSP_Uart_Disable(UART_ID_COM);
			BSP_LowPower_Enter(sLoItf.mode);
			sLoItf.eState = LOITF_STATE_WKUP;

			LOG_DBG("ITF LP exit mode %d\n", sLoItf.mode);
		}

		if (sLoItf.eState == LOITF_STATE_WKUP)
		{
			BSP_Uart_Enable(UART_ID_COM);
			BSP_Uart_Transmit(UART_ID_COM, "WAKEUP"LOITF_TRANS_SUFFIX, (uint16_t)8);
			xTaskNotifyWait(0, ULONG_MAX, &ulEvent, u32LoItfTmo );
			bTmoPending = 0;
			bTxPending = 0;
			bRxPending = 0;
			sLoItf.eState = LOITF_STATE_IDLE;
		}

		if(bRxPending)
		{
			if (sLoItf.eState == LOITF_STATE_IDLE)
			{
				sLoItf.eState = LOITF_STATE_BUSY;
				__cycle_counter_reset__();
				LoItf_OnReceived();
				u32CycCnt = __cycle_counter_read__();
				LOG_INF("ITF cy %u\n", u32CycCnt);
				bRxPending = 0;
				sLoItf.eState = LOITF_STATE_IDLE;
			}
#ifdef LOITF_HAS_BUSY_MSG
			else
			{
				if (!bTxPending)
				{
					bTxPending = 1;

					BSP_Uart_Transmit(UART_ID_COM, "BUSY"LOITF_TRANS_SUFFIX, (uint16_t)6);
				}
			}
#endif
		}

		if (sLoItf.eState == LOITF_STATE_IDLE)
		{
			if (!bTmoPending)
			{
				BSP_Uart_Receive(UART_ID_COM, sLoItf.pBufIn, BUF_INPUT_SZ);
			}
			else
			{
				sLoItf.eState = LOITF_STATE_SLEEP;
			}
		}

		if( (!bTxPending) && (sLoItf.i16OutSz > 0) )
		{
			BSP_Uart_Transmit(UART_ID_COM, sLoItf.pBufOut, (uint16_t)sLoItf.i16OutSz);
		}


		if (xTaskNotifyWait(0, ULONG_MAX, &ulEvent, u32LoItfTmo ))
		{
			bTmoPending = 0;
			switch (ulEvent)
			{
				case UART_EVT_TX_CPLT:
					if (bTxPending)
					{
						bTxPending = 0;
					}
					else
					{
						sLoItf.i16OutSz = 0;
					}
					break;
				case UART_EVT_RX_CPLT:
					bRxPending = 1;
					break;
				case UART_EVT_RX_ABT:
					bTmoPending = 1;
					break;
				default:
					break;
			}
		}
		else // event TMO
		{
			bTmoPending = 1;
		}
	}
}

static void _loitf_evt_(void *p_CbParam,  uint32_t evt)
{
	BaseType_t xHigherPriorityTaskWoken;
	xTaskNotifyFromISR(sLoItf.hTask, evt, eSetBits, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
/******************************************************************************/

#ifdef __cplusplus
}
#endif

/*! @} */

