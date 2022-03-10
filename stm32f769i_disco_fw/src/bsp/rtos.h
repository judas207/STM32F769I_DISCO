/*
 * rtos.h
 *
 *  Created on: 2022. 2. 21.
 *      Author: hs
 */

#ifndef SRC_BSP_RTOS_H_
#define SRC_BSP_RTOS_H_

#ifdef __cplusplus
 extern "C" {
#endif



#include "def.h"

#include "cmsis_os.h"
#include "cpu_utils.h"


void rtosInit(void);



#ifdef __cplusplus
}
#endif

#endif /* SRC_BSP_RTOS_H_ */
