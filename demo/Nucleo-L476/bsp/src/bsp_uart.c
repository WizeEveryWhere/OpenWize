/**
  * @file: bsp_uart.c
  * @brief: This file expose public functions of uart devices.
  * 
  *****************************************************************************
  * @Copyright 2019, GRDF, Inc.  All rights reserved.
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
  *****************************************************************************
  *
  * Revision history
  * ----------------
  * 1.0.0 : 2019/12/20[BPI]
  * Initial version
  *
  *
  */

/*!
 * @ingroup Sources
 * @{
 * @ingroup Board
 * @{
 * @ingroup BSP
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_uart.h"
#include "platform.h"
#include <stm32l4xx_hal.h>

extern UART_HandleTypeDef *paUART_BusHandle[UART_ID_MAX];
extern uart_dev_t aDevUart[UART_ID_MAX];
/*******************************************************************************/

void BSP_Console_SetTXCallback (pfHandlerCB_t const pfCb)
{
	pfConsoleTXEvent = pfCb;
}

void BSP_Console_SetRXCallback (pfHandlerCB_t const pfCb)
{
	pfConsoleRXEvent = pfCb;
}

void BSP_Console_SetWakupCallback (pfHandlerCB_t const pfCb)
{
	pfConsoleWakupEvent = pfCb;
}

/*
 * TODO:
 *
 * HAL_UARTEx_StopModeWakeUpSourceConfig(UART_HandleTypeDef *huart, UART_WakeUpTypeDef WakeUpSelection)
 * HAL_UARTEx_EnableStopMode(UART_HandleTypeDef *huart)
 * HAL_UARTEx_DisableStopMode(UART_HandleTypeDef *huart)
 */

/*******************************************************************************/
uint8_t BSP_Console_Init(void)
{
	dev_res_e eRet = DEV_INVALID_PARAM;


	return eRet;
}

uint8_t BSP_Console_Send(uint8_t *pData, uint16_t u16Length)
{
	dev_res_e eRet = DEV_INVALID_PARAM;

	//eRet = HAL_UART_Transmit(paUART_BusHandle[UART_ID_CONSOLE], pData, u16Length, CONSOLE_TX_TIMEOUT);
	//eRet = HAL_UART_Transmit_DMA(paUART_BusHandle[UART_ID_CONSOLE], pData, u16Length);
	eRet = HAL_UART_Transmit_IT(paUART_BusHandle[UART_ID_CONSOLE], pData, u16Length);
	return eRet;
}

uint8_t BSP_Console_Received(uint8_t *pData, uint16_t u16Length)
{
	dev_res_e eRet = DEV_INVALID_PARAM;
	//eRet = HAL_UART_Receive(paUART_BusHandle[UART_ID_CONSOLE], pData, u16Length, CONSOLE_RX_TIMEOUT);
	eRet = HAL_UART_Receive_IT(paUART_BusHandle[UART_ID_CONSOLE], pData, u16Length);

	return eRet;
}
/******************************************************************************/
extern uart_dev_t aDevUart[UART_ID_MAX];

static void _bsp_com_TxISR_8BIT(UART_HandleTypeDef *huart)
{
	uint16_t tmp;

	// Check that a Tx process is ongoing
	if (huart->gState == HAL_UART_STATE_BUSY_TX)
	{
		if (huart->TxXferCount == 0U)
		{
			// Disable the UART Transmit Data Register Empty Interrupt
			CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE);

			// Enable the UART Transmit Complete Interrupt
			SET_BIT(huart->Instance->CR1, USART_CR1_TCIE);
		}
		else
		{
			tmp = (uint16_t)(*huart->pTxBuffPtr & 0xFF);
			if ( (huart->Instance->CR1 & UART_WORDLENGTH_9B ) && (huart->TxXferCount == huart->TxXferSize) )
			{
				// Set the 9th bit as address marker
				tmp |= 0x100;
			}
			huart->Instance->TDR = tmp;
			huart->pTxBuffPtr++;
			huart->TxXferCount--;
		}
	}
	else
	{
		// TX flush request
		__HAL_UART_SEND_REQ(huart, UART_TXDATA_FLUSH_REQUEST);
	}
}

