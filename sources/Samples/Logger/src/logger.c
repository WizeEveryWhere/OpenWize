
/**
  * @file logger.c
  * @brief This file implement a Logger module to help print log messages.
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
  * @par 1.0.0 : 2020/11/14[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup logger
 * @{
 *
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <sys/time.h>
#include <limits.h>

#include "rtos_macro.h"
#include "logger.h"

#ifdef LOGGER_HAS_COLOR
#include "color.h"
extern const char *color_str[LOG_LV_ID_MAX];
#endif

/*!
 * @brief This global variable define the output logger level.
 */
uint8_t gLoggerLevel = LOG_LV_QUIET;

/*!
 * @cond INTERNAL
 * @{
 */

/* @def This macro define the logger stack size */
#define LOGGER_TASK_STACK_SIZE 200
/* @def This macro define the logger task priority */
#define LOGGER_TASK_PRIORITY (UBaseType_t)(tskIDLE_PRIORITY+2)
/* @def This macro define the number of item available in the logger message queue */
#define LOGGER_QUEUE_MESSAGE_ITEM_NB 15
/* @def This macro define the size of item in the logger message queue*/
#define LOGGER_QUEUE_MESSAGE_ITEM_SZ sizeof(uint32_t)
/* @def This macro define the available logger buffer size to store messages */
#define LOGGER_POOL_BUFFER_SIZE 90

SYS_TASK_CREATE_DEF(logger, LOGGER_TASK_STACK_SIZE, LOGGER_TASK_PRIORITY);
SYS_CNTSEM_CREATE_DEF(logger, LOGGER_QUEUE_MESSAGE_ITEM_NB, LOGGER_QUEUE_MESSAGE_ITEM_NB);
SYS_QUEUE_CREATE_DEF(logger, LOGGER_QUEUE_MESSAGE_ITEM_NB, LOGGER_QUEUE_MESSAGE_ITEM_SZ);
SYS_MUTEX_CREATE_DEF(logger);

static int32_t _logger_add_timestamp_(int32_t id);
static int32_t _logger_acquire_id_();
static void _logger_release_id_(int32_t id);
static void _logger_main_( void const* argument);

/*!
 * @}
 * @endcond
 */

/* @struct This structure hold the logger context variables*/
struct logger_ctx_s
{
	void* hTask;     //<! The task handle
	void* hMutex;    //<! Mutex use to lock access to the message buffer index
	void* hQueue;    //<! Synchronization Queue between logger task and other caller task
	void* hCntSem;   //<! Counting semaphore use to trace available buffer index
	FILE* pFile;     //!< Output File pointer
#ifdef LOGGER_USE_FWRITE
	int32_t (*pfOut)(const char*, size_t, size_t, FILE*); //!< Output function pointer
	uint8_t aPoolLen[LOGGER_QUEUE_MESSAGE_ITEM_NB]; //<! Message length available in Buffer
#else // USE_FPUTS
	int32_t (*pfOut)(const char*, FILE*); //!< Output function pointer
#endif
	uint8_t aPoolBuffer[LOGGER_QUEUE_MESSAGE_ITEM_NB][LOGGER_POOL_BUFFER_SIZE+1];
													 //<! Buffer messages
	uint8_t aPoolLock[LOGGER_QUEUE_MESSAGE_ITEM_NB]; //<! Buffer index locks
	uint8_t u8Tstmp; //<! Variable to setup the timestamp mode used in formated output message
};

/* @var sLoggerCtx variable to hold the Logger context*/
static struct logger_ctx_s sLoggerCtx;

/*!
 * @brief This function initialize the Logger module
 *
 * @return None
 */
#ifdef LOGGER_USE_FWRITE
void Logger_Setup(int32_t (*pfOut)(const char*, size_t, size_t, FILE*), FILE* pFile)
#else
void Logger_Setup(int32_t (*pfOut)(const char*, FILE*), FILE* pFile)
#endif
{
	sLoggerCtx.hMutex = SYS_MUTEX_CREATE_CALL(logger);
	assert(sLoggerCtx.hMutex);
	sLoggerCtx.hQueue = SYS_QUEUE_CREATE_CALL(logger);
	assert(sLoggerCtx.hQueue);
	sLoggerCtx.hCntSem = SYS_CNTSEM_CREATE_CALL(logger);
	assert(sLoggerCtx.hCntSem);
	sLoggerCtx.hTask = SYS_TASK_CREATE_CALL(logger, &_logger_main_, NULL);
	assert(sLoggerCtx.hTask);
	memset(sLoggerCtx.aPoolLock, 0x00, LOGGER_QUEUE_MESSAGE_ITEM_NB);

	sLoggerCtx.pfOut = pfOut;
	sLoggerCtx.pFile = pFile;
}


