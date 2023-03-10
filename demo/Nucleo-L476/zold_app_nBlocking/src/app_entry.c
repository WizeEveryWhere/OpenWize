/**
  * @file app_entry.c
  * @brief This implement the default entry point after board initialization
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
  * @par 1.0.0 : 2019/11/20 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup app
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "app_entry.h"

extern void Sys_Init(void);
extern void Sys_Start(void);

void App_Init(void);

/**
  * @brief  The application entry point.
  * @retval None
  */
void app_entry(void)
{
  	Sys_Init();
  	App_Init();
  	Sys_Start();
}

/******************************************************************************/

/*!
 * @cond INTERNAL
 * @{
 */

void* hTask;
static void Main_Task(void const * argument);
#define APP_TASK_NAME main
#define APP_TASK_FCT Main_Task
#define APP_STACK_SIZE 300
#define APP_PRIORITY (UBaseType_t)(tskIDLE_PRIORITY+1)
SYS_TASK_CREATE_DEF(main, APP_STACK_SIZE, APP_PRIORITY);

#define APP_TMO_EVT 0xFFFFFFFF

/*!
 * @}
 * @endcond
 */

/**
  * @brief  Called to initialize application before starting the scheduler.
  */
void App_Init(void)
{
	hTask = SYS_TASK_CREATE_CALL(main, APP_TASK_FCT, NULL);
}

/**
  * @static
  * @brief  This is the demo application task
  */
static void Main_Task(void const * argument)
{
	(void)argument;

	uint8_t i;
	uint8_t pData[50];
	uint8_t u8Size;
	uint8_t ret;
	uint32_t ulEvent;
	TickType_t xTicksToDelay;
	TickType_t xTicksToEvent;
	uint8_t bIsRunning = 0;

	// Set the L6App
	pData[0] = 0x05;
	// Set the rest of the message
	strncpy(&pData[1], "Hello World!", 50);
	u8Size = strlen( (char*)pData) ;

	xTicksToDelay = 1000;
	xTicksToEvent = 500;
	i = 0;
	while(1)
	{
		LOG_INF("---------- \n");
		if (bIsRunning)
		{
			if (xTaskNotifyWait(0, ULONG_MAX, &ulEvent, xTicksToEvent ))
			{
				if ( ulEvent & WIZE_API_FLG_FAILED)
				{
					LOG_WRN("Main %d : session failed\n", i);
				}
				else if ( ulEvent & WIZE_API_FLG_SUCCESS)
				{
					if ( ulEvent & WIZE_API_FLG_REQUEST)
					{
						LOG_INF("Main %d : session with CMD\n", i);
					}
					else
					{
						LOG_INF("Main %d : session success\n", i);
					}
				}
				else // WIZE_API_FLG_NONE
				{
				}
				i++;
				if(i > 9) {i = 0;}
				bIsRunning = 0;
			}
			else
			{
				LOG_WRN("Main %d : no event tmo - %d ms\n", i, xTicksToEvent);
			}
		}

		switch(i)
		{
			case 0:
				LOG_INF("Main %d : Install\n", i);
				ret = WizeApi_ExecPing();
				break;
			default :
				LOG_INF("Main %d : Data\n", i);
				ret = WizeApi_Send(pData, u8Size, APP_DATA);
				break;
		}
		if (ret == WIZE_API_ACCESS_TIMEOUT)
		{
			xTicksToDelay = 1000;
			LOG_WRN("Main %d : device busy\n", i);
		}
		else
		{
			bIsRunning = 1;
			xTicksToDelay = 0;
		}

		if ( xTicksToDelay )
		{
			LOG_INF("Main %d : wait delay - %d ms\n", i, xTicksToDelay);
			vTaskDelay(xTicksToDelay );
		}

	}
}
/******************************************************************************/

#ifdef __cplusplus
}
#endif

/*! @} */