static void _bsp_com_RxISR_8BIT(UART_HandleTypeDef *huart)
{
	uint16_t uhMask = huart->Mask;
	uint16_t uhdata;

	uint8_t evt = UART_EVT_NONE;

	uint32_t isrflags   = READ_REG(huart->Instance->ISR);
	uint32_t cr1its     = READ_REG(huart->Instance->CR1);
	//uint32_t cr3its     = READ_REG(huart->Instance->CR3);

	/* Check that a Rx process is ongoing */
	if (huart->RxState == HAL_UART_STATE_BUSY_RX)
	{
		uhdata = (uint16_t) READ_REG(huart->Instance->RDR);
		// Character match detected ?
		if ( (isrflags & USART_ISR_CMF) )
		{
			if ( (uhdata & 0x100) && (cr1its & UART_WORDLENGTH_9B) )
			{
				// SOB
				huart->pRxBuffPtr -= huart->RxXferSize - huart->RxXferCount;
				huart->RxXferCount = huart->RxXferSize;
				evt = UART_EVT_RX_HCPLT;
			}
			else if ( (cr1its & USART_CR1_CMIE)  )
			{
				/* Disable Character Match interrupt */
				CLEAR_BIT(huart->Instance->CR1, USART_CR1_CMIE);
				// EOB
				evt = UART_EVT_RX_CPLT;
			}
			/* Clear Character Match Flag */
			WRITE_REG(huart->Instance->ICR, USART_ICR_CMCF);
		}

		*huart->pRxBuffPtr = (uint8_t)(uhdata & (uint8_t)uhMask);
		huart->pRxBuffPtr++;
		huart->RxXferCount--;

		if(cr1its & UART_WORDLENGTH_9B)
		{
			// Frame Length is given in the 2nd byte
			if ( (huart->RxXferSize - huart->RxXferCount) == 2)
			{
				if ( (uhdata & 0xFF) < huart->RxXferSize ) // +2
				{
					huart->RxXferCount = uhdata & 0xFF;
				}
				else
				{
					// Cancel : buffer will overflow
					evt = UART_EVT_RX_ABT;
				}
			}
		}

		if ( (huart->RxXferCount == 0U) && (evt != UART_EVT_RX_ABT) )
		{
			evt = UART_EVT_RX_CPLT;
		}

		if ( evt == UART_EVT_RX_CPLT )
		{
			if( !(cr1its & UART_WORDLENGTH_9B))
			{
				*huart->pRxBuffPtr = '\0';
			}
			/* Disable the UART Parity Error Interrupt and RXNE interrupts */
			CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_CMIE));

			/* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
			CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

			/* Rx process is completed, restore huart->RxState to Ready */
			huart->RxState = HAL_UART_STATE_READY;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
			/*Call registered Rx complete callback*/
			huart->RxCpltCallback(huart);
#else
			/*Call legacy weak Rx complete callback*/
			HAL_UART_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
		}
		else if ( evt == UART_EVT_RX_HCPLT )
		{
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
			/*Call registered Rx complete callback*/
			huart->RxHalfCpltCallback(huart);
#else
			/*Call legacy weak Rx half complete callback*/
			HAL_UART_RxHalfCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
		}
	}
	else
	{
		/* RX flush request */
		__HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);
	}


	if ( (isrflags & USART_ISR_RTOF) && (cr1its & USART_CR1_RTOIE) )
	{
		// TMO detected
		evt = UART_EVT_RX_ABT;
		/* Disable Receive Timeout interrupt */
		//CLEAR_BIT(huart->Instance->CR1, USART_CR1_RTOIE);

		/* Clear Receive Timeout interrupt */
		//CLEAR_BIT(huart->Instance->CR1, USART_CR1_IDLEIE);
		WRITE_REG(huart->Instance->ICR, (USART_ICR_RTOCF)); //| USART_ICR_IDLECF) );


		/* Disable Receive Timeout */
		//CLEAR_BIT(huart->Instance->CR2, USART_CR2_RTOEN);
		//huart->gState |= HAL_UART_STATE_TIMEOUT;
		/* Call user Abort complete callback */
	}

	if (evt == UART_EVT_RX_ABT)
	{
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
		/* Call registered Abort Receive Complete Callback */
		huart->AbortReceiveCpltCallback(huart);
#else
		/* Call legacy weak Abort Receive Complete Callback */
		HAL_UART_AbortReceiveCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
		huart->RxState = HAL_UART_STATE_READY;
	}
}

