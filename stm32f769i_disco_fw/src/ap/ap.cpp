/*
 * ap.cpp
 *
 *  Created on: Nov 26, 2021
 *      Author: hs
 */





#include "ap.h"



void cliBoot(cli_args_t *args);


void apInit(void)
{
#if 0
  //if (sdIsDetected() == true)
  //{
    usbBegin(USB_MSC_MODE);
  //}
#else
  usbBegin(USB_CDC_MODE);  // for auto boot_loader
#endif


  cliOpen(_DEF_UART_LOG_CLI, 115200);
  cliAdd("boot", cliBoot);

}

void apMain(void)
{
  uint32_t pre_time;


  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 500)
    {
      pre_time = millis();

      ledToggle(_DEF_LED1);
      ledToggle(_DEF_LED2);
    }

#if  0
    if (uartAvailable(_DEF_UART2) > 0)
    {
      uint8_t rx_data;

      rx_data = uartRead(_DEF_UART2);

      uartPrintf(_DEF_UART2, "usb rx : 0x%x(%c)\n", rx_data, rx_data);
    }
#endif

    cliMain();
  }
}


void cliBoot(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 1 && args->isStr(0, "info") == true)
  {
    firm_version_t *p_boot_ver = (firm_version_t *)(FLASH_ADDR_BOOT_VER);


    cliPrintf("boot ver   : %s\n", p_boot_ver->version);
    cliPrintf("boot name  : %s\n", p_boot_ver->name);
    cliPrintf("boot param : 0x%X\n", rtcBackupRegRead(0));

    cliPrintf("PCLK2 : %d\n",HAL_RCC_GetPCLK2Freq());

    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "jump_boot") == true)
  {
    rtcBackupRegWrite(0, 1);
    resetToBoot(0);
    ret = true;
  }

  if (args->argc == 1 && args->isStr(0, "jump_fw") == true)
  {
    //rtcBackupRegWrite(0, 0);
    ret = true;
  }

  if (ret != true)
  {
    cliPrintf("boot info\n");
    cliPrintf("boot jump_boot\n");
    cliPrintf("boot jump_fw\n");
  }
}
