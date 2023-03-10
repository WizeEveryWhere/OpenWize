/**
  * @file bsp_lptimer.c
  * @brief This file contains functions to deal with LPTim as simple timer.
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
  * @par 1.0.0 : 2020/08/29[GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup lptimer
 * @ingroup bsp
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_lptimer.h"
#include "platform.h"

extern void Error_Handler(void);

#if defined(HAL_LPTIM_MODULE_ENABLED)
#if defined (LPTIM1)
extern LPTIM_HandleTypeDef hlptim1;
#endif
#if defined (LPTIM2)
extern LPTIM_HandleTypeDef hlptim2;
#endif

static const uint32_t prescaler_table[] =
{
	LPTIM_PRESCALER_DIV1,
	LPTIM_PRESCALER_DIV2,
	LPTIM_PRESCALER_DIV4,
	LPTIM_PRESCALER_DIV8,
	LPTIM_PRESCALER_DIV16,
	LPTIM_PRESCALER_DIV32,
	LPTIM_PRESCALER_DIV64,
	LPTIM_PRESCALER_DIV128
};

static uint16_t _set_prescaler_(LPTIM_HandleTypeDef *hlptim, uint32_t u32NbClkCyc);

static uint16_t _set_prescaler_(LPTIM_HandleTypeDef *hlptim, uint32_t u32NbClkCyc)
{
//#define NB_CYC_MAX 0x00800000U // 8 388 608 cycles
#define NB_CYC_MAX 0x007FFFFFU // 8 388 607 cycles
	uint32_t tmp, pres;
	uint8_t prescaler_rank = 0;
	uint8_t last_index = (sizeof(prescaler_table) / sizeof(prescaler_table[0])) - 1;

	hlptim->State = HAL_LPTIM_STATE_BUSY;

	// find best prescaler
	pres = (u32NbClkCyc & NB_CYC_MAX );
	while ((pres > 65535) && (prescaler_rank < last_index)) {
		pres = pres >> 1;
		prescaler_rank++;
	}
	// get CFG register
	tmp = hlptim->Instance->CFGR;
	// clear prescaler bits
	tmp &= ~ LPTIM_CFGR_PRESC_Msk;
	// set the new prescaler
	hlptim->Instance->CFGR = tmp | prescaler_table[prescaler_rank];

	hlptim->State = HAL_LPTIM_STATE_READY;
	return ((uint16_t)pres);
}
#endif

// u32Elapse is in ms
uint32_t BSP_LpTimer_Start(const uint8_t u8TimerId, uint32_t u32Elapse)
{
	uint32_t u32NbClkCyc = 0;
	uint32_t frequency;
#if defined(HAL_LPTIM_MODULE_ENABLED)
	LPTIM_HandleTypeDef *pHandle;
	if (u8TimerId)
	{
#if defined (LPTIM1)
		frequency = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_LPTIM1);
		pHandle = &hlptim1;
#else
		return 0;
#endif
	}
	else
	{
#if defined (LPTIM2)
		frequency = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_LPTIM2);
		pHandle = &hlptim2;
#else
		return 0;
#endif
	}
	HAL_LPTIM_SetOnce_Stop_IT(pHandle);
	// u32Elapse is in ms and frequency in Hertz
	u32NbClkCyc = (frequency/1000) * u32Elapse;
	u32NbClkCyc = (uint32_t)_set_prescaler_(pHandle, u32NbClkCyc);
	HAL_LPTIM_SetOnce_Start_IT(pHandle, 0xFFFF, u32NbClkCyc);
#endif
	return u32NbClkCyc;
}

uint32_t BSP_LpTimer_Stop(const uint8_t u8TimerId)
{
	uint32_t u32NbClkCyc = 0;
#if defined(HAL_LPTIM_MODULE_ENABLED)
	LPTIM_HandleTypeDef *pHandle;
	if (u8TimerId)
	{
#if defined (LPTIM1)
		pHandle = &hlptim1;
#else
		return 0;
#endif
	}
	else
	{
#if defined (LPTIM2)
		pHandle = &hlptim2;
#else
		return 0;
#endif
	}
	HAL_LPTIM_SetOnce_Stop_IT(pHandle);
	// Read LPTIM_CNT is not reliable, so read it multiple time
	u32NbClkCyc = HAL_LPTIM_ReadCounter(pHandle);
	for (uint8_t i=0; i < 3; i++)
	{
		uint16_t temp = HAL_LPTIM_ReadCounter(pHandle);
		if (temp == u32NbClkCyc)
		{
			break;
		}
		u32NbClkCyc = temp;
	}
#endif
	return u32NbClkCyc;
}

void BSP_LpTimer_SetHandler (const uint8_t u8TimerId, pfHandlerCB_t const pfCb)
{
	if (u8TimerId)
	{
#if defined (LPTIM1)
		pfLptim1Event = pfCb;
#endif
	}
	else
	{
#if defined (LPTIM2)
		pfLptim2Event = pfCb;
#endif
	}
}

#ifdef __cplusplus
}
#endif

/*! @} */
