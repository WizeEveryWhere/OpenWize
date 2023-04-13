/**
  * @file stm32l4xx_hal_hires_time.c
  * @brief // TODO This file ...
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
  * @par 1.0.0 : 2023/04/09 [TODO: your name]
  * Initial version
  *
  */

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
/******************************************************************************/
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_tim.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/**
  * @brief  This function configures the TIM6 as a time base source.
  *         The time source is configured  to have 1ms time base with a dedicated
  *         Tick interrupt priority.
  * @note   This function is called  automatically at the beginning of program after
  *         reset by HAL_Init() or at any time when clock is configured, by HAL_RCC_ClockConfig().
  * @param  TickPriority: Tick interrupt priority.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_HiEesTime_Init(void)
{
	RCC_ClkInitTypeDef    clkconfig;
	uint32_t              uwTimclock = 0;
	uint32_t              uwPrescalerValue = 0;
	uint32_t              pFLatency;

	// Get clock configuration
	HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
	// Compute TIM clock
	uwTimclock = HAL_RCC_GetPCLK1Freq();
	// Compute the prescaler value to have counter clock equal to 1MHz
	uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000) - 1);

/*
	// Enable TIM3 clock
	__HAL_RCC_TIM3_CLK_ENABLE();
	// Initialize TIM3
	htim3.Instance = TIM3;
	htim3.Init.Period = (1000000 / 1000) - 1;
	htim3.Init.Prescaler = uwPrescalerValue;
	htim3.Init.ClockDivision = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;

	TIM_IC_InitTypeDef sConfig;
	sConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfig.ICPrescaler = TIM_ICPSC_DIV1;
	sConfig.ICPolarity  = TIM_ICPOLARITY_RISING;
	sConfig.ICFilter    = 0;
	if (HAL_TIM_IC_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_3) != HAL_OK)
	{
		return HAL_ERROR;
	}

	TIM_MasterConfigTypeDef sMasterConfig;
	sMasterConfig.MasterOutputTrigger  = TIM_TRGO_OC3REF;
	sMasterConfig.MasterSlaveMode      = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);



	// Enable TIM2 clock
	__HAL_RCC_TIM2_CLK_ENABLE();
	// Initialize TIM2
	htim2.Instance = TIM2;
	htim2.Init.Period = 0xFFFFFFFF;
	htim2.Init.Prescaler = uwPrescalerValue;
	htim2.Init.ClockDivision = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;

	if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
	{
		return HAL_ERROR;
	}

	TIM_SlaveConfigTypeDef sSlaveConfig;
	sSlaveConfig.SlaveMode        = TIM_SLAVEMODE_TRIGGER;
	sSlaveConfig.InputTrigger     = TIM_TS_ITR2; // TIM3
	sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
	sSlaveConfig.TriggerPolarity  = TIM_TRIGGERPOLARITY_RISING;
	sSlaveConfig.TriggerFilter    = 0;
	if (HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig) != HAL_OK)
	{
		return HAL_ERROR;
	}

	sConfig.ICSelection = TIM_ICSELECTION_TRC;
	if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfig, TIM_CHANNEL_1) != HAL_OK)
	{
		return HAL_ERROR;
	}

	GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ADF7030_GPIO5_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(ADF7030_GPIO5_GPIO_Port, &GPIO_InitStruct);


	HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_3);
	HAL_TIM_IC_Start(&htim2, TIM_CHANNEL_1);
	*/

	/*
	void LL_TIM_StructInit(LL_TIM_InitTypeDef *TIM_InitStruct);
	ErrorStatus LL_TIM_Init(TIM_TypeDef *TIMx, LL_TIM_InitTypeDef *TIM_InitStruct);

	void LL_TIM_IC_StructInit(LL_TIM_IC_InitTypeDef *TIM_ICInitStruct);
	ErrorStatus LL_TIM_IC_Init(TIM_TypeDef *TIMx, uint32_t Channel, LL_TIM_IC_InitTypeDef *TIM_IC_InitStruct);
	*/


	// Reset and Enable clock
	__HAL_RCC_TIM2_FORCE_RESET();
	__HAL_RCC_TIM2_CLK_ENABLE();
	__HAL_RCC_TIM2_RELEASE_RESET();
	// Initialize TIM2
	htim2.Instance = TIM2;

	// Set the Prescaler value
	htim2.Instance->PSC = uwPrescalerValue;
	htim2.Instance->EGR = TIM_EGR_UG;

	// Configure channels as IC
	htim2.Instance->CCMR1 = (uint32_t)0x303;
	htim2.Instance->CCMR2 = (uint32_t)0x303;
	// Enable channels as IC
	htim2.Instance->CCER = 0x1111;
	// Start TIM2
	__HAL_TIM_ENABLE(&htim2);
	/* Return function status */
	return HAL_OK;

}

HAL_StatusTypeDef HAL_HiResTime_DeInit(void)
{
	__HAL_RCC_TIM2_CLK_DISABLE();
	//__HAL_RCC_TIM3_CLK_DISABLE();

	return HAL_OK;
}


int32_t HiResTime_EnDis(uint8_t bEnable)
{
	if (bEnable)
	{
		if( HAL_HiEesTime_Init())
		{
			return -1;
		}
	}
	else
	{
		HAL_HiResTime_DeInit();
	}
	return 0;
}

//inline
uint32_t HiResTime_Get(register uint8_t id)
{
	//return HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1);
	id--;
	return ((uint32_t*)(&(htim2.Instance->CCR1)))[id & 0x3];
}

//inline
void HiResTime_Capture(register uint8_t id)
{
	//HAL_TIM_GenerateEvent(&htim2, TIM_EVENTSOURCE_CC3);
	id--;
	htim2.Instance->EGR = ( 0b10 << (id & 0x3) );
}

/*
__attribute__((naked))
void HiResTime_Capture1(void)
{
	//htim2.Instance->EGR = 0b00010;
	//uint32_t tim_reg = htim2.Instance->EGR;
	__asm__ volatile (
    	" mov r4, #2   \n"
    	" ldr r5, =%0 \n"
        " str r4, [r5] \n"
        " bx r14       \n"
        "              \n"
        " .align 4     \n"
		:// no output
		:"i" (&(TIM2->EGR)) // no input
		:"r4", "r5", "memory"
    );
}

void HiResTime_Capture2(void)
{
	htim2.Instance->EGR = 0b00100;
}

void HiResTime_Capture3(void)
{
	htim2.Instance->EGR = 0b01000;
}

void HiResTime_Capture4(void)
{
	htim2.Instance->EGR = 0b10000;
}
*/

/*
void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim)
{

}
*/

#ifdef __cplusplus
}
#endif
