/*
 * hw_def.h
 *
 *  Created on: Nov 26, 2021
 *      Author: hs
 */

#ifndef SRC_HW_HW_DEF_H_
#define SRC_HW_HW_DEF_H_


#include "def.h"
#include "bsp.h"


#define _DEF_BOOT_VERSION     "B20220105_R1"
#define _DEF_BOARD_NAME       "Bootloader"

//#define _USE_HW_SD
//#define _USE_HW_FATFS
//#define _USE_HW_QSPI
#define _USE_HW_FLASH
//#define _USE_HW_YMODEM
#define _USE_HW_RESET


#define _USE_HW_RTC
#define      HW_RTC_CFG_DATA_1      RTC_BKP_DR1
#define      HW_RTC_CFG_DATA_2      RTC_BKP_DR2

#define _USE_HW_LED
#define      HW_LED_MAX_CH          2

#define _USE_HW_UART
#define      HW_UART_MAX_CH         3

#define _USE_HW_LOG
#define      HW_LOG_CH              _DEF_UART1
#define      HW_LOG_BOOT_BUF_MAX    1024
#define      HW_LOG_LIST_BUF_MAX    1024

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    16
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    4
#define      HW_CLI_LINE_BUF_MAX    64

#define _USE_HW_BUTTON
#define      HW_BUTTON_MAX_CH       1

//#define _USE_HW_GPIO
//#define      HW_GPIO_MAX_CH         1

#define _USE_HW_USB
#define _USE_HW_CDC
#define      HW_USE_CDC             1
#define      HW_USE_MSC             0

#define _USE_HW_CMD
#define      HW_CMD_MAX_DATA_LENGTH (1024)


#define _HW_DEF_RTC_BOOT_RESET      RTC_BKP_DR3
#define _HW_DEF_RTC_BOOT_MODE       RTC_BKP_DR4
#define _HW_DEF_RTC_RESET_SRC       RTC_BKP_DR5


#define _DEF_UART_LOG_CLI           _DEF_UART1
#define _DEF_UART_CDC               _DEF_UART2
#define _DEF_UART_HW                _DEF_UART3
//#define _DEF_UART_ESP_UART          _DEF_UART4

//#define _PIN_GPIO_SDCARD_DETECT     0


#define FLASH_ADDR_TAG              0x08020000
#define FLASH_ADDR_FW               0x08020400
#define FLASH_ADDR_FW_VER           0x08020800

#define FLASH_ADDR_START            0x08020000
#define FLASH_ADDR_END              (FLASH_ADDR_START + (2048-128)*1024)

#endif /* SRC_HW_HW_DEF_H_ */
