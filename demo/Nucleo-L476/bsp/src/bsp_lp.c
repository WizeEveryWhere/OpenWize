/**
  * @file bsp_lp.c
  * @brief This implement the bsp function to accesss the low power
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
  * @par 1.0.0 : 2021/11/04 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup low_power
 * @ingroup bsp
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#include "bsp_lp.h"
#include "platform.h"
#include <stm32l4xx_hal.h>

/*!
 * @cond INTERNAL
 * @{
 */

void BSP_LowPower_Init (void)
{
/*
 * Power control register 1 (PWR_CR1) : This register is reset after wakeup
 *                                      from Standby mode.
 * Bit 8 DBP: Disable backup domain write protection
 *
 *
 *
 * Power control register 3 (PWR_CR3) : This register is not reset when exiting
 *                                      Standby modes and with the PWRRST bit
 *                                      in the RCC_APB1RSTR1 register.
 * Bit 15 EIWUL: Enable internal wakeup line
 * Bit 11 ENULP: Enable ULP sampling
 * Bit 10 APC: Apply pull-up and pull-down configuration
 * Bit 8 RRS: SRAM2 retention in Standby mode
 * Bit 4-0 EWUP[5-1]: Enable Wakeup pin WKUP 5-1
 *
 *
 *
 * Power control register 4 (PWR_CR4) : This register is not reset when exiting
 *                                      Standby modes and with the PWRRST bit
 *                                      in the RCC_APB1RSTR1 register.
 * Bit 4-0 WP[5-1]: Wakeup pin WKUP 5-1 polarity
 *
 *
 *
 * Power status register 1 (PWR_SR1) : This register is not reset when exiting
 *                                     Standby modes and with the PWRRST bit
 *                                     in the RCC_APB1RSTR1 register.
 * Bit 15 WUFI: Wakeup flag internal
 * Bit 8 SBF: Standby flag
 * Bit 4-0 WUF[5-1]: Wakeup flag 5-1
 *
 *
 * Power status clear register (PWR_SCR)
 * Bit 8 CSBF: Clear standby flag
 * Bit 4-0 CWUF[5-1]: Clear wakeup flag 5-1
 *
 * Power Port A pull-down control register (PWR_PDCRA)
 * Power Port A pull-up control register (PWR_PUCRA) : This register is not
 *                                                     reset when exiting
 *                                                     Standby modes and with
 *                                                     the PWRRST bit in the
 *                                                     RCC_APB1RSTR1 register.
 *
 *
 * Flash option register (FLASH_OPTR)
 * Bit 25 SRAM2_RST: SRAM2 Erase when system reset
 * Bit 18 IWDG_STDBY: Independent watchdog counter freeze in Standby mode
 * Bit 17 IWDG_STOP: Independent watchdog counter freeze in Stop mode
 * Bit 14 nRST_SHDW
 * Bit 13 nRST_STDBY
 * Bit 12 nRST_STOP
 *
 * SYSCFG SRAM2 control and status register (SYSCFG_SCSR) : System reset value:
 *                                                          0x0000 0000
 * Bit 1SRAM2BSY: SRAM2 busy by erase operation
 * Bit 0 SRAM2ER: SRAM2 Erase
 *
 * *****************************
 * Reset and clock control (RCC)
 *
 * Control/status register (RCC_CSR)
 * Bits 11:8MSISRANGE[3:1] MSI range after Standby mode
 *
 *
 *
 */
}

/*!
 * @}
 * @endcond
 */


/*!
 * @cond INTERNAL
 * @{
 */

__attribute__((weak)) const uint8_t u8WkupPinEn = 0;
__attribute__((weak)) const uint8_t u8WkupPinPoll = 0;
__attribute__((weak)) const uint16_t u16LpPuPortA = 0;
__attribute__((weak)) const uint16_t u16LpPdPortA = 0;
__attribute__((weak)) const uint16_t u16LpPuPortB = 0;
__attribute__((weak)) const uint16_t u16LpPdPortB = 0;
__attribute__((weak)) const uint16_t u16LpPuPortC = 0;
__attribute__((weak)) const uint16_t u16LpPdPortC = 0;
#if defined(GPIOD)
__attribute__((weak)) const uint16_t u16LpPuPortD = 0;
__attribute__((weak)) const uint16_t u16LpPdPortD = 0;
#endif
#if defined(GPIOE)
__attribute__((weak)) const uint16_t u16LpPuPortE = 0;
__attribute__((weak)) const uint16_t u16LpPdPortE = 0;
#endif
#if defined(GPIOF)
__attribute__((weak)) const uint16_t u16LpPuPortF = 0;
__attribute__((weak)) const uint16_t u16LpPdPortF = 0;
#endif
#if defined(GPIOG)
__attribute__((weak)) const uint16_t u16LpPuPortG = 0;
__attribute__((weak)) const uint16_t u16LpPdPortG = 0;
#endif
__attribute__((weak)) const uint16_t u16LpPuPortH = 0;
__attribute__((weak)) const uint16_t u16LpPdPortH = 0;
#if defined(GPIOI)
__attribute__((weak)) const uint16_t u16LpPuPortI = 0;
__attribute__((weak)) const uint16_t u16LpPdPortI = 0;
#endif

