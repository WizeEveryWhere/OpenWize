/**
  * @file bsp_rtc.c
  * @brief This file contains functions to deal with RTC (Time, Wake-up, Alarm).
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
  * @par 1.0.0 : 2020/08/29 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup rtc
 * @ingroup bsp
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_rtc.h"
#include "platform.h"
#include <stm32l4xx_hal.h>

/*!
 * @cond INTERNAL
 * @{
 */

__attribute__((weak)) __attribute__((used)) daylight_sav_e _daylight_sav_ = NONE_TIME_CHANGE;
__attribute__((weak)) __attribute__((used)) uint32_t _wakeup_period_ = 86400; // in seconds
__attribute__((used)) static pfEventCB_t pfWakeUpTimerCallBack;
__attribute__((used)) static pfEventCB_t pfUpdateTimeCallBack;

/*!
 * @}
 * @endcond
 */

/*******************************************************************************/

extern void Error_Handler(void);
extern RTC_HandleTypeDef hrtc;

static void _rtc_wakeUptimer_handler_(void);
/*******************************************************************************/

/*!
  * @brief This function setup the RTC clock
  *
  * @param [in] clock_sel RTC clock selection
  * 
  * @return None
  * 
  */
void BSP_Rtc_Setup_Clk(uint32_t clock_sel)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInit.RTCClockSelection = clock_sel & RCC_BDCR_RTCSEL;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
}

/*!
  * @brief This function setup the RTC divider
  *
  * @param [in] div_s DIV_S factor
  * @param [in] div_a DIV_A factor
  * 
  * @return None
  * 
  */
void BSP_Rtc_Setup(uint16_t div_s, uint8_t div_a)
{
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

	hrtc.Init.AsynchPrediv = div_a & 0x7F;
	hrtc.Init.SynchPrediv = div_s & 0xFFFF;

	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		Error_Handler();
	}
	HAL_RTCEx_DeactivateRefClock(&hrtc);
	HAL_RTCEx_DeactivateTimeStamp(&hrtc);
	HAL_RTCEx_DeactivateInternalTimeStamp(&hrtc);
	HAL_RTCEx_DeactivateCalibrationOutPut(&hrtc);
}
/*******************************************************************************/

/*!
  * @brief This function write to backup register
  *
  * @param [in] regNum The backup register numbre to write
  * @param [in] data   The value to write in
  * 
  * @return None
  * 
  */
void BSP_Rtc_Backup_Write(uint32_t regNum, uint32_t data)
{
  HAL_RTCEx_BKUPWrite(&hrtc, regNum, data);
}

/*!
  * @brief This function read from backup register
  *
  * @param [in] regNum The backup register numbre to read from
  * 
  * @return The read value
  * 
  */
uint32_t BSP_Rtc_Backup_Read(uint32_t regNum)
{
   return HAL_RTCEx_BKUPRead(&hrtc, regNum);
}
/*******************************************************************************/

/*!
  * @brief This function set the current time
  *
  * @param [in] t The current time to set (in epoch)
  * 
  * @return None
  * 
  */
void BSP_Rtc_Time_Write(time_t t)
{
	struct tm *pTimeInfo;
	RTC_TimeTypeDef sTime = {
			.DayLightSaving = RTC_DAYLIGHTSAVING_NONE,
			.StoreOperation = RTC_STOREOPERATION_RESET,
	};
	RTC_DateTypeDef sDate;
	pTimeInfo = localtime(&t);
	if (pTimeInfo)
	{
		// Setup the structure for the RTC
		sDate.WeekDay = pTimeInfo->tm_wday;
		sDate.Month   = pTimeInfo->tm_mon +1 ;
		sDate.Date    = pTimeInfo->tm_mday;
		sDate.Year    = pTimeInfo->tm_year - 100;
		sTime.Hours   = pTimeInfo->tm_hour;
		sTime.Minutes = pTimeInfo->tm_min;
		sTime.Seconds = pTimeInfo->tm_sec;

		if (HAL_RTC_SetTime(&hrtc, &sTime, FORMAT_BIN) != HAL_OK)
		{
			Error_Handler();
		}
		if (HAL_RTC_SetDate(&hrtc, &sDate, FORMAT_BIN) != HAL_OK)
		{
			Error_Handler();
		}
	}
}

/*!
  * @brief This function read the current time 
  *
  * @param [in] tp Pointer on timeval structure to hold the read time
  * 
  * @return None
  * 
  */
