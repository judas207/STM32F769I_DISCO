/*
 * button.h
 *
 *  Created on: 2021. 12. 8.
 *      Author: hs
 */

#ifndef SRC_COMMON_HW_INCLUDE_BUTTON_H_
#define SRC_COMMON_HW_INCLUDE_BUTTON_H_

#ifdef __cplusplus
 extern "C" {
#endif



#include "hw_def.h"


#ifdef _USE_HW_BUTTON


#define BUTTON_MAX_CH       HW_BUTTON_MAX_CH


typedef struct
{
  uint8_t  ch;
  uint8_t  state;
  uint32_t repeat_time;
  uint32_t pre_time;
} button_obj_t;


bool buttonInit(void);
bool buttonPinMode(uint8_t ch, uint32_t mode);
bool buttonGetPressed(uint8_t ch);

void buttonObjCreate(button_obj_t *p_obj, uint8_t ch, uint32_t repeat_time);
bool buttonObjGetClicked(button_obj_t *p_obj, uint32_t pressed_time);


#endif



#ifdef __cplusplus
}
#endif

#endif /* SRC_COMMON_HW_INCLUDE_BUTTON_H_ */
