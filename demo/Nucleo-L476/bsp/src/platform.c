/**
  * @file platform.c
  * @brief This file contains some specific platform constants and call-back..
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
  * @par 1.0.0 : 2021/09/09 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup platform
 * @ingroup bsp
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "platform.h"
#include "bsp.h"
#include <stm32l4xx_hal.h>

/*!
 * @cond INTERNAL
 * @{
 */

/******************************************************************************/


extern RTC_HandleTypeDef hrtc;

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef hlpuart1;

uart_dev_t aDevUart[UART_ID_MAX] =
{
	[UART_ID_CONSOLE] = {
			.hHandle = &huart2,
			.pfEvent = NULL
	},
	[UART_ID_COM]     = {
			.hHandle = &huart4,
			.pfEvent = NULL
	},
	[UART_ID_PHY]     = {
			.hHandle = &hlpuart1,
			.pfEvent = NULL
	},
};

/*******************************************************************************/
// RTC related call-back handler
pfHandlerCB_t pfWakeUpTimerEvent = NULL;
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
  UNUSED(hrtc);
  if (pfWakeUpTimerEvent)
  {
	  pfWakeUpTimerEvent();
  }
}

pfHandlerCB_t pfAlarmAEvent = NULL;
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	UNUSED(hrtc);
	if (pfAlarmAEvent)
	{
		pfAlarmAEvent();
	}
}

pfHandlerCB_t pfAlarmBEvent = NULL;
void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc)
{
	UNUSED(hrtc);
	if (pfAlarmBEvent)
	{
		pfAlarmBEvent();
	}
}

/*******************************************************************************/
// UART related call-back handler

static void _send_event_to_cb_(UART_HandleTypeDef *huart, uint32_t evt);

static void _send_event_to_cb_(UART_HandleTypeDef *huart, uint32_t evt)
{
	register uint8_t id;
	for (id = 0; id < UART_ID_MAX; id++)
	{
		if (aDevUart[id].hHandle == huart)
		{
			if (aDevUart[id].pfEvent != NULL)
			{
				aDevUart[id].pfEvent(aDevUart[id].pCbParam, evt);
			}
			break;
		}
	}
}

__weak void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	_send_event_to_cb_(huart, UART_EVT_TX_CPLT);
}

__weak void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	_send_event_to_cb_(huart, UART_EVT_RX_CPLT);
}

__weak void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	_send_event_to_cb_(huart, UART_EVT_RX_HCPLT);
}

__weak void HAL_UART_AbortReceiveCpltCallback(UART_HandleTypeDef *huart)
{
	_send_event_to_cb_(huart, UART_EVT_RX_ABT);
}


/******************************************************************************/
// STDBY and SHUTDOWN LP modes related

// This define the current wake-up pin(s) and polarity
const uint8_t u8WkupPinEn = (LP_WAKEUP_PIN2_EN | LP_WAKEUP_PIN1_EN) & AVAILABLE_WKUP_PIN;
const uint8_t u8WkupPinPoll = (LP_WAKEUP_PIN2_POL_LOW) & AVAILABLE_WKUP_PIN; // | LP_WAKEUP_PIN1_POL_LOW;

// PU or PD during LP Standby/Shutdown modes
const uint16_t u16LpPuPortA = 0 & AVAILABLE_PIN_PORTA_MSK;
const uint16_t u16LpPdPortA = 0 & AVAILABLE_PIN_PORTA_MSK;
const uint16_t u16LpPuPortB = 0 & AVAILABLE_PIN_PORTB_MSK;
const uint16_t u16LpPdPortB = 0 & AVAILABLE_PIN_PORTB_MSK;
const uint16_t u16LpPuPortC = 0 & AVAILABLE_PIN_PORTC_MSK;
const uint16_t u16LpPdPortC = 0 & AVAILABLE_PIN_PORTC_MSK;

// STOP 0, 1, 2 LP modes related
#define COM_TXD_Pin GPIO_PIN_0
#define COM_TXD_GPIO_Port GPIOA

#define COM_RXD_Pin GPIO_PIN_1
#define COM_RXD_GPIO_Port GPIOA

#ifdef COM_SWAP_PINS
	#define WKUP_PIN_NAME COM_TXD
#else
	#define WKUP_PIN_NAME COM_RXD
#endif

