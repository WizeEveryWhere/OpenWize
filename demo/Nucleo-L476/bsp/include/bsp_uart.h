/*!
  * @file bsp_uart.h
  * @brief This file define bsp functions to access UART preipheral
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
  * @par 1.0.0 : 2020/09/30 [GBI]
  * Initial version
  *
  *
  */

/*!
 * @addtogroup uart
 * @ingroup bsp
 * @{
 */

#ifndef _BSP_UART_H_
#define _BSP_UART_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

/*!
 * @brief This enum define possible events from UART
 */
typedef enum
{
	UART_EVT_NONE     = 0x00, /*!< None */
	UART_EVT_TX_CPLT  = 0x01, /*!< Transmition is complete */
	UART_EVT_RX_CPLT  = 0x02, /*!< Reception is complete */
	UART_EVT_RX_HCPLT = 0x04, /*!< */
	UART_EVT_RX_ABT   = 0x08, /*!< Timeout */
} uart_evt_e;

/*!
 * @brief This enum define flags ... from UART
 */
typedef enum
{
	UART_FLG_NONE,    /*!< */
	UART_FLG_RX_TMO,  /*!< */
	UART_FLG_RX_OVFL, /*!< */
	UART_FLG_RX_SOB,  /*!< */
	UART_FLG_RX_EOB,  /*!< */
} uart_flag_e;

/*!
 * @brief This enum define the "detection" mode for UART
 */
typedef enum
{
	UART_MODE_NONE, /*!< None (wait until the buffer reach the geiven size) */
	UART_MODE_EOB,  /*!< Event is sent when character match the end of block */
	UART_MODE_ADDR, /*!< Event is sent when character match the start of block */
} uart_mode_e;

/*!
 * @brief This structure define the UART device
 */
typedef struct
{
    uint8_t bus_id;       /*!< Peripheral Bus Id */
   	uint8_t u8Mode;       /*!< Current UART device mode */
    uint8_t u8CharMatch;  /*!< Character to match (if mode is enabled) */
   	uint32_t u32Tmo;      /*!< Time-out value (0 : disable) */
    pfEvtCb_t pfEvent;    /*!< Function pointer on event call-back */
    void *pCbParam;       /*!< Pointer on Call-back parameter */
    void *hHandle;        /*!< Pointer on HAL UART handle*/
} uart_dev_t;

/*!
 * @brief This type define a pointer on UART device structure
 */
typedef uart_dev_t* p_uart_dev_t;

/*******************************************************************************/
/*!
 * @cond INTERNAL
 * @{
 */

#ifndef CONSOLE_TX_TIMEOUT
#define CONSOLE_TX_TIMEOUT 2000
#endif
#ifndef CONSOLE_RX_TIMEOUT
#define CONSOLE_RX_TIMEOUT 0xFFFF
#endif

/*!
 * @}
 * @endcond
 */

int __io_putchar(int ch);
int __io_getchar(void);

uint8_t BSP_Console_Init(void);
uint8_t BSP_Console_Send(uint8_t *pData, uint16_t u16Length);
uint8_t BSP_Console_Received(uint8_t *pData, uint16_t u16Length);

uint8_t BSP_Uart_Enable(uint8_t u8DevId);
uint8_t BSP_Uart_Disable(uint8_t u8DevId);
uint8_t BSP_Uart_Init(uint8_t u8DevId, uint8_t u8CharMatch, uint8_t u8Mode, uint32_t u32Tmo);
uint8_t BSP_Uart_SetCallback (uint8_t u8DevId, pfEvtCb_t const pfEvtCb, void *pCbParam);
uint8_t BSP_Uart_Transmit(uint8_t u8DevId, uint8_t *pData, uint16_t u16Length);
uint8_t BSP_Uart_Receive(uint8_t u8DevId, uint8_t *pData, uint16_t u16Length);
uint8_t BSP_Uart_AbortReceive(uint8_t u8DevId);
uint16_t BSP_Uart_GetNbReceive(uint8_t u8DevId);
uint16_t BSP_Uart_GetNbTransmit(uint8_t u8DevId);

/*******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif /* _BSP_UART_H_ */

/*! @} */