void BSP_Rtc_Time_ReadMicro(struct timeval * tp)
{
    RTC_DateTypeDef sDate;
    RTC_TimeTypeDef sTime;
    struct tm timeinfo;
    if (tp) {
		// Read actual date and time
		// Warning: the time must be read first!
		HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BIN);

		// Setup a tm structure based on the RTC
		timeinfo.tm_wday = sDate.WeekDay;
		timeinfo.tm_mon  = sDate.Month - 1;
		timeinfo.tm_mday = sDate.Date;
		timeinfo.tm_year = sDate.Year + 100;
		timeinfo.tm_hour = sTime.Hours;
		timeinfo.tm_min  = sTime.Minutes;
		timeinfo.tm_sec  = sTime.Seconds;
		// Convert to timestamp
    	tp->tv_sec = mktime(&timeinfo);
    	tp->tv_usec = ((sTime.SecondFraction - sTime.SubSeconds)*1000000)/(sTime.SecondFraction +1);
    }
}

/*!
  * @brief This function get the millisecond epoch time 
  *
  * @return the millisecond epoch time
  * 
  */
uint64_t BSP_Rtc_Time_GetEpochMs(void)
{
	struct timeval tp;
	BSP_Rtc_Time_ReadMicro(&tp);
	return ( (tp.tv_sec*1000)+ (tp.tv_usec/1000) );
}

/*!
  * @brief This function get the epoch time 
  *
  * @return the epoch time
  * 
  */
time_t BSP_Rtc_Time_Read(void)
{
	struct timeval tp;
	BSP_Rtc_Time_ReadMicro(&tp);
	return (time_t)( tp.tv_sec );
}

/*!
  * @brief This function update the RTC clock with summer/winter time
  *
  * @param [in] daylight_sav The daylight 
  * 
  * @return None 
  * 
  */
void BSP_Rtc_Time_UpdateDaylight(daylight_sav_e daylight_sav)
{
    if (daylight_sav == WINTER_TIME_CHANGE)
    {
    	__HAL_RTC_DAYLIGHT_SAVING_TIME_SUB1H(&hrtc, 1);
    }
    else if (daylight_sav == SUMMER_TIME_CHANGE)
    {
    	__HAL_RTC_DAYLIGHT_SAVING_TIME_ADD1H(&hrtc, 0);
    }
    // else, do nothing
}

/*!
  * @brief This function force the RTC wake-up interrupt (in one second)
  *
  */
void BSP_Rtc_Time_ForceNotify(void)
{
	HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 1, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
	/*
	 *  The following should be better, but hal_rtc check that WUTF flag,
	 *  indicating that wakeup timer reload.
	 *  So, the HAL_RTCEx_WakeUpTimerEventCallback is not called
	 *
	 *  // EXTI->SWIER1 |= RTC_EXTI_LINE_WAKEUPTIMER_EVENT;
	 */
}

/*!
  * @brief This function is not implemented
  *
  */
void BSP_Rtc_Alarm_ForceNotify(void)
{
	/*
	 *  The following should be better, but hal_rtc check that RTC_FLAG_ALR(A/B)F flag,
	 *  indicating that alarm timer is reached.
	 *  So, the HAL_RTCEx_Alarm(A/B)EventCallback is not called
	 *
	 *  // EXTI->SWIER1 |= RTC_EXTI_LINE_ALARM_EVENT;
	 */
	//HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, FORMAT_BIN);
}


/*!
  * @brief This function set the callback for "time changed"
  *
  * @param [in] pfCb Callback function 
  * 
  */
void BSP_Rtc_Time_Update_SetCallback (pfEventCB_t const pfCb)
{
	pfUpdateTimeCallBack = pfCb;
}

/*******************************************************************************/

/*!
  * @brief This function enable the RTC wake-up timer
  *
  */
void BSP_Rtc_WakeUpTimer_Enable(void)
{
	BSP_Rtc_WakeUpTimer_SetHandler(NULL);
	BSP_Rtc_WakeUptimer_Reload();
}

/*!
  * @brief This function disable the RTC wake-up timer
  * 
  */
void BSP_Rtc_WakeUpTimer_Disable(void)
{
	HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
}

/*!
  * @brief This function reload the RTC wake-up timer to the next wake-up event (i.e. the next midnight)
  * 
  */
void BSP_Rtc_WakeUptimer_Reload(void)
{
    RTC_DateTypeDef sDate;
    RTC_TimeTypeDef sTime;
	// Read actual date and time
	// Warning: the time must be read first!
	HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BIN);
	uint32_t u32Now = ( (sTime.Hours)*60 + sTime.Minutes )*60  + sTime.Seconds;
	uint32_t u32NextWakeUp = _wakeup_period_ - (u32Now % _wakeup_period_);
	if ( u32NextWakeUp > 0xFFFF )
	{
		// greater than 2^16, so use RTC_WAKEUPCLOCK_CK_SPRE_17BITS
		u32NextWakeUp -= 0xFFFF;
		u32NextWakeUp &= 0xFFFF;
		HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, u32NextWakeUp, RTC_WAKEUPCLOCK_CK_SPRE_17BITS);
	}
	else
	{
		if (u32NextWakeUp == 0) {
			u32NextWakeUp = 1;
		}
		// lower than 2^16, so use RTC_WAKEUPCLOCK_CK_SPRE_16BITS
		u32NextWakeUp &= 0xFFFF;
		HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, u32NextWakeUp, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
	}
}

