/**
  * @file time_evt.h
  * @brief This file contains the basic time event management definitions.
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
  * @par 1.0.0 : 2020/09/09[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup time_evt
 * @{
 *
 */
#ifndef _TIME_EVT_H_
#define _TIME_EVT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/*!
 * @def TIME_EVT_LOCK_TMO
 * @brief This macro define the timeout on OS lock acquire
 */
#ifndef TIME_EVT_LOCK_TMO
	#define TIME_EVT_LOCK_TMO 0xFFFFFFFF
#endif

/*!
 * @def TIME_EVT_ALARM_ID
 * @brief This macro define the used alarm id
 */
#ifndef TIME_EVT_ALARM_ID
	#define TIME_EVT_ALARM_ID 1
#endif

/*!
 * @brief This enum define the timer configuration type
 */
typedef enum
{
	TIMEEVT_CFG_ONESHOT,  //!< One-shot relative timer
	TIMEEVT_CFG_PERIODIC, //!< Periodic relative timer
	TIMEEVT_CFG_ABSOLUTE, //!< One-shot absolute timer
} time_evt_cfg_e;

/*!
 * @brief This struct define the timer context
 */
struct time_evt_s {
	struct time_evt_s *pNext; //!< Pointer on the next timer
#if defined ( __OS__ ) && ( OS_FreeRTOS == 1 )
	void  *pvTaskHandle;      //!< Task handle to notify
#else
	void  (*pvTaskHandle)(uint32_t evt); //!< Pointer on call-back notification function
#endif
	uint64_t u64InitVal;      //!< The initial timer value
	uint64_t u64Value;        //!< The current timer value

	uint32_t u32Event;        //!< Event to notify on expire timer
	time_evt_cfg_e eCfg;      //!< The timer configuration
};

/*!
 * @brief This struct defines the timer type.
 */
typedef struct time_evt_s time_evt_t;

/*!
 * @brief This defines the interrupt call back for the TimeEvt.
 */
typedef void (*pfTimeEvt_HandlerCB_t)(void);


extern void _timer_set_handler(const uint8_t u8TimerId, pfTimeEvt_HandlerCB_t const pfCb);
extern void _timer_start(const uint8_t u8TimerId, uint64_t u64Value);
extern void _timer_stop(const uint8_t u8TimerId);
extern void _get_current_time_ms(uint64_t *u64MsTime);
extern void _set_current_time(time_t t);

#if defined ( __OS__ ) && ( OS_FreeRTOS == 1 )
uint8_t TimeEvt_TimerInit(time_evt_t *pTimeEvt, void *pvTaskHandle, time_evt_cfg_e eCfg);
#else
uint8_t TimeEvt_TimerInit(time_evt_t *pTimeEvt, void (*pvTaskHandle)(uint32_t evt), time_evt_cfg_e eCfg);
#endif
uint8_t TimeEvt_TimerStart(time_evt_t *pTimeEvt, uint32_t u32Elapse, int16_t i16DeltaMs, uint32_t u32Event);
void TimeEvt_TimerStop(time_evt_t *pTimeEvt);

void TimeEvt_Setup(void);
void TimeEvt_Init(void);
void TimeEvt_UpdateTime(time_t t);
void TimeEvt_EventHandler(void);

#ifdef __cplusplus
}
#endif
#endif /* _TIME_EVT_H_ */

/*! @} */
