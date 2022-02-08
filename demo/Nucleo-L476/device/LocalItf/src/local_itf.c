
/**
  * @file: local_itf.c
  * @brief: // TODO This file ...
  * 
  *****************************************************************************
  * @Copyright 2019, GRDF, Inc.  All rights reserved.
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
  *****************************************************************************
  *
  * Revision history
  * ----------------
  * 1.0.0 : 2021/11/04[GBI]
  * Initial version
  *
  *
  */
#ifdef __cplusplus
extern "C" {
#endif

#include "local_itf.h"

#include "rtos_macro.h"
#include "logger.h"
#include <bsp.h>

#include <time.h>
#include "time_evt.h"

#include <platform.h>

#include "wize_api.h"
#include "proto.h"
#include "parameters.h"

static void LoItf_MainTask(void const * argument);
//static void _lo_itf_txevt_(void);
//static void _lo_itf_rxevt_(void);
//static void _lo_itf_fsm(uint32_t u32Evt);
static void _lo_itf_evt_(void *p_CbParam,  uint32_t evt);

#define LO_ITF_TASK_NAME litf
#define LO_ITF_TASK_FCT LoItf_MainTask
#define LO_ITF_STACK_SIZE 300
#define LO_ITF_PRIORITY (UBaseType_t)(tskIDLE_PRIORITY+1)

SYS_TASK_CREATE_DEF(litf, LO_ITF_STACK_SIZE, LO_ITF_PRIORITY);

#define LO_ITF_TMO_EVT 0xFFFFFFFF



typedef enum
{
	LO_ITF_EVT_SENT = 0x01,
	LO_ITF_EVT_RECV = 0x02,
	LO_ITF_EVT_TMO  = 0x03,
	LO_ITF_EVT_WKUP  = 0x04,
} lo_itf_evt_e;

typedef enum
{
	LO_ITF_STATE_IDLE  = 0x00,
	LO_ITF_STATE_BUSY  = 0x01,
	LO_ITF_STATE_SLEEP = 0x10,
	LO_ITF_STATE_WKUP  = 0x20,
} lo_itf_state_e;

typedef enum
{
	LO_ITF_ERR_NONE              = 0x00,
	LO_ITF_ERR_INVALID_CMD       = 0x01,
	LO_ITF_ERR_INVALID_PARAM_ID  = 0x02,
	LO_ITF_ERR_CONFORM_PARAM_VAL = 0x03,
	LO_ITF_ERR_ACCESS            = 0x04,
} lo_itf_error_e;

const char * const loitf_error_msgs[] =
{
    [LO_ITF_ERR_NONE]              = "ERR_NONE",
	[LO_ITF_ERR_INVALID_CMD]       = "INVALID CMD",
	[LO_ITF_ERR_INVALID_PARAM_ID]  = "INVALID ID",
	[LO_ITF_ERR_CONFORM_PARAM_VAL] = "INVALID VAL",
	[LO_ITF_ERR_ACCESS]            = "ACCESS FORBIDDEN",
};

struct circular_buff_s
{
	uint8_t *pData;
	uint16_t u16Size;
	uint16_t u16Head;
	uint16_t u16Tail;
	uint8_t bFull;
} ;

void circular_buff_init(struct circular_buff_s *pCirBuff, uint8_t* pData, uint16_t u16Size)
{
	assert(pCirBuff && pData && u16Size);
	pCirBuff->pData = pData;
	pCirBuff->u16Size = u16Size;
	pCirBuff->u16Head = 0;
	pCirBuff->u16Tail = 0;
	pCirBuff->bFull = 0;
}


uint8_t circular_buff_put(struct circular_buff_s *pCirBuff, uint8_t u8Data)
{

	if(pCirBuff->u16Tail == pCirBuff->u16Head)
	{
		// buffer is empty
	}

	if(pCirBuff->u16Tail +1 == pCirBuff->u16Head)
	{
		// buffer is full
		return 1;
	}

	pCirBuff->pData[pCirBuff->u16Tail++] = u8Data;
	if(pCirBuff->u16Tail >= pCirBuff->u16Size)
	{
		pCirBuff->u16Tail = 0;
	}
	return 0;
}

void circular_buff_gut(struct circular_buff_s *pCirBuff, uint8_t *pData, uint16_t u16Size)
{


}



#define BUF_INPUT_SZ 255
#define BUF_OUTPUT_SZ 255

struct lo_itf_s
{
	void *hTask;
	time_evt_t sTimeOut;
	uint8_t pBufIn[BUF_INPUT_SZ];
	uint8_t pBufOut[BUF_OUTPUT_SZ];

	int16_t i16InSz;
	int16_t i16OutSz;

	lo_itf_state_e eState;

	lo_itf_error_e eError;

	uint8_t mode;
};

static struct lo_itf_s sLoItf;

void LoItf_Setup(void)
{
	sLoItf.hTask = SYS_TASK_CREATE_CALL(litf, LO_ITF_TASK_FCT, NULL);
	assert(sLoItf.hTask);
	BSP_Uart_SetCallback(UART_ID_COM, _lo_itf_evt_, NULL);
	BSP_Uart_Init(UART_ID_COM, '\r', UART_MODE_EOB, 576000); // 5000ms
}

extern boot_state_t gBootState;


static void LoItf_OnReceived(void)
{
	uint8_t *pData = sLoItf.pBufIn;
	uint8_t cmd = pData[0];

	sLoItf.eState = LO_ITF_STATE_BUSY;
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
		if (pData[1] == '0')
		{
			sLoItf.mode = LP_STDBY_MODE;
		}
		else if (pData[1] == '1')
		{
			sLoItf.mode = LP_SHTDWN_MODE;
		}
	}
	else if (cmd == 'T')
	{
		LOG_DBG("ITF Wakeup timer\n");
		TimeEvt_TimerStart(&sLoItf.sTimeOut, 7, 0, (uint32_t)LO_ITF_EVT_WKUP );
	}
	else if (cmd == 'I')
	{
		LOG_DBG("ITF Install\n");
		WizeApi_ExecPing();
	}
	else if (cmd == 'D')
	{
		LOG_DBG("ITF Data\n");
		uint8_t u8Size = strlen( (char*)&(pData[1]) ) -1 ;
		WizeApi_SendEx(&(pData[1]), u8Size, APP_DATA);
	}

	else if (cmd == 'P')
	{
		if ( Param_IsValidId(pData[2]) == 0)
		{
			sLoItf.eError = LO_ITF_ERR_INVALID_PARAM_ID;
		}
		else
		{
			if ( pData[1] )
			{
				if ( Param_CheckConformity(pData[2], &(pData[3]) ) == 0)
				{
					sLoItf.eError = LO_ITF_ERR_CONFORM_PARAM_VAL;
				}
				else
				{
					if (Param_LocalAccess(pData[2], &(pData[3]), 1) == 0)
					{
						sLoItf.eError = LO_ITF_ERR_ACCESS;
					}
				}
			}
			else
			{
				if (Param_LocalAccess(pData[2], sLoItf.pBufOut, 0) )
				{
					sLoItf.i16OutSz = Param_GetSize(pData[2]);
					sLoItf.pBufOut[sLoItf.i16OutSz++] = '\r';
					sLoItf.pBufOut[sLoItf.i16OutSz++] = '\n';
				}
				else
				{
					sLoItf.eError = LO_ITF_ERR_ACCESS;
				}
			}
		}
	}
	else
	{
		sLoItf.eError = LO_ITF_ERR_INVALID_CMD;
		memset(sLoItf.pBufIn, 0, 20);
	}

	if (sLoItf.eError)
	{
		sLoItf.i16OutSz = sprintf((char*)sLoItf.pBufOut, "%s\r\n", loitf_error_msgs[sLoItf.eError]);
		if(sLoItf.i16OutSz <= 0)
		{
			sLoItf.i16OutSz = 0;
		}
		sLoItf.eError = LO_ITF_ERR_NONE;
	}
	sLoItf.eState = LO_ITF_STATE_IDLE;
	pData[0] = 0;
}


