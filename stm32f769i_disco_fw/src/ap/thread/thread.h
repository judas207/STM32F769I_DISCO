/*
 * thread.h
 *
 *  Created on: 2022. 3. 3.
 *      Author: hs
 */

#ifndef SRC_AP_THREAD_THREAD_H_
#define SRC_AP_THREAD_THREAD_H_

#ifdef __cplusplus
 extern "C" {
#endif



#include "ap_def.h"



bool threadInit(void);
bool threadNotify(Event_t event);



#ifdef __cplusplus
}
#endif

#endif /* SRC_AP_THREAD_THREAD_H_ */
