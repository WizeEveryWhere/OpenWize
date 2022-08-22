#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"

void Error_Handler(void);

// Blue Button, active low
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC

// Logger, use USART2 via USB on ttyACMx
#define DEBUG_TXD_Pin GPIO_PIN_2
#define DEBUG_TXD_GPIO_Port GPIOA
#define DEBUG_RXD_Pin GPIO_PIN_3
#define DEBUG_RXD_GPIO_Port GPIOA

// Green LED
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA

//
#define EXT_SCL_Pin GPIO_PIN_10
#define EXT_SCL_GPIO_Port GPIOB
#define EXT_SDA_Pin GPIO_PIN_11
#define EXT_SDA_GPIO_Port GPIOB

// Debug Probe
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