static void LoItf_MainTask(void const * argument)
{
	uint32_t ulEvent;
	sLoItf.mode = LP_STDBY_MODE;
	sLoItf.eError = LO_ITF_ERR_NONE;
	sLoItf.eState = LO_ITF_STATE_IDLE;
	sLoItf.i16InSz = 0;
	sLoItf.i16OutSz = 0;

	LOG_DBG("ITF Start\n");

	sLoItf.i16OutSz = sprintf((char*)sLoItf.pBufOut, "B : 0x%lx\n\r", gBootState.state);
	if(sLoItf.i16OutSz > 0)
	{
		sLoItf.eState = LO_ITF_STATE_BUSY;
		BSP_Uart_Transmit(UART_ID_COM, sLoItf.pBufOut, sLoItf.i16OutSz);
	}
	TimeEvt_TimerInit(&(sLoItf.sTimeOut), sLoItf.hTask, TIMEEVT_CFG_ONESHOT);


	while(1)
	{
		BSP_Uart_Receive(UART_ID_COM, sLoItf.pBufIn, 100);
		if (xTaskNotifyWait(0, ULONG_MAX, &ulEvent, LO_ITF_TMO_EVT ))
		{
			switch (ulEvent)
			{
				case UART_EVT_TX_CPLT:
					sLoItf.i16OutSz = 0;
					sLoItf.eState = LO_ITF_STATE_IDLE;
					break;
				case UART_EVT_RX_CPLT:
					if (sLoItf.eState != LO_ITF_STATE_BUSY)
					{
						LoItf_OnReceived();
						if(sLoItf.i16OutSz > 0)
						{
							sLoItf.eState = LO_ITF_STATE_BUSY;
							BSP_Uart_Transmit(UART_ID_COM, sLoItf.pBufOut, (uint16_t)sLoItf.i16OutSz);
						}
					}
					break;
				case UART_EVT_RX_ABT:
					if (sLoItf.eState == LO_ITF_STATE_IDLE)
					{
						sLoItf.eState = LO_ITF_STATE_SLEEP;
						LOG_DBG("ITF LP enter\n");
						//BSP_LowPower_Enter(mode);
					}
					break;
				default:
					break;
			}
		}
	}
}

