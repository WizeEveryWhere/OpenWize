/**
  * @file logger.h
  * @brief This file declare available Logger functions
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
  * @par 1.0.0 : 2020/11/15[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup logger
 * @{
 *
 */
#ifndef _LOGGER_H_
#define _LOGGER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "OpenWize_config.h"

/*!
 * This enum define the logger output level
 */
typedef enum
{
	LOG_LV_QUIET   = 0x00, //!< Quite, no log is output
	LOG_LV_ERR     = 0x01, //!< Error log is output
	LOG_LV_WRN     = 0x02, //!< Warning log is output
	LOG_LV_INF     = 0x04, //!< Info log is output
	LOG_LV_DBG     = 0x08, //!< Debug log is output
	LOG_LV_FRM_IN  = 0x40, //!< Frame in log is output
	LOG_LV_FRM_OUT = 0x80  //!< Frame out log is output
} logger_level_e;

#ifdef LOGGER_HAS_COLOR
#define LOG_COLOR_EN_MSK 0x80 //!< Enable/Disable Color mask
/*!
 * This enum define the logger level id, mainly use for add color
 */
typedef enum
{
	LOG_LV_ID_QUIET   = 0x00, //!< Quite level id
	LOG_LV_ID_ERR     = 0x01, //!< Error level id
	LOG_LV_ID_WRN     = 0x02, //!< Warning level id
	LOG_LV_ID_INF     = 0x03, //!< Info level id
	LOG_LV_ID_DBG     = 0x04, //!< Debug level id
	LOG_LV_ID_FRM_IN  = 0x05, //!< Frame level id
	LOG_LV_ID_FRM_OUT = 0x06,  //!< Frame level id
	//
	LOG_LV_ID_MAX
} logger_level_id_e;
#endif

/*!
 * This enum define the Timestamp added to the log output
 */
typedef enum
{
	LOG_TSTAMP_NONE  = 0x0, //!< No Timestamp
	LOG_TSTAMP_NORM  = 0x1, //!< Add Timestamp (epoch) to the log output
	LOG_TSTAMP_HIRES = 0x2, //!< Add Timestamp (epoch) plus millisecond to the log output
	LOG_TSTAMP_TRUNC = 0x4, //!< Truncate Timestamp on 5 digits
} logger_tstamp_e;

extern uint8_t gLoggerLevel;

#ifdef IS_LOGGER_ENABLE

#ifdef LOGGER_USE_FWRITE
void Logger_Setup(int32_t (*pfOut)(const char*, size_t, size_t, FILE*), FILE* pFile);
#else // USE_FPUTS
void Logger_Setup(int32_t (*pfOut)(const char*, FILE*), FILE* pFile);
#endif

void Logger_SetLevel(uint8_t u8LogLevel, uint8_t u8Tstmp);
void Logger_Put(char *str, uint32_t u32Nb);
void Logger_Post(uint8_t level, char *format, ...);
void Logger_Frame(char *pStr, uint8_t *pData, uint8_t u8NbData);

#ifndef LOG_MSG
/* @def  LOG_MSG Macro use to log generic message */
#define LOG_MSG(level, fmt, ...) \
{	\
	Logger_Post(level, fmt, ##__VA_ARGS__); \
}
#endif

#ifndef LOG_ERR
/* @def  LOG_ERR Macro use to log error  message */
#define LOG_ERR(fmt,...) do { if (gLoggerLevel & LOG_LV_ERR) { \
	LOG_MSG(LOG_LV_ID_ERR, "ERR: "fmt, ##__VA_ARGS__); \
} } while(0)
#endif

#ifndef LOG_WRN
/* @def  LOG_MSG Macro use to log warning message */
#define LOG_WRN(fmt,...) do { if (gLoggerLevel & LOG_LV_WRN) { \
	LOG_MSG(LOG_LV_ID_WRN, "WRN: "fmt, ##__VA_ARGS__); \
} } while(0)
#endif

#ifndef LOG_INF
/* @def  LOG_MSG Macro use to log info message */
#define LOG_INF(fmt,...) do { if (gLoggerLevel & LOG_LV_INF) { \
	LOG_MSG(LOG_LV_ID_INF, "INF: "fmt, ##__VA_ARGS__); \
} } while(0)
#endif

#ifndef LOG_DBG
/* @def  LOG_MSG Macro use to log debug message */
#define LOG_DBG(fmt,...) do { if (gLoggerLevel & LOG_LV_DBG) { \
	LOG_MSG(LOG_LV_ID_DBG, "DBG: "fmt, ##__VA_ARGS__); \
} } while(0)
#endif

#ifndef LOG_FRM_OUT
/* @def  LOG_MSG Macro use to log out frame as bytes array*/
#define LOG_FRM_OUT(pFrm, nb_data) do { if (gLoggerLevel & LOG_LV_FRM_OUT) { \
	Logger_Frame("-->: ", pFrm, nb_data); \
} } while(0)
#endif

#ifndef LOG_FRM_IN
/* @def  LOG_MSG Macro use to log in frame as bytes array */
#define LOG_FRM_IN(pFrm, nb_data) do { if (gLoggerLevel & LOG_LV_FRM_IN) { \
	Logger_Frame("<--: ", pFrm, nb_data); \
} } while(0)
#endif

#else
#ifndef Logger_Setup
#define Logger_Setup(...)
#endif
#ifndef Logger_SetLevel
#define Logger_SetLevel(...)
#endif
#ifndef Logger_Put
#define Logger_Put(...)
#endif
#ifndef Logger_Post
#define Logger_Post(...)
#endif
#ifndef Logger_Frame
#define Logger_Frame(...)
#endif

#ifndef LOG_MSG
#define LOG_MSG(level, fmt, ...)
#endif
#ifndef LOG_ERR
#define LOG_ERR(fmt,...)
#endif
#ifndef LOG_WRN
#define LOG_WRN(fmt,...)
#endif
#ifndef LOG_INF
#define LOG_INF(fmt,...)
#endif
#ifndef LOG_DBG
#define LOG_DBG(fmt,...)
#endif
#ifndef LOG_FRM_OUT
#define LOG_FRM_OUT(pFrm, nb_data)
#endif
#ifndef LOG_FRM_IN
#define LOG_FRM_IN(pFrm, nb_data)
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif /* _LOGGER_H_ */

/*! @} */