uint8_t BSP_Uart_SetCallback (uint8_t u8DevId, pfEvtCb_t const pfEvtCb, void *pCbParam)
{
	if (u8DevId >= UART_ID_MAX)
	{
		return DEV_INVALID_PARAM;
	}
	aDevUart[u8DevId].pfEvent = pfEvtCb;
	aDevUart[u8DevId].pCbParam = pCbParam;

	//HAL_UART_RegisterCallback(paUART_BusHandle[UART_ID_CONSOLE], HAL_UART_RX_COMPLETE_CB_ID, BSP_RxCpltCallback);
	return DEV_SUCCESS;
}

uint8_t BSP_Uart_Init(uint8_t u8DevId, uint8_t u8CharMatch, uint8_t u8Mode, uint32_t u32Tmo)
{
	UART_HandleTypeDef *huart = aDevUart[u8DevId].hHandler;
	if (u8DevId >= UART_ID_MAX)
	{
		return DEV_INVALID_PARAM;
	}

	__HAL_UART_DISABLE(huart);

	// Disable all interrupt
	CLEAR_BIT(huart->Instance->CR1, (
			USART_CR1_CMIE | USART_CR1_MME | USART_CR1_PCE |
			USART_CR1_PEIE | USART_CR1_TXEIE | USART_CR1_TCIE |
			USART_CR1_RXNEIE | USART_CR1_IDLEIE) );

	aDevUart[u8DevId].u8CharMatch = u8CharMatch;
	aDevUart[u8DevId].u8Mode = u8Mode;

	if(u32Tmo)
	{
		aDevUart[u8DevId].u32Tmo = u32Tmo & 0x00FFFFFF;
		/* Enable Receiver timeout */
		SET_BIT(huart->Instance->CR2, USART_CR2_RTOEN);
		/* Set Receiver timeout value */
		MODIFY_REG(huart->Instance->RTOR, USART_RTOR_RTO, u32Tmo);
	}
	else
	{
		aDevUart[u8DevId].u32Tmo = 0;
		CLEAR_BIT(huart->Instance->CR2, USART_CR2_RTOEN);
	}

	if (u8Mode != UART_MODE_NONE)
	{
		/* Set Address value*/
		MODIFY_REG(huart->Instance->CR2, USART_CR2_ADD, (u8CharMatch << USART_CR2_ADD_Pos) );
		/* Set 7 bits Address */
		SET_BIT(huart->Instance->CR2, USART_CR2_ADDM7);//UART_ADDRESS_DETECT_7B

		if (u8Mode == UART_MODE_ADDR)
		{
			SET_BIT(huart->Instance->CR1, (USART_CR1_MME | USART_CR1_WAKE));
			CLEAR_BIT(huart->Instance->CR1, (USART_CR1_PCE));
		}
	}
	/* Clear all flag : already done with __HAL_UART_DISABLE */
	// WRITE_REG(huart->Instance->ICR, 0xFFFFFFFF );

	huart->RxISR = _bsp_com_RxISR_8BIT;
	huart->TxISR = _bsp_com_TxISR_8BIT;

	 __HAL_UART_ENABLE(huart);
	return DEV_SUCCESS;
}

uint8_t BSP_Uart_Transmit(uint8_t u8DevId, uint8_t *pData, uint16_t u16Length)
{
	UART_HandleTypeDef *huart = aDevUart[u8DevId].hHandler;
	if (u8DevId >= UART_ID_MAX)
	{
		return DEV_INVALID_PARAM;
	}

	/* Check that a Tx process is not already ongoing */
	if (huart->gState == HAL_UART_STATE_READY)
	{
		if ((pData == NULL) || (u16Length == 0U))
		{
			return DEV_INVALID_PARAM;
		}

		__HAL_LOCK(huart);

		huart->pTxBuffPtr  = pData;
		huart->TxXferSize  = u16Length;
		huart->TxXferCount = u16Length;
		huart->TxISR = _bsp_com_TxISR_8BIT;

		huart->ErrorCode = HAL_UART_ERROR_NONE;
		huart->gState = HAL_UART_STATE_BUSY_TX;

		__HAL_UNLOCK(huart);

		/* Enable the Transmit Data Register Empty interrupt */
		SET_BIT(huart->Instance->CR1, USART_CR1_TXEIE);

		return DEV_SUCCESS;
	}
	else
	{
		return DEV_BUSY;
	}
}

