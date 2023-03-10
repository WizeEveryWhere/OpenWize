/**
  * @file port.c
  * @brief This file implement ported function intended to the time_evt module
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
  * @par 1.0.0 : 2021/12/14 [GBI]
  * Initial version
  *
  *
  */

/*!
 *  @addtogroup sys
 *  @ingroup app
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "time_evt.h"
#include "bsp.h"

/******************************************************************************/
void _timer_set_handler(const uint8_t u8TimerId, pfTimeEvt_HandlerCB_t const pfCb);
void _timer_start(const uint8_t u8TimerId, uint64_t u64Value);
void _timer_stop(const uint8_t u8TimerId);
void _get_current_time_ms(uint64_t *u64MsTime);
void _get_current_time(time_t *t);
void _set_current_time(time_t t);

/*!
  * @brief Set the RTC Alarm callback handler
  *
  * @param [in] u8TimerId Alarm Id
  * @param [in] pfCb      Pointer on the callback function
  *
  */
inline
void _timer_set_handler(const uint8_t u8TimerId, pfTimeEvt_HandlerCB_t const pfCb)
{
	BSP_Rtc_Alarm_SetHandler(u8TimerId, pfCb);
}

/*!
  * @brief Start the RTC Alarm
  *
  * @param [in] u8TimerId Alarm Id
  * @param [in] u64Value  Time in millisecond until alarm occurs
  *
  */
inline
void _timer_start(const uint8_t u8TimerId, uint64_t u64Value)
{
	// Check if less than 1 day
	if (u64Value < 86400000)
	{
		BSP_Rtc_Alarm_StartMs(TIME_EVT_ALARM_ID, u64Value);
	}
	else
	{
		BSP_Rtc_Alarm_Stop(TIME_EVT_ALARM_ID);
	}
}

/*!
  * @brief Stop the RTC Alarm
  *
  * @param [in] u8TimerId Alarm Id
  *
  */
inline
void _timer_stop(const uint8_t u8TimerId)
{
	BSP_Rtc_Alarm_Stop(u8TimerId);
}

/*!
  * @brief Get the current epoch in millisecond
  *
  * @param [in] u64MsTime Pointer to hold epoch value
  *
  */
inline
void _get_current_time_ms(uint64_t *u64MsTime)
{
	*u64MsTime = BSP_Rtc_Time_GetEpochMs();
}

/*!
  * @brief Get the current epoch en second
  *
  * @param [in] t Pointer to hold epoch value
  *
  */
inline
void _get_current_time(time_t *t)
{
	*t = BSP_Rtc_Time_Read();
}

/*!
  * @brief Set the current epoch in second
  *
  * @param [in] t Epoch value to set
  *
  */
inline
void _set_current_time(time_t t)
{
	BSP_Rtc_Time_Write(t);
}

/******************************************************************************/
void _time_wakeup_enable(void);
void _time_wakeup_reload(void);
void _time_update_set_handler(pfTimeEvt_HandlerCB_t const pfCb);
void _time_wakeup_force(void);

/*!
  * @brief Enable the RTC wake-up timer
  */
inline
void _time_wakeup_enable(void)
{
	BSP_Rtc_WakeUpTimer_Enable();
}

/*!
  * @brief Reload the RTC wake-up timer
  */
inline
void _time_wakeup_reload(void)
{
	BSP_Rtc_WakeUptimer_Reload();
}

/*!
  * @brief Set the time update callback handler
  *
  * @param [in] pfCb Pointer on the callback function
  *
  */
inline
void _time_update_set_handler(pfTimeEvt_HandlerCB_t const pfCb)
{
	BSP_Rtc_Time_Update_SetCallback(pfCb);
}

/*!
  * @brief Force the RTC wake-up timer notify
  */
inline
void _time_wakeup_force(void)
{
	BSP_Rtc_Time_ForceNotify();
}

#ifdef __cplusplus
}
#endif

/*! @} */
