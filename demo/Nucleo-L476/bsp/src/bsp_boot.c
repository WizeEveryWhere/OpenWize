/**
  * @file bsp_boot.c
  * @brief Function to boot/reboot the platform
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
  * @par 1.0.0 : 2020/10/05 [GBI]
  * Initial version
  *
  *
  */

/*!
 *  @addtogroup boot
 *  @ingroup bsp
 *  @{
 */

#include "bsp_boot.h"
#include "platform.h"
#include <stm32l4xx_hal.h>

/*!
 * @cond INTERNAL
 * @{
 */

#ifndef BOOT_STATE_BKPR
#define BOOT_STATE_BKPR BKP31R
#endif

/*!
 * @}
 * @endcond
 */

/*!
  * @brief Reboot
  *
  * @param [in] bReset Reset the backup domain (1 ;yes, 0: no)
  *
  * @return None
  *
  */
void BSP_Boot_Reboot(uint8_t bReset)
{
	if (bReset)
	{
		__HAL_RCC_BACKUPRESET_FORCE();
	}
	__HAL_RCC_BACKUPRESET_RELEASE();
	NVIC_SystemReset();
}

/*!
  * @brief Get the boot state
  *
  * @return the boot state as u32 (see @link boot_state_t @endlink)
  *
  */
uint32_t BSP_Boot_GetState(void)
{
	register uint32_t u32BootState = 0;
	// Get Boot reason
	u32BootState |= RCC->CSR >> RCC_CSR_FWRSTF_Pos;

	// Clear reset flags
	__HAL_RCC_CLEAR_RESET_FLAGS();

	// Get if it was in Standby mode
	u32BootState |= (PWR->SR1 & PWR_SR1_SBF)?(STANDBY_WKUP_MSK):(0);
	// Get if it was internal wake-up
	u32BootState |= (PWR->SR1 & PWR_SR1_WUFI)?(INTERNAL_WKUP_MSK):(0);
	// Get wake-up pins
	u32BootState |= (PWR->SR1 & PWR_SR1_WUF) << WKUP_PIN_POS;

	// clear Standby flag and all wake-up flag
	SET_BIT(PWR->SCR, (PWR_SCR_CSBF | PWR_SCR_CWUF) );
	//__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);

	// check wake-up internal : RTC ALARM or RTC WAKEUP TIMER
	if (u32BootState & INTERNAL_WKUP_MSK)
	{
		// Disable RTC protect
		RTC->WPR = 0xCAU;
		RTC->WPR = 0x53U;
		// case RTC WAKEUP TIMER
		if ( RTC->ISR & RTC_FLAG_WUTF )
		{
			u32BootState |= TIMER_WKUP_MSK;
		}
		// case RTC ALARM A // SHOULD never happen, in SHUTDOWN
		if ( RTC->ISR & RTC_FLAG_ALRAF )
		{
			u32BootState |= ALARMA_WKUP_MSK;
		}
		// case RTC ALARM B // SHOULD never happen, in SHUTDOWN
		if ( RTC->ISR & RTC_FLAG_ALRBF )
		{
			u32BootState |= ALARMB_WKUP_MSK;
		}

		// Disable Timer and Alarms
		RTC->CR &= ~(RTC_CR_WUTE | RTC_CR_ALRAE | RTC_CR_ALRBE);
		// Disable Timer and Alarms interrupt
		RTC->CR &= ~(RTC_CR_WUTIE | RTC_CR_ALRAIE | RTC_CR_ALRBIE);
		// Clear flags
		RTC->ISR &= ~(RTC_ISR_WUTF | RTC_ISR_ALRAF | RTC_ISR_ALRBF);
		// Enable RTC protect
		RTC->WPR = 0xFFU;
	}

	// check if BOOT_STATE_BKPR == 0, i.e. Backup Domain Clear/Reset
	if (!( RTC->BOOT_STATE_BKPR ))
	{
		// RTC Calendar Configure is required
		u32BootState |= CALENDAR_UNINIT_MSK | BACKUP_RESET_MSK;
		// Reset the SRAM2
		__HAL_SYSCFG_SRAM2_WRP_UNLOCK();
		__HAL_SYSCFG_SRAM2_ERASE();
	}
	// Save the BootState
	RTC->BOOT_STATE_BKPR = u32BootState;
	return u32BootState;
}

/*! @} */