static void LoItf_TRxTask(void const * argument)
{
	uint32_t ulEvent;
	while(1)
	{
		BSP_Uart_Receive(UART_ID_COM, sLoItf.pBufIn, BUF_INPUT_SZ);
		if (xTaskNotifyWait(0, ULONG_MAX, &ulEvent, LO_ITF_TMO_EVT ))
		{
			switch (ulEvent)
			{
				case LO_ITF_EVT_SENT:
					break;
				case LO_ITF_EVT_RECV:
					break;
				case LO_ITF_EVT_TMO:
					break;
				default:
					break;
			}
		}
		else
		{
			// Tmo
		}
	}
}
/*
static void LoItf_ReceiveTask(void const * argument);

static void LoItf_TransmitTask(void const * argument);

static void LoItf_MainTask(void const * argument)
{
	uint32_t ulEvent;
	while(1)
	{
		if (xTaskNotifyWait(0, ULONG_MAX, &ulEvent, LO_ITF_TMO_EVT ))
		{

		}
	}
}

static void LoItf_TransmitTask(void const * argument)
{
	uint32_t ulEvent;
	while(1)
	{
		if (xTaskNotifyWait(0, ULONG_MAX, &ulEvent, LO_ITF_TMO_EVT ))
		{
			switch (ulEvent)
			{
				case LO_ITF_EVT_SENT:
					break;
				case LO_ITF_EVT_RECV:
					// Decode the frame
					// LoItf_MainTask :
					//    case IDLE
					//    case BUSY
					break;
				case LO_ITF_EVT_RECV:
					BSP_LowPower_Enter(mode);
					break;
				default:
					break;
			}
		}
	}
}
*/

static void _lo_itf_evt_(void *p_CbParam,  uint32_t evt)
{
	BaseType_t xHigherPriorityTaskWoken;
	xTaskNotifyFromISR(sLoItf.hTask, evt, eSetBits, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
/******************************************************************************/

#ifdef __cplusplus
}
#endif
