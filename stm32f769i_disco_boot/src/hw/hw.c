/*
 * hw.c
 *
 *  Created on: Nov 26, 2021
 *      Author: hs
 */



#include "hw.h"


const __attribute__((section(".version"))) firm_version_t boot_ver =   // 0x08000400
    {
        _DEF_BOOT_VERSION,
        _DEF_BOARD_NAME
    };


bool hwInit(void)
{
  resetInit();
  cliInit();
  logInit();
  ledInit();
  buttonInit();
  //gpioInit();
  //usbInit();
  uartInit();
  //uartOpen(_DEF_UART_STLINK,  115200);
  uartOpen(_DEF_UART_LOG_CLI, 115200);
  uartOpen(_DEF_UART_CDC,     115200);

  logOpen(_DEF_UART_LOG_CLI, 115200);
  logPrintf("\r\n[ Bootloader Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", boot_ver.name);
  logPrintf("Booting..Ver  \t\t: %s\r\n", boot_ver.version);
  logPrintf("Sys Clk       \t\t: %d Mhz\r\n", HAL_RCC_GetSysClockFreq()/1000000);


  //qspiInit();
  rtcInit();
  flashInit();

  //sdInit();
  //fatfsInit();

  //ymodemInit();


  logBoot(false);

  return true;
}
