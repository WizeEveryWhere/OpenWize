/**
  * @file bsp_rtc.h
  * @brief This file defines functions to deal with RTC (Time, Wake-up, Alarm).
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

#ifndef _BSP_RTC_H_
#define _BSP_RTC_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

/*!
 * @cond INTERNAL
 * @{
 */

extern pfHandlerCB_t pfWakeUpTimerEvent;
extern pfHandlerCB_t pfAlarmAEvent;
extern pfHandlerCB_t pfAlarmBEvent;

typedef enum {
	WINTER_TIME_CHANGE = -1,
	NONE_TIME_CHANGE   = 0,
	SUMMER_TIME_CHANGE = 1,
} daylight_sav_e;

/*!
 * @}
 * @endcond
 */

void BSP_Rtc_Setup_Clk(uint32_t clock_sel);
void BSP_Rtc_Setup(uint16_t div_s, uint8_t div_a);

void BSP_Rtc_Backup_Write(uint32_t regNum, uint32_t data);
uint32_t BSP_Rtc_Backup_Read(uint32_t regNum);

void BSP_Rtc_Time_Write(time_t t);
void BSP_Rtc_Time_ReadMicro(struct timeval * tp);
uint64_t BSP_Rtc_Time_GetEpochMs(void);
time_t BSP_Rtc_Time_Read(void);

void BSP_Rtc_Time_UpdateDaylight(daylight_sav_e dayligth_sav);
void BSP_Rtc_Time_Update (time_t t);
void BSP_Rtc_Time_ForceNotify(void);
void BSP_Rtc_Alarm_ForceNotify(void);

void BSP_Rtc_Time_Update_SetCallback (pfEventCB_t const pfCb);

void BSP_Rtc_WakeUpTimer_Enable(void);
void BSP_Rtc_WakeUpTimer_Disable(void);
void BSP_Rtc_WakeUptimer_Reload(void);
void BSP_Rtc_WakeUpTimer_SetCallback (pfEventCB_t const pfCb);
void BSP_Rtc_WakeUpTimer_SetHandler (pfHandlerCB_t const pfCb);

void BSP_Rtc_Alarm_Start(const uint8_t u8AlarmId, uint32_t u32Elapse);
void BSP_Rtc_Alarm_StartMs(const uint8_t u8AlarmId, uint64_t u64Elapse);
void BSP_Rtc_Alarm_Stop(const uint8_t u8AlarmId);
void BSP_Rtc_Alarm_SetHandler (const uint8_t u8AlarmId, pfHandlerCB_t const pfCb);

#ifdef __cplusplus
}
#endif
#endif /* _BSP_RTC_H_ */

/*! @} */
