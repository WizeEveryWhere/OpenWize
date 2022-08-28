/*!
  * @file bsp_gpio_it.c
  * @brief This file contains functions to deal with external interrupt lines.
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
  * @par 1.0.0 : 2019/12/14 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup gpio_it
 * @ingroup bsp
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp_gpio_it.h"
#include "platform.h"
#include <stm32l4xx_hal.h>

/*!
 * @brief This enum define GPIO port
 */
typedef enum {
    GPIO_PORTA,      /*!< Port 000 */
    GPIO_PORTB,      /*!< Port 001 */
    GPIO_PORTC,      /*!< Port 010 */
	GPIO_PORTD,      /*!< Port 011 */
	GPIO_PORTE,      /*!< Port 100 */
	//GPIO_PORTH,      /*!< Port  */
    GPIO_NUM_PORTS   /*!< maximum number of ports */
} gpio_port_e;

/*!
 * @brief This struct define the gpio interrupt
 */
typedef struct {
	pf_cb_t pf_cb;          /*!< Interruption callback */
	void *p_CbParam;        /*!< Pointer on Callback parameter */
	union
	{
		pf_cb_t pf_cpy_cb; /*!< Callback for gpio copy */
		struct
		{
			uint32_t port; /*!< Gpio copy on port */
			uint16_t pin;  /*!< Gpio copy on pin */
		};
	};
}gpio_it_t;

/*!
 * @cond INTERNAL
 * @{
 */

#define INIT_GPIO_CB() .pf_cb = (pf_cb_t)NULL, .p_CbParam = NULL, .port = 0, .pin = 0
static gpio_it_t aGpioCb[16] = {
		[0] = {INIT_GPIO_CB()},
		[1] = {INIT_GPIO_CB()},
		[2] = {INIT_GPIO_CB()},
		[3] = {INIT_GPIO_CB()},
		[4] = {INIT_GPIO_CB()},
		[5] = {INIT_GPIO_CB()},
		[6] = {INIT_GPIO_CB()},
		[6] = {INIT_GPIO_CB()},
		[7] = {INIT_GPIO_CB()},
		[8] = {INIT_GPIO_CB()},
		[9] = {INIT_GPIO_CB()},
		[10] = {INIT_GPIO_CB()},
		[11] = {INIT_GPIO_CB()},
		[12] = {INIT_GPIO_CB()},
		[13] = {INIT_GPIO_CB()},
		[14] = {INIT_GPIO_CB()},
		[15] = {INIT_GPIO_CB()},
};

#define GET_MODE(GPIOx, pin)  ((GPIOx->MODER >> (pin << 2) ) & 0b11)
#define GET_GPIO_PIN(pin) ((uint32_t)(1 << pin))

/*!
 * @}
 * @endcond
 */

static uint32_t _bsp_gpioit_getport_(uint32_t u32Line);
static gpio_port_e _bsp_gpioit_getnumport_(const uint32_t u32Port);

/*!
  * @static
  * @brief Retrieve GPIO port address from exti line
  *
  * @param [in] u32Line EXTI line number
  *
  * @return the gpio port address
  *
  */
static uint32_t _bsp_gpioit_getport_(uint32_t u32Line)
{
	uint32_t port;

	uint32_t p = SYSCFG->EXTICR[u32Line >> 2u];
	p = ( p >> ((u32Line & 0x03)*4) ) & 0x07;
	switch (p) {
	case 0 :
		port = (uint32_t)GPIOA;
		break;
	case 1 :
		port = (uint32_t)GPIOB;
		break;
	case 2 :
		port = (uint32_t)GPIOC;
		break;
	case 3 :
		port = (uint32_t)GPIOD;
		break;
	case 4 :
		port = (uint32_t)GPIOE;
		break;
	default:
		port = (uint32_t)(NULL);
		break;
	}
	return port;
}

/*!
  * @static
  * @brief Retrieve GPIO port number from port address
  *
  * @param [in] u32Port Gpio port address
  *
  * @return the gpio port number
  *
  */