struct rcc_clk_state_s
{
	uint32_t ahb1_clk;
	uint32_t ahb2_clk;
	uint32_t ahb3_clk;
	uint32_t apb1r1_clk;
	uint32_t apb1r2_clk;
	uint32_t apb2_clk;
};

static struct rcc_clk_state_s _rcc_clk_state_;

void BSP_LowPower_OnStandbyShutdwnEnter(lp_mode_e eLpMode)
{
	(void)eLpMode;
}

void BSP_LowPower_OnStopEnter(lp_mode_e eLpMode)
{
	(void)eLpMode;
	int8_t i8LineId;
	// Disable all peripheral except SRAM1/2,

	// Save the current rcc clock state
	_rcc_clk_state_.ahb2_clk = RCC->AHB2ENR;
	_rcc_clk_state_.apb1r1_clk = RCC->APB1ENR1;
	_rcc_clk_state_.apb1r2_clk = RCC->APB1ENR2;
	_rcc_clk_state_.apb2_clk = RCC->APB2ENR;

	// Set all ETXI intended to wake-up from STOP (RTC_WKUP, RTC_ALM, PHY_IT, COM_IT)

	BSP_Gpio_InputEnable( LINE_INIT(WKUP_PIN_NAME), 1);
    BSP_GpioIt_ConfigLine( LINE_INIT(WKUP_PIN_NAME), GPIO_IRQ_FALLING_EDGE);
    BSP_GpioIt_SetLine( LINE_INIT(WKUP_PIN_NAME), 1);
    BSP_GpioIt_SetCallback( LINE_INIT(WKUP_PIN_NAME), NULL, NULL );


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

    // Disable all clock
	RCC->APB1ENR1 = 0;
	RCC->APB1ENR2 = 0;
	RCC->APB2ENR = 0;
	RCC->AHB2ENR = 0;
	// Disable the FLASH => require run code and remap vector in SRAM

}

void BSP_LowPower_OnStopExit(lp_mode_e eLpMode)
{
	(void)eLpMode;
	// Restore the current rcc clock state
	RCC->AHB2ENR = _rcc_clk_state_.ahb2_clk;
	RCC->APB1ENR1 = _rcc_clk_state_.apb1r1_clk;
	RCC->APB1ENR2 = _rcc_clk_state_.apb1r2_clk;
	RCC->APB2ENR = _rcc_clk_state_.apb2_clk;

	BSP_GpioIt_SetLine( LINE_INIT(WKUP_PIN_NAME), 0);
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
#ifdef USE_SPI
#if defined(HAL_SPI_MODULE_ENABLED)

extern SPI_HandleTypeDef hspi1;

SPI_HandleTypeDef *paSPI_BusHandle[SPI_ID_MAX] =
{
};

#endif
#endif

/*******************************************************************************/
#ifdef USE_I2C
#if defined(HAL_I2C_MODULE_ENABLED)

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

I2C_HandleTypeDef *paI2C_BusHandle[I2C_ID_MAX] =
{
	[I2C_ID_EXT] = &hi2c2,
};

i2c_dev_t i2c_SSD1306 =
{
	.bus_id = I2C_ID_EXT,
	.device_id = SSD1306_I2C_ADDR,
};

#endif
#endif

/******************************************************************************/
// LPTIM related call-back handler

#ifdef USE_LPTIMER

#if defined(HAL_LPTIM_MODULE_ENABLED)

#if defined (LPTIM1)
extern LPTIM_HandleTypeDef hlptim1;
#endif

#if defined (LPTIM2)
extern LPTIM_HandleTypeDef hlptim2;
#endif

#endif

#if defined(HAL_LPTIM_MODULE_ENABLED)

#if defined (LPTIM1)
pfHandlerCB_t pfLptim1Event = NULL;
#endif

#if defined (LPTIM2)
pfHandlerCB_t pfLptim2Event = NULL;
#endif

void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
#if defined (LPTIM1)
	if ( (hlptim == &hlptim1) && (pfLptim1Event) )
	{
		pfLptim1Event();
	}
#endif
#if defined (LPTIM2)
	if ( (hlptim == &hlptim2) && (pfLptim2Event) )
	{
		pfLptim2Event();
	}
#endif
}
#endif
#endif

/*******************************************************************************/

/*!
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

/*! @} */
