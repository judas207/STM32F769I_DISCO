/*
 * fatfs.h
 *
 *  Created on: 2021. 12. 13.
 *      Author: hs
 */

#ifndef SRC_COMMON_HW_INCLUDE_FATFS_H_
#define SRC_COMMON_HW_INCLUDE_FATFS_H_

#ifdef __cplusplus
 extern "C" {
#endif



#include "hw_def.h"


#ifdef _USE_HW_FATFS


bool fatfsInit(void);


#endif



#ifdef __cplusplus
}
#endif

#endif /* SRC_COMMON_HW_INCLUDE_FATFS_H_ */
