/*
 * bsp.h
 *
 *  Created on: Nov 26, 2021
 *      Author: hs
 */

#ifndef SRC_BSP_BSP_H_
#define SRC_BSP_BSP_H_

#ifdef __cplusplus
 extern "C" {
#endif



#include "def.h"


#include "stm32f7xx_hal.h"
#include "rtos.h"



#define _USE_DBG_PRINTF    1


#if _USE_DBG_PRINTF
#define dbgPrintf(fmt, ...)     printf(fmt, ##__VA_ARGS__)
#else
#define dbgPrintf(fmt, ...)
#endif

void logPrintf(const char *fmt, ...);




bool bspInit(void);
bool bspDeInit(void);

void delay(uint32_t time_ms);
uint32_t millis(void);

void Error_Handler(void);



#ifdef __cplusplus
}
#endif

#endif /* SRC_BSP_BSP_H_ */