static gpio_port_e _bsp_gpioit_getnumport_(const uint32_t u32Port)
{
	gpio_port_e gpio_port;
	switch (u32Port) {
	case GPIOA_BASE :
		gpio_port = GPIO_PORTA;
		break;
	case GPIOB_BASE :
		gpio_port = GPIO_PORTB;
		break;
	case GPIOC_BASE :
		gpio_port = GPIO_PORTC;
		break;
	case GPIOD_BASE :
		gpio_port = GPIO_PORTD;
		break;
	case GPIOE_BASE :
		gpio_port = GPIO_PORTE;
		break;
	default:
		gpio_port = GPIO_NUM_PORTS;
		break;
	}
	return gpio_port;
}

/*!
  * @brief Get EXTI line id from gpio pin number
  *
  * @param [in] u16Pin Gpio pin number
  *
  * @return the corresponding exti line
  *
  */
int8_t BSP_GpioIt_GetLineId(const uint16_t u16Pin)
{
	int8_t i8_ItLineId;
	uint16_t u16_GpioPin;

	i8_ItLineId = -1;
	u16_GpioPin = u16Pin;
	// get the EXTI line id
	for (i8_ItLineId = 0 ; i8_ItLineId < 16; i8_ItLineId++){
		if(u16_GpioPin & 0x1){
			break;
		}
		u16_GpioPin = u16_GpioPin >> 1;
	}
	return i8_ItLineId;
}

/*!
  * @brief Configure the gpio interruption
  *
  * @param [in] u32Port Gpio port
  * @param [in] u16Pin  Gpio pin
  * @param [in] ePol    Interrupt polarity
  *
  * @retval DEV_SUCCESS if success (see @link dev_res_e::DEV_SUCCESS @endlink)
  * @retval DEV_FAILURE if fail (see @link dev_res_e::DEV_FAILURE @endlink)
  *
  */
uint8_t BSP_GpioIt_ConfigLine (const uint32_t u32Port, const uint16_t u16Pin, const gpio_irq_trg_cond_e ePol)
{
	int8_t i8_ItLineId;
	uint32_t u32_ItLine;
	uint8_t e_ret;
	gpio_port_e e_Port;

	e_ret = DEV_FAILURE;
	e_Port = _bsp_gpioit_getnumport_(u32Port);
	i8_ItLineId = BSP_GpioIt_GetLineId(u16Pin);

	if (e_Port < GPIO_NUM_PORTS && i8_ItLineId >= 0){
		// setup the required port for the given EXTI line
		SYSCFG->EXTICR[i8_ItLineId >> 2u] |= (e_Port & 0x07) << (i8_ItLineId % 0x04)*4;
		u32_ItLine = (1 << i8_ItLineId);
		// Setup edge
		(ePol & GPIO_IRQ_RISING_EDGE)? ( EXTI->RTSR1 |= u32_ItLine ):( EXTI->RTSR1 &= (~u32_ItLine) );
		(ePol & GPIO_IRQ_FALLING_EDGE)?( EXTI->FTSR1 |= u32_ItLine ):( EXTI->FTSR1 &= (~u32_ItLine) );
		// Disable interrupt for given EXTI line
		EXTI->IMR1 &= (~u32_ItLine);
		e_ret = DEV_SUCCESS;
	}
	return e_ret;
}

/*!
  * @brief Enable the gpio interruption
  *
  * @param [in] u32Port Gpio port
  * @param [in] u16Pin  Gpio pin
  * @param [in] bEnable Enable/Disable interrupt
  *
  * @retval DEV_SUCCESS if success (see @link dev_res_e::DEV_SUCCESS @endlink)
  * @retval DEV_FAILURE if fail (see @link dev_res_e::DEV_FAILURE @endlink)
  *
  */
uint8_t BSP_GpioIt_SetLine (const uint32_t u32Port, const uint16_t u16Pin, const bool bEnable)
{
	UNUSED(u32Port);
	uint8_t e_ret;
	uint32_t u32_ItLine;
	int8_t i8_ItLineId = BSP_GpioIt_GetLineId(u16Pin);
	e_ret = DEV_FAILURE;
	if (i8_ItLineId >= 0){
		u32_ItLine = (1 << i8_ItLineId);
		(bEnable)?(EXTI->IMR1 |= u32_ItLine):(EXTI->IMR1 &= (~u32_ItLine));
		EXTI->PR1 = u32_ItLine;
		e_ret = DEV_SUCCESS;
	}
	return e_ret;
}

