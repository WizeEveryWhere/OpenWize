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

#include "pin_cfg.h"

/*!
 * @cond INTERNAL
 * @{
 */

#define GP_PORT_NAME(name) name ##_GPIO_Port
#define GP_PIN_NAME(name) name##_Pin

#define GP_PORT(name) (uint32_t)(GP_PORT_NAME(name))
#define GP_PIN(name)  GP_PIN_NAME(name)
#define LINE_INIT(name) GP_PORT(name), GP_PIN(name)

/*!
 * @}
 * @endcond
 */

#include "bsp_gpio_it.h"
extern RTC_HandleTypeDef hrtc;
extern UART_HandleTypeDef *paUART_BusHandle[UART_ID_MAX];

/*!
 * @cond INTERNAL
 * @{
 */

#define WKUP_PIN_NAME DEBUG_RXD
#define WKUP_UART_ID UART_ID_CONSOLE
//#define UART_IS_DEINIT

static inline void _lp_clk_cfg_ (void);

//#define CLK_CFG_HSE_PLL_HSI
//#define CLK_CFG_MSI_HSI
static inline void _lp_clk_cfg_ (void)
{
#if defined( CLK_CFG_HSE_PLL_HSI )
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	uint32_t pFLatency = 0;
	HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;

	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

	//RCC_OscInitStruct.OscillatorType |= RCC_OSCILLATORTYPE_HSI;
	//RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	//RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, pFLatency) != HAL_OK)
	{
		Error_Handler();
	}

#elif defined (CLK_CFG_MSI_HSI )
	HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI | RCC_OSCILLATORTYPE_HSI;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, pFLatency) != HAL_OK)
	{
		Error_Handler();
	}

#else // (CLK_CFG_MSI)
	// nothing to do
#endif
}

static void _wkup_cb_(void *pParam, void *pArg)
{

}

static void _lp_entry_ (void)
{
	int8_t i8LineId;

#ifdef UART_IS_DEINIT
	if (HAL_UART_DeInit( paUART_BusHandle[WKUP_UART_ID] ) != HAL_OK)
	{
		Error_Handler();
	}
	BSP_Gpio_InputEnable( LINE_INIT(WKUP_PIN_NAME), 1);
#endif
    BSP_GpioIt_ConfigLine( LINE_INIT(WKUP_PIN_NAME), GPIO_IRQ_FALLING_EDGE);
    BSP_GpioIt_SetLine( LINE_INIT(WKUP_PIN_NAME), 1);
    BSP_GpioIt_SetCallback( LINE_INIT(WKUP_PIN_NAME), &_wkup_cb_, NULL );


    i8LineId = BSP_GpioIt_GetLineId( GP_PIN(WKUP_PIN_NAME));
    if (i8LineId < 5)
    {
    	// IT line from 0, 1, 2, 3 and 4
    	HAL_NVIC_EnableIRQ(EXTI0_IRQn + i8LineId);
    }
    else
    {
        if(i8LineId < 10)
        {
        	// IT line from 5 to 9
        	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
        }
        else
        {
        	// IT line from 10 to 15
        	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
        }
    }
#if defined (CLK_CFG_HSE_PLL_HSI) || defined (CLK_CFG_MSI_HSI)
    // Auto enable HSI when MSI is ready
    SET_BIT(RCC->CR, RCC_CR_HSIASFS );
#endif
}

static void _lp_exit_ (void)
{
	BSP_GpioIt_SetLine( LINE_INIT(WKUP_PIN_NAME), 0);

	_lp_clk_cfg_();

#ifdef UART_IS_DEINIT
	if (HAL_UART_Init( paUART_BusHandle[WKUP_UART_ID] ) != HAL_OK)
	{
		Error_Handler();
	}
#endif
}

void BSP_LowPower_Init (void)
{
/*
 * Power control register 1 (PWR_CR1) : This register is reset after wakeup
 *                                      from Standby mode.
 * Bit 8 DBP: Disable backup domain write protection In reset state, the RTC and backup registers are
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

/*
 * On LP exit
 * - from SHUTDOWN
 * - from STANDBY
 * - from STOP2
 * - from STOP1
 * - from STOP0
 * - from SLEEP
 *
 *
 */

/*
	CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
    bPaState = Phy_GetPa();


    Phy_OnOff(&sPhyDev, 0);

	BSP_LowPower_Enter(LP_STOP2_MODE);

	Phy_OnOff(&sPhyDev, 1);



	Phy_SetPa(bPaState);
    SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
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

// PA0, PC13, PE6, PA2, PC5.
#define PA0_WKUP_PIN  PWR_WAKEUP_PIN1 // PWR_CR3_EWUP1
#define PC13_WKUP_PIN PWR_WAKEUP_PIN2_LOW //PWR_WAKEUP_PIN2 // PWR_CR3_EWUP2
#define PE6_WKUP_PIN  PWR_WAKEUP_PIN3 // PWR_CR3_EWUP3
#define PA2_WKUP_PIN  PWR_WAKEUP_PIN4 // PWR_CR3_EWUP4
#define PC5_WKUP_PIN  PWR_WAKEUP_PIN5 // PWR_CR3_EWUP5

/*!
 * @}
 * @endcond
 */

/*!
 * @brief This define the current wake-up pin
 */
#define WAKEUP_PIN PC13_WKUP_PIN

/*!
  * @brief This function enter in the given low power mode
  *
  * @param [in] eLpMode The low power mode (see @link lp_mode_e @endlink)
  *
  */
void BSP_LowPower_Enter (lp_mode_e eLpMode)
{
	HAL_SuspendTick();
	//HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);

	//_lp_entry_();

	HAL_PWR_EnableWakeUpPin(WAKEUP_PIN);
	HAL_PWREx_EnableInternalWakeUpLine();
	HAL_PWREx_EnableSRAM2ContentRetention();

	switch(eLpMode)
	{
		case LP_SHTDWN_MODE:
			HAL_PWREx_EnterSHUTDOWNMode();
			break;
		case LP_STDBY_MODE:
			HAL_PWR_EnterSTANDBYMode();
			break;
		case LP_STOP1_MODE:
			HAL_PWREx_EnterSTOP1Mode(PWR_STOPENTRY_WFI);
			break;
		case LP_STOP2_MODE:
			HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
			break;
		case LP_SLEEP_MODE:
		default:
			HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
			break;
	}
	//_lp_exit_();
	HAL_ResumeTick();
}

/*
 * Flash option register (FLASH_OPTR)
 * OB_USER_nRST_STOP
 * OB_USER_nRST_STDBY
 * OB_USER_nRST_SHDW
 *
 * OB_USER_IWDG_SW
 * OB_USER_IWDG_STOP
 * OB_USER_IWDG_STDBY
 * OB_USER_WWDG_SW
 */


/******************************************************************************/

#ifdef __cplusplus
}
#endif

/*! @} */