/*!
 * @brief This function setup the Logger module
 *
 * @param[in] u8LogLevel Logger level (ORed logger_level_e values)
 * @param[in] u8Tstmp    Type of time-stamp to use
 *
 * @return None
 */
inline void Logger_SetLevel(uint8_t u8LogLevel, uint8_t u8Tstmp)
{
	gLoggerLevel = u8LogLevel;
	sLoggerCtx.u8Tstmp = u8Tstmp;
}

/*!
 * @brief This function "put" a raw string to the logger
 *
 * @param[in] str   Raw string
 * @param[in] u32Nb The number of char to logout
 *
 * @return None
 */
void Logger_Put(char *str, uint32_t u32Nb)
{
    int32_t id;
    uint32_t len;
    uint8_t *p;
    if (str)
    {
		if ( xSemaphoreTake(sLoggerCtx.hCntSem, 0xFFFFFFFF))
		{
			id = _logger_acquire_id_();
			if ( id >= 0 )
			{
				p = sLoggerCtx.aPoolBuffer[id];
				len = u32Nb;
				if (len > LOGGER_POOL_BUFFER_SIZE)
				{
					len = LOGGER_POOL_BUFFER_SIZE;
				}
				memcpy((char*) p, str, len);
				*(p+len) = '\0';
#ifdef LOGGER_USE_FWRITE
				sLoggerCtx.aPoolLen[id] = (uint8_t)len;
#endif
				xQueueSend(sLoggerCtx.hQueue, &id, 0xFFFFFFFF);
			}
		}
    }
}

/*!
 * @brief This function "post" a formated string to the logger
 *
 * @param[in] level  Level of verbosity
 * @param[in] format Formated string
 * @param[in] ...    va_args List of variable to log
 *
 * @return None
 */
void Logger_Post(uint8_t level, char *format, ...)
{
    va_list args;
    int32_t id, max;
    uint32_t len;
    uint8_t *p;

    if ( xSemaphoreTake(sLoggerCtx.hCntSem, 0xFFFFFFFF))
    {
		id = _logger_acquire_id_();
		if ( id >= 0 )
		{
			va_start (args, format);
			// fill the timestamp
			len = _logger_add_timestamp_(id);
			p = sLoggerCtx.aPoolBuffer[id];
			max = LOGGER_POOL_BUFFER_SIZE - len;
			if (max > 0)
			{
				p += len;
//#define LOGGER_HAS_COLOR
#ifdef LOGGER_HAS_COLOR
				if (sLoggerCtx.u8Tstmp & LOG_COLOR_EN_MSK)
				{
					// fill the color (opening part)
					len = snprintf((char*) p, max, color_str[level] );
					// In the following the '-1' is to remove ending '\0' (due to the #define)
					max -= len + sizeof(END_ESC) - 1;
					p += len;
					if (max > 0)
					{
						// fill the message
						len = vsnprintf((char*) p, max, format, args);
						p += len;
						p--;

						/*
						if (len > max)
						{
							p += max;
							*(p-1) = '\n';
						}
						else
						{
							p += len;
						}
						*/
					}
					// fill the color (ending part)
					len = snprintf((char*) p, sizeof(END_ESC), END_ESC);
					// add new line
					*(p + len) = '\n';
					len++;
				}
				else
#endif
				{
					// fill the message
					len = vsnprintf((char*) p, max, format, args);
				}
			}
#ifdef LOGGER_USE_FWRITE
			sLoggerCtx.aPoolLen[id] = (uint8_t)( p - sLoggerCtx.aPoolBuffer[id] +  len );
#endif
			va_end (args);
			xQueueSend(sLoggerCtx.hQueue, &id, 0xFFFFFFFF);
		}
    }
}

/*!
 * @brief This function log a frame as formated string of bytes array
 *
 * @param[in] pStr     Formated string
 * @param[in] pData    Frame data to log
 * @param[in] u8NbData Number of byte to log
 *
 * @return None
 */
void Logger_Frame(char *pStr, uint8_t *pData, uint8_t u8NbData)
{
	int32_t id, max;
    uint32_t len;
    uint8_t *p;
	uint8_t u8Idx;
	// At least one of the two input buffer has to be available
	if(pData == NULL && pStr == NULL)
	{
		return;
	}
    if ( xSemaphoreTake(sLoggerCtx.hCntSem, 0xFFFFFFFF))
    {
		id = _logger_acquire_id_();
		if ( id >= 0 )
		{
			len = _logger_add_timestamp_(id);
			p = sLoggerCtx.aPoolBuffer[id] + len;
			max = LOGGER_POOL_BUFFER_SIZE - len -1;

			if (pStr)
			{
				len = snprintf((char *)p, max, "%s", pStr);
				p += len;
				max -= len;
			}
			if (pData)
			{
				if( max > 0 )
				{
					*p = '[';
					p++; max --;
				}
				for (u8Idx = 0; u8Idx < u8NbData;  u8Idx++)
				{
					len = snprintf((char *)p, max, "%02x ", pData[u8Idx]);
					max -= len;
					if( max <= 0 )
					{
						break;
					}
					p += len;
				}
				if( max > 0 )
				{
					*p = ']';
					p++; max --;
				}
			}
			//*p = "\n";
			len = snprintf((char *)p, 2, "\n");
#ifdef LOGGER_USE_FWRITE
			sLoggerCtx.aPoolLen[id] = (uint8_t)( p - sLoggerCtx.aPoolBuffer[id] +  len );
#endif
			//sLoggerCtx.aPoolBuffer[id][LOGGER_POOL_BUFFER_SIZE] = "\n";
			xQueueSend(sLoggerCtx.hQueue, &id, 0xFFFFFFFF);
		}
    }
}