static void _bsp_lp_setup_pupd_(void);

static void _bsp_lp_setup_pupd_(void)
{
	/* Set PU/PD on port A */
	WRITE_REG(PWR->PUCRA, u16LpPuPortA);
	WRITE_REG(PWR->PDCRA, u16LpPuPortA);
	/* Set PU/PD on port B */
	WRITE_REG(PWR->PUCRB, u16LpPuPortA);
	WRITE_REG(PWR->PDCRB, u16LpPuPortA);
	/* Set PU/PD on port C */
	WRITE_REG(PWR->PUCRC, u16LpPuPortA);
	WRITE_REG(PWR->PDCRC, u16LpPuPortA);

#if defined(GPIOD)
	/* Set PU/PD on port D */
	WRITE_REG(PWR->PUCRD, u16LpPuPortD);
	WRITE_REG(PWR->PDCRD, u16LpPuPortD);
#endif
#if defined(GPIOE)
	/* Set PU/PD on port E */
	WRITE_REG(PWR->PUCRE, u16LpPuPortE);
	WRITE_REG(PWR->PDCRE, u16LpPuPortE);
#endif
#if defined(GPIOF)
	/* Set PU/PD on port F */
	WRITE_REG(PWR->PUCRF, u16LpPuPortF);
	WRITE_REG(PWR->PDCRF, u16LpPuPortF);
#endif
#if defined(GPIOG)
	/* Set PU/PD on port G */
	WRITE_REG(PWR->PUCRG, u16LpPuPortG);
	WRITE_REG(PWR->PDCRG, u16LpPuPortG);
#endif
	/* Set PU/PD on port H */
	WRITE_REG(PWR->PUCRH, u16LpPuPortH);
	WRITE_REG(PWR->PDCRH, u16LpPuPortH);
#if defined(GPIOI)
	/* Set PU/PD on port I */
	WRITE_REG(PWR->PUCRI, u16LpPuPortI);
	WRITE_REG(PWR->PDCRI, u16LpPuPortI);
#endif
}

/*!
 * @}
 * @endcond
 */

/*!
  * @brief This function enter in the given low power mode
  *
  * @param [in] eLpMode The low power mode (see @link lp_mode_e @endlink)
  *
  */
void BSP_LowPower_Enter (lp_mode_e eLpMode)
{
	if (eLpMode < LP_NB_MODE)
	{
		if ( eLpMode > LP_SHTDWN_MODE ) // SLEEP,
		{
			// TODO : it will no longer stay in sleep mode due to tick and systick interrupt
			HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
		}
		else
		{
			CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
			HAL_SuspendTick();
			if (eLpMode > LP_STOP2_MODE) // STDBY and SHUTDOWN modes
			{
				if (u8WkupPinEn)
				{
					/* Init wake-up pin and polarity */
					CLEAR_BIT(PWR->CR3, u8WkupPinEn );
					CLEAR_BIT(PWR->CR4, u8WkupPinPoll );
					/* Set PU/PD on ports  */
					_bsp_lp_setup_pupd_();
					/* Set wake-up pin polarity */
					SET_BIT(PWR->CR4, u8WkupPinPoll );
					/* Enable wake-up pin, internal wake-up line, SRAM2 retention and PU/PD */
					SET_BIT(PWR->CR3, u8WkupPinEn | PWR_CR3_EIWF | PWR_CR3_RRS | PWR_CR3_APC);
					/* Clear Wake-up Status */
					WRITE_REG(PWR->SCR, 0x11F);
					/* Set LP mode */
					MODIFY_REG(PWR->CR1, PWR_CR1_LPMS, eLpMode );

					/* If required, do something before enter in standby or shutdown */
					BSP_LowPower_OnStandbyShutdwnEnter(eLpMode);

					/* Set SLEEPDEEP bit of Cortex System Control Register */
					SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
					__WFI();
				}
				// else // Unable to Wake-up from external
			}
			else // STOPx modes
			{
				MODIFY_REG(PWR->CR1, PWR_CR1_LPMS, eLpMode);
				BSP_LowPower_OnStopEnter(eLpMode);
				SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
				__WFI();
				CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
				BSP_LowPower_OnStopExit(eLpMode);
			}
			HAL_ResumeTick();
			SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
		}
	}
}

__attribute__((weak)) void BSP_LowPower_OnStandbyShutdwnEnter(lp_mode_e eLpMode)
{
	(void)eLpMode;
	/*
	 * Warning, at this point :
	 * - SYS_TICK is disable, so RTOS is stalled
	 * - HAL tick is suspend, so HAL timeout is stalled
	 */
}

__attribute__((weak)) void BSP_LowPower_OnStopEnter(lp_mode_e eLpMode)
{
	(void)eLpMode;
}

__attribute__((weak)) void BSP_LowPower_OnStopExit(lp_mode_e eLpMode)
{
	(void)eLpMode;
}

/******************************************************************************/

#ifdef __cplusplus
}
#endif

/*! @} */