/*!
  * @brief Set the gpio interruption callback
  *
  * @param [in] u32Port  Gpio port
  * @param [in] u16Pin   Gpio pin
  * @param [in] pfCb     Pointer on the Callback function
  * @param [in] pCbParam Pointer on the Callback parameter
  *
  * @retval DEV_SUCCESS (see @link dev_res_e::DEV_SUCCESS @endlink)
  *
  */
uint8_t BSP_GpioIt_SetCallback (const uint32_t u32Port, const uint16_t u16Pin, pf_cb_t const pfCb, void *const pCbParam )
{
	UNUSED(u32Port);
	uint8_t u8_ItLineId;
	u8_ItLineId = (uint8_t)BSP_GpioIt_GetLineId(u16Pin);
	aGpioCb[u8_ItLineId].pf_cb = pfCb;
	aGpioCb[u8_ItLineId].p_CbParam = pCbParam;
	return DEV_SUCCESS;
}

/*!
  * @brief Set/Enable the gpio as copy of interrupt of the given exti line
  *
  * @param [in] u8ItLineId EXTI line to copy
  * @param [in] u32Port    Gpio port on which to copy
  * @param [in] u16Pin     Gpio pin on which to copy
  *
  * @retval DEV_SUCCESS if success (see @link dev_res_e::DEV_SUCCESS @endlink)
  *
  */
uint8_t BSP_GpioIt_SetGpioCpy( const uint8_t u8ItLineId, const uint32_t u32Port, const uint16_t u16Pin)
{
	uint8_t u8_ItLineId;
	u8_ItLineId = (uint8_t)u8ItLineId & 0xF;
	aGpioCb[u8_ItLineId].port = u32Port;
	aGpioCb[u8_ItLineId].pin  = u16Pin;
	BSP_Gpio_OutputEnable(u32Port, u16Pin, 1);
	return DEV_SUCCESS;
}

/*!
  * @brief Clr/Disable the gpio as copy of interrupt of the given exti line
  *
  * @param [in] u8ItLineId EXTI line to disable the copy
  *
  * @retval DEV_SUCCESS if success (see @link dev_res_e::DEV_SUCCESS @endlink)
  *
  */
uint8_t BSP_GpioIt_ClrGpioCpy( const uint8_t u8ItLineId)
{
	uint8_t u8_ItLineId;
	u8_ItLineId = (uint8_t)u8ItLineId & 0xF;
	BSP_Gpio_OutputEnable(aGpioCb[u8_ItLineId].port, aGpioCb[u8_ItLineId].pin, 0);
	aGpioCb[u8_ItLineId].port = 0;
	aGpioCb[u8_ItLineId].pin  = 0;
	return DEV_SUCCESS;
}

/*!
  * @brief This is the gpio (exti) interrupt handler
  *
  * @param [in] i8_ItLineId The "interrupted" exti line
  *
  */
void BSP_GpioIt_Handler(int8_t i8_ItLineId)
{
	uint8_t u8_ItLineId;
	u8_ItLineId = (uint8_t)i8_ItLineId & 0xF;

	if (aGpioCb[u8_ItLineId].pf_cb != NULL)
	{
		if (aGpioCb[u8_ItLineId].port)
		{
			BSP_Gpio_Set(aGpioCb[u8_ItLineId].port, aGpioCb[u8_ItLineId].pin, 1);
		}
		aGpioCb[u8_ItLineId].pf_cb(
				aGpioCb[u8_ItLineId].p_CbParam,
				(void *)((uint32_t)(u8_ItLineId))
				);
		if (aGpioCb[u8_ItLineId].port)
		{
			BSP_Gpio_Set(aGpioCb[u8_ItLineId].port, aGpioCb[u8_ItLineId].pin, 0);
		}
	}
}

#ifdef __cplusplus
}
#endif

/*! @} */