/*!
 * @static
 * @brief Internal function to add timestamp
 *
 * @param[in] id The buffer id on which to add the timestamp
 *
 * @return The number of written bytes
 */
static int32_t _logger_add_timestamp_(int32_t id)
{
	struct timeval tp;
    uint32_t len;
    uint8_t *p;

    len = 0;
	if(sLoggerCtx.u8Tstmp >= LOG_TSTAMP_NORM )
	{
		p = sLoggerCtx.aPoolBuffer[id];
		gettimeofday(&tp, NULL);
		if(sLoggerCtx.u8Tstmp & LOG_TSTAMP_TRUNC)
		{
			tp.tv_sec -= ((uint32_t)(tp.tv_sec) / 100000 ) * 100000;
			len = snprintf((char *)p, LOGGER_POOL_BUFFER_SIZE, "[%05lu", (uint32_t)(tp.tv_sec));
		}
		else
		{
			len = snprintf((char *)p, LOGGER_POOL_BUFFER_SIZE, "[%lu", (uint32_t)(tp.tv_sec));
		}
		p += len;

		if(sLoggerCtx.u8Tstmp & LOG_TSTAMP_HIRES )
		{
			tp.tv_usec = tp.tv_usec/1000;
			len = snprintf((char *)p, LOGGER_POOL_BUFFER_SIZE - len , ".%03lu", (uint32_t)(tp.tv_usec));
			p += len;
		}
		*p = ']';
		p++;
		len = p - sLoggerCtx.aPoolBuffer[id];
	}
	return len;
}


/*!
 * @static
 * @brief Internal function to acquire an available buffer id
 *
 * @return The buffer id
 */
static int32_t _logger_acquire_id_()
{
	int32_t i;
	// take mutex
	xSemaphoreTake (sLoggerCtx.hMutex, 0xFFFFFFFF);
	// try to find a free buffer
	for (i = 0; i < LOGGER_QUEUE_MESSAGE_ITEM_NB; i++)
	{
		// this is not lock, so take it
		if(sLoggerCtx.aPoolLock[i] == 0)
		{
			sLoggerCtx.aPoolLock[i] = 1;
			break;
		}
	}
	if (i >= LOGGER_QUEUE_MESSAGE_ITEM_NB) { i = -1; }
    // release mutex
	xSemaphoreGive(sLoggerCtx.hMutex);
	return i;
}

/*!
 * @static
 * @brief Internal function to release a buffer id
 *
 * @param[in] id The buffer id to release
 *
 * @return None
 */
static void _logger_release_id_(int32_t id)
{
	if (id >= 0 && id < LOGGER_QUEUE_MESSAGE_ITEM_NB)
	{
		// take mutex
		xSemaphoreTake (sLoggerCtx.hMutex, 0xFFFFFFFF);
		sLoggerCtx.aPoolLock[id] = 0;
		// release mutex
		xSemaphoreGive(sLoggerCtx.hMutex);
	}
}

/*!
 * @static
 * @brief This is the main task function
 *
 * @param [in] argument (not used)
 *
 * @return      None
 */
static void _logger_main_(void const* argument)
{
	(void)(argument);
	int32_t id;
    while(1)
    {
        /* Block to wait for the next string to print. */
    	if ( xQueueReceive(sLoggerCtx.hQueue, &id, 0xFFFFFFFF) )
    	{
    		if (sLoggerCtx.pfOut)
    		{
#ifdef LOGGER_USE_FWRITE
    			sLoggerCtx.pfOut( (char*)(sLoggerCtx.aPoolBuffer[id]), (size_t)1, (size_t)sLoggerCtx.aPoolLen[id], sLoggerCtx.pFile );
#else // USE_FPUTS
    			sLoggerCtx.pfOut( (char*)(sLoggerCtx.aPoolBuffer[id]), sLoggerCtx.pFile );
#endif
    		}
    		_logger_release_id_(id);
    		xSemaphoreGive(sLoggerCtx.hCntSem);
    	}
    }
}

/*******************************************************************************/

#ifdef __cplusplus
}
#endif

/*! @} */
