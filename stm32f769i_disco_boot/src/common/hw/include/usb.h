/*
 * usb.h
 *
 *  Created on: 2021. 12. 14.
 *      Author: hs
 */

#ifndef SRC_COMMON_HW_INCLUDE_USB_H_
#define SRC_COMMON_HW_INCLUDE_USB_H_
#ifdef __cplusplus
extern "C" {
#endif



#include "hw_def.h"


#ifdef _USE_HW_USB


typedef enum UsbMode
{
  USB_NON_MODE,
  USB_CDC_MODE,
  USB_MSC_MODE
} usb_mode_t;

typedef enum UsbType
{
  USB_CON_CDC = 0,
  USB_CON_CAN = 1,
  USB_CON_CLI = 2,
  USB_CON_ESP = 3,
} usb_type_t;

bool usbInit(void);
bool usbBegin(usb_mode_t usb_mode);
void usbDeInit(void);
bool usbIsOpen(void);
bool usbIsConnect(void);

usb_mode_t usbGetMode(void);
usb_type_t usbGetType(void);

#endif



#ifdef __cplusplus
}
#endif
#endif /* SRC_COMMON_HW_INCLUDE_USB_H_ */