/*!
  * @brief This function set the callback for the wake-up timer event
  *
  * @param [in] pfCb Callback funtion
  * 
  */
void BSP_Rtc_WakeUpTimer_SetCallback (pfEventCB_t const pfCb)
{
	pfWakeUpTimerCallBack = pfCb;
}

/*!
  * @brief This function set the RTC wake-up timer interrupt handler
  *
  * @param [in] pfCb Interrupt handler
  * 
  */
void BSP_Rtc_WakeUpTimer_SetHandler (pfHandlerCB_t const pfCb)
{
	if (pfCb)
	{
		pfWakeUpTimerEvent = pfCb;
	}
	else
	{
		pfWakeUpTimerEvent = _rtc_wakeUptimer_handler_;
	}
}
/*******************************************************************************/

/*!
  * @brief This function start the given alarm
  *
  * @param [in] u8AlarmId The alarm id to start
  * @param [in] u32Elapse The elapse time in second before alarm occurs
  * 
  */
void BSP_Rtc_Alarm_Start(const uint8_t u8AlarmId, uint32_t u32Elapse)
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	RTC_AlarmTypeDef sAlarm =
	{
		.AlarmTime = {
				.DayLightSaving = RTC_DAYLIGHTSAVING_NONE,
				.StoreOperation = RTC_STOREOPERATION_RESET,
				.TimeFormat = RTC_HOURFORMAT12_AM,
		},
		// Date and day don't care
		.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY, // (RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES | RTC_ALARMMASK_SECONDS),
		// select date (unnecessary)
		.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE,
		.AlarmDateWeekDay = 0x0,
		// Sub-second take care all
		.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE,
	};

	HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BIN);

	if (u32Elapse > 86399){
		u32Elapse = 86399;
	}
									   // with u32Elapse = 86399
	uint16_t Hours = u32Elapse / 3600; // 23
	uint16_t reste = u32Elapse % 3600; // 3599
	uint16_t Minutes = (reste)/60;     // 59
	uint16_t Seconds = (reste)%60;      // 59

	Seconds = Seconds + sTime.Seconds;
	Minutes = Minutes + sTime.Minutes + Seconds/60;
	Hours = Hours + sTime.Hours + Minutes/60;

	sAlarm.AlarmTime.Hours = Hours%24;
	sAlarm.AlarmTime.Minutes = Minutes%60;
	sAlarm.AlarmTime.Seconds = Seconds%60;
	sAlarm.AlarmTime.SubSeconds = sTime.SubSeconds;

	if (u8AlarmId)
	{
		sAlarm.Alarm = RTC_ALARM_B;
	}
	else
	{
		sAlarm.Alarm = RTC_ALARM_A;
	}

	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}

/*!
  * @brief This function start the given alarm at milisecond scale
  *
  * @param [in] u8AlarmId The alarm id to start
  * @param [in] u64Elapse The elapse time in millisecond before alarm occurs
  * 
  */
