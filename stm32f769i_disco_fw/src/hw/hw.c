/*
 * hw.c
 *
 *  Created on: Nov 26, 2021
 *      Author: hs
 */



#include "hw.h"



extern uint32_t _flash_tag_addr;
extern uint32_t __isr_vector_addr;


__attribute__((section(".tag"))) firm_tag_t firm_tag =
    {
        .magic_number = 0xAAAA5555,

        //-- fw info
        //
        .addr_tag = (uint32_t)&_flash_tag_addr,
        .addr_fw  = (uint32_t)&__isr_vector_addr,
        .size_tag = 1024,


        //-- tag info
        //
    };


__attribute__((section(".version"))) firm_version_t firm_ver =
    {
        _DEF_FIRMWATRE_VERSION,
        _DEF_BOARD_NAME
    };


bool hwInit(void)
{
  resetInit();
  cliInit();
  logInit();
  ledInit();
  buttonInit();
  gpioInit();
  usbInit();
  uartInit();
  uartOpen(_DEF_UART_CLI, 115200);  // UART6
  uartOpen(_DEF_UART_CDC, 115200);  // USB_CDC

  logOpen(_DEF_UART1, 115200);
  logPrintf("\r\n[ Firmware Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", firm_ver.name);
  logPrintf("Booting..Ver  \t\t: %s\r\n", firm_ver.version);
  logPrintf("Sys Clk       \t\t: %d Mhz\r\n", HAL_RCC_GetSysClockFreq()/1000000);

  rtcInit();

  sdramInit();
  qspiInit();

  flashInit();

  sdInit();
  fatfsInit();



  logBoot(false);

  //cliOpen(_DEF_UART_CLI, 115200);

  return true;
}
