/*
 * sdram.h
 *
 *  Created on: 2022. 1. 19.
 *      Author: hs
 */

#ifndef SRC_COMMON_HW_INCLUDE_SDRAM_H_
#define SRC_COMMON_HW_INCLUDE_SDRAM_H_


#ifdef __cplusplus
 extern "C" {
#endif


#include "hw_def.h"

#ifdef _USE_HW_SDRAM



#define SDRAM_OK         ((uint8_t)0x00)
#define SDRAM_ERROR      ((uint8_t)0x01)

#define SDRAM_DEVICE_ADDR  0xC0000000
#define SDRAM_DEVICE_SIZE  (16*1024*1024)  /* SDRAM device size in MBytes */


bool sdramInit(void);
bool sdramIsInit(void);
bool sdramTest(void);

uint32_t sdramGetAddr(void);
uint32_t sdramGetLength(void);

#endif

#ifdef __cplusplus
}
#endif


#endif /* SRC_COMMON_HW_INCLUDE_SDRAM_H_ */
