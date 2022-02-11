#ifndef _PIN_CFG_H_
#define _PIN_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32l4xx_hal.h>

/**************************** GPIO pinout *************************************/
// SWD
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

// Debug
#define DEBUG_TXD_Pin GPIO_PIN_0
#define DEBUG_TXD_GPIO_Port GPIOA
#define DEBUG_RXD_Pin GPIO_PIN_1
#define DEBUG_RXD_GPIO_Port GPIOA

#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA

#ifdef __cplusplus
}
#endif

#endif /* _PIN_CFG_H_ */