void BSP_Rtc_Alarm_StartMs(const uint8_t u8AlarmId, uint64_t u64Elapse)
{
	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef sDate;
	RTC_AlarmTypeDef sAlarm =
	{
		.AlarmTime = {
				.DayLightSaving = RTC_DAYLIGHTSAVING_NONE,
				.StoreOperation = RTC_STOREOPERATION_RESET,
				.TimeFormat = RTC_HOURFORMAT12_AM,
		},
		// Date and day don't care
		.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY, // (RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES | RTC_ALARMMASK_SECONDS),
		// select date (unnecessary)
		.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE,
		.AlarmDateWeekDay = 0x0,
		// Sub-second take care all
		.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE,
	};

	HAL_RTC_GetTime(&hrtc, &sTime, FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, FORMAT_BIN);

	uint32_t u32Elapse = u64Elapse/1000;
	uint16_t u16Hours = u32Elapse / 3600;
	uint16_t u16reste = u32Elapse % 3600;
	uint16_t u16Minutes = (u16reste)/60;
	uint16_t u16Seconds = (u16reste)%60;
	uint32_t u32Mseconds = u64Elapse - ((uint64_t)u32Elapse)*1000;

	uint32_t A = (sTime.SecondFraction - sTime.SubSeconds)*1000;
	uint32_t B = A/(sTime.SecondFraction +1);
#ifdef ROUND_CORRECTION
	uint32_t C = A*10/(sTime.SecondFraction +1) - (B)*10;
	B += (C<5)?(0):(1);
#endif

	u32Mseconds = u32Mseconds + B;
	//u32Mseconds = u32Mseconds + (sTime.SecondFraction - sTime.SubSeconds)*1000/(sTime.SecondFraction +1);
	u16Seconds = u16Seconds + sTime.Seconds + u32Mseconds/1000;
	u16Minutes = u16Minutes + sTime.Minutes + u16Seconds/60;
	u16Hours = u16Hours + sTime.Hours + u16Minutes/60;

	u32Mseconds %= 1000;

	A = sTime.SecondFraction*1000 -  ( u32Mseconds * (sTime.SecondFraction +1) );
	B = A/1000;

#ifdef ROUND_CORRECTION
	C = A/100 - B*10;
	B += (C<5)?(0):(1);
#endif

	sAlarm.AlarmTime.Hours = u16Hours%24;
	sAlarm.AlarmTime.Minutes = u16Minutes%60;
	sAlarm.AlarmTime.Seconds = u16Seconds%60;
	sAlarm.AlarmTime.SubSeconds = B;

	if (u8AlarmId)
	{
		sAlarm.Alarm = RTC_ALARM_B;
	}
	else
	{
		sAlarm.Alarm = RTC_ALARM_A;
	}

	if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, FORMAT_BIN) != HAL_OK)
	{
		Error_Handler();
	}
}

/*!
  * @brief This function stop the given alarm
  *
  * @param [in] u8AlarmId The alarm id to stop
  * 
  */
void BSP_Rtc_Alarm_Stop(const uint8_t u8AlarmId)
{
	uint32_t Alarm;
	if (u8AlarmId)
	{
		Alarm = RTC_ALARM_B;
	}
	else
	{
		Alarm = RTC_ALARM_A;
	}
	HAL_RTC_DeactivateAlarm(&hrtc, Alarm);
}

/*!
  * @brief This function set the alarm interrupt handler
  *
  * @param [in] u8AlarmId The alarm id 
  * @param [in] pfCb      The interrupt handler
  * 
  */
void BSP_Rtc_Alarm_SetHandler (const uint8_t u8AlarmId, pfHandlerCB_t const pfCb)
{
	if (u8AlarmId)
	{
		pfAlarmBEvent = pfCb;
	}
	else
	{
		pfAlarmAEvent = pfCb;
	}
}
/*******************************************************************************/

/* Note on RTC Periodic WakeUp
 *
 * input clock :
 * - RTC clock (RTCCLK LSE(32.768kHz)) divided by 2, 4, 8, or 16.
 * --- allow period from 122 μs to 32 s, with a resolution down to 61 μs.
 * - ck_spre (usually 1 Hz internal clock)
 * --- allow period from 1 s to around 36 hours with one-second resolution.
 *
 * First time
 * - Get the current date and time (now)
 * - Compute the delta between 00:00:00 and now
 *      - case 1 : delta in [0; 65535]
 *          => use RTC_WAKEUPCLOCK_CK_SPRE_16BITS (i.e.: WUCKSEL[2:1] = 10)
 *      - case 2 : delta in [65536; 86400]
 *          => use RTC_WAKEUPCLOCK_CK_SPRE_17BITS (i.e.: WUCKSEL[2:1] = 11).
 *          => Subtract 0xFFFF from delta
 * - Program the wake-up timer
 *
 * At next wake-up timer interrupt (time and date should be 00:00:00)
 * - Program the wake-up timer to be in 86400 seconds
 *       => use RTC_WAKEUPCLOCK_CK_SPRE_17BITS
 *       => set the WUTR to 20864  (86400 - 65536)
 *
 * At next wake-up timer interrupt
 * - nothing
 */
 
 /*!
  * @static
  * @brief This function is default RTC wake-up timer interrupt handler
  *
  * @details When wake-up interrupt occurs, both pfUpdateTimeCallBack and 
  * pfWakeUpTimerCallBack callback function will be called. The first one 
  * is intended to treat the time update (time management). The second one
  * is free to be used.
  * 
  */
static void _rtc_wakeUptimer_handler_(void)
{
	// if "new clock update" is required, set it
	if (pfUpdateTimeCallBack)
	{
		pfUpdateTimeCallBack();
	}

	if (pfWakeUpTimerCallBack)
	{
		pfWakeUpTimerCallBack();
	}
}
/*******************************************************************************/

#ifdef __cplusplus
}
#endif

/*! @} */