uint8_t BSP_Uart_Receive(uint8_t u8DevId, uint8_t *pData, uint16_t u16Length)
{
	UART_HandleTypeDef *huart = aDevUart[u8DevId].hHandler;
	if (u8DevId >= UART_ID_MAX)
	{
		return DEV_INVALID_PARAM;
	}

	register uint32_t itflags = READ_REG(huart->Instance->CR1);
	/* Check that a Rx process is not already ongoing */
	if (huart->RxState == HAL_UART_STATE_READY)
	{
		if ((pData == NULL) || (u16Length == 0U))
		{
			return DEV_INVALID_PARAM;
		}

		__HAL_LOCK(huart);

		huart->pRxBuffPtr  = pData;
		huart->RxXferSize  = u16Length;
		huart->RxXferCount = u16Length;
		//huart->RxISR       = NULL;
		huart->RxISR = _bsp_com_RxISR_8BIT;

		/* Computation of UART mask to apply to RDR register */
		UART_MASK_COMPUTATION(huart);

		huart->ErrorCode = HAL_UART_ERROR_NONE;
		huart->RxState = HAL_UART_STATE_BUSY_RX;

		/* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
		SET_BIT(huart->Instance->CR3, USART_CR3_EIE);

		/* Set Receiver timeout value */
		if(aDevUart[u8DevId].u32Tmo)
		{
			/* Set Receiver timeout value */
			MODIFY_REG(huart->Instance->RTOR, USART_RTOR_RTO, aDevUart[u8DevId].u32Tmo);
			/* Enable Receiver timeout interrupt*/
			itflags |= USART_CR1_RTOIE;
		}
		else
		{
			itflags |= USART_CR1_RTOIE;
		}

		if(aDevUart[u8DevId].u8Mode != UART_MODE_EOB)
		//if(aDevUart[u8DevId].u8Mode == UART_MODE_NONE)
		{
			itflags &= ~(USART_CR1_CMIE);
		}
		else
		{
			/* Enable Character Match interrupt*/
			itflags |= USART_CR1_CMIE;
		}

		//huart->RxISR = _bsp_com_RxISR_8BIT;
		/* Enable the UART Parity Error interrupt and Data Register Not Empty interrupt */
		itflags |= USART_CR1_PEIE | USART_CR1_RXNEIE;
		__HAL_UNLOCK(huart);

		WRITE_REG(huart->Instance->CR1, itflags);

		__HAL_UART_SEND_REQ(huart, UART_MUTE_MODE_REQUEST);

		return DEV_SUCCESS;
	}
	else
	{
		return DEV_BUSY;
	}
}

uint8_t BSP_Uart_AbortReceive(uint8_t u8DevId)
{
	UART_HandleTypeDef *huart = aDevUart[u8DevId].hHandler;
	if (u8DevId >= UART_ID_MAX)
	{
		return DEV_INVALID_PARAM;
	}

	register uint32_t itflags = READ_REG(huart->Instance->CR1);
	if(itflags & USART_CR1_RXNEIE)
	{
		CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
		CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

		/* Reset Rx transfer counter */
	    huart->RxXferCount = 0U;

	    /* Clear RxISR function pointer */
	    huart->pRxBuffPtr = NULL;

	    /* Clear the Error flags in the ICR register */
	    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);

		/* Discard the received data */
		__HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);

		/* Restore huart->RxState to Ready */
	    huart->RxState = HAL_UART_STATE_READY;
	}
	return DEV_SUCCESS;
}
/*******************************************************************************/

#ifdef __cplusplus
}
#endif
