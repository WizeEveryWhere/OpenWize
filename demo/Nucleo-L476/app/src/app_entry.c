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

/*!
 * @}
 * @endcond
 */

/**
  * @brief  Called to initialize application before starting the scheduler.
  */
void App_Init(void)
{
	WizeApp_Init();
	hTask = SYS_TASK_CREATE_CALL(main, APP_TASK_FCT, NULL);
}

/**
  * @static
  * @brief  This is the demo application task
  *
  * @details The task periodically request a PING/PONG or Send a DATA (as admin
  * session). The DATA or ADM session period is 2 s. The PING/PONG session
  * period is 9x the DATA period.
  *
  */
static void Main_Task(void const * argument)
{
	(void)argument;
	const uint32_t tmo = 0xFFFFFFFF;
	uint32_t ulEvent;

	wize_api_ret_e eApiRet;

	uint32_t ret;
	uint8_t state = 1;

	uint8_t i;
	uint8_t pData[50];
	uint8_t u8Size;

	i = 0;
	while(1)
	{
		LOG_INF("-------- \n");
		if (state)
		{
			switch(i)
			{
				case 0:
					LOG_INF("Main start PING/PONG session\n");
					*((inst_ping_t*)pData) = InstInt_Init(&sPingReply);
					eApiRet = WizeApi_ExecPing(pData, sizeof(inst_ping_t));
					if ( eApiRet != WIZE_API_SUCCESS)
					{
						LOG_ERR("Ping/Pong failed");
					}
					break;
				default :
					LOG_INF("Main start DATA session\n");
					// Set the L6App
					pData[0] = 0x05;
					// Set the rest of the message
					strncpy(&pData[1], "Hello World!", 50);
					u8Size = strlen( (char*)pData) ;
					eApiRet = WizeApi_Send(pData, u8Size, APP_DATA);
					if ( eApiRet != WIZE_API_SUCCESS)
					{
						LOG_ERR("DATA failed");
					}
					break;
			}
			if ( eApiRet == WIZE_API_SUCCESS)
			{
				state = 0;
			}
			i++;
			if(i > 9) {i = 0;}
		}

		sys_flag_wait(&ulEvent, tmo);
		ret = WizeApp_Common(ulEvent);

		if (ulEvent & SES_FLG_SES_COMPLETE_MSK)
		{
			if (ulEvent & SES_FLG_SES_ERROR_MSK)
			{
				LOG_DBG("Session Error\n");
			}
			else
			{
				LOG_DBG("Session Success\n");
			}
			msleep(2000);
			state = 1;
		}
		//else { }
	}
}
/******************************************************************************/

#ifdef __cplusplus
}
#endif

/*! @} */
