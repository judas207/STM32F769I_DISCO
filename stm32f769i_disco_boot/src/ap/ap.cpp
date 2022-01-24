/*
 * ap.cpp
 *
 *  Created on: Nov 26, 2021
 *      Author: hs
 */



#include "ap.h"
#include "boot/boot.h"



#define MAX_BOOT_CH           2



#define BOOT_MODE_LOADER      0
#define BOOT_MODE_CLI         1
#define BOOT_MODE_JUMP_FW     2


uint8_t boot_mode = BOOT_MODE_LOADER;
static cmd_t cmd_boot[MAX_BOOT_CH];

uint8_t  cli_ch     = _DEF_UART_CLI;
uint32_t cli_baud   = 115200;



void apInit(void)
{
  uint8_t boot_param;


  boot_param = resetGetBootMode();


  // RTC Backup Registers 의 RTC_BKP_DR4 가 set일때 부트모드
  // user 버튼이 눌린 상태로 전원 ON시 부트모드
  // reset 버튼 2번 연속 눌렸을경우 부트모드(사용안함)
  //
  if (boot_param & RESET_MODE_RUN_BOOT || buttonGetPressed(_DEF_BUTTON1) == true)// || resetGetCount() == 2)
  {
    boot_mode = BOOT_MODE_LOADER;
    logPrintf("boot begin...\r\n");

    boot_param &= ~RESET_MODE_RUN_BOOT;
    rtcWriteBackupData(_HW_DEF_RTC_BOOT_MODE, boot_param);

    usbInit();
    usbBegin(USB_CDC_MODE);
    return;
  }

  boot_mode = BOOT_MODE_JUMP_FW;


  logPrintf("jump fw...\r\n");

  if (bootVerifyCrc() != true)
  {
    if (bootVerifyFw() != true)
    {
      logPrintf("fw empty  \t\t: Fail\r\n");
    }
    else
    {
      logPrintf("fw crc    \t\t: Fail\r\n");
    }
    logPrintf("boot begin...\r\n");
    boot_mode = BOOT_MODE_LOADER;
  }
  else
  {
    delay(100);
    bootJumpToFw();
  }

  usbInit();
  usbBegin(USB_CDC_MODE);

}

void apMain(void)
{
  uint32_t pre_time;


  cmdInit(&cmd_boot[0]);
  cmdOpen(&cmd_boot[0], _DEF_UART_CDC, 115200);


  pre_time = millis();
  while(1)
  {
    if (boot_mode == BOOT_MODE_LOADER)
    {
      for (int i=0; i<MAX_BOOT_CH; i++)
      {
        if (cmdReceivePacket(&cmd_boot[i]) == true)
        {
          bootProcessCmd(&cmd_boot[i]);
        }
      }

      if (millis()-pre_time >= 100)
      {
        pre_time = millis();
        //ledToggle(_DEF_LED1);
        ledToggle(_DEF_LED2);
      }
    }
  }
}


#if 0
#define MAX_BOOT_CH           2



#define BOOT_MODE_LOADER      0
#define BOOT_MODE_CLI         1
#define BOOT_MODE_JUMP_FW     2


uint8_t boot_mode = BOOT_MODE_LOADER;
static cmd_t cmd_boot[MAX_BOOT_CH];

uint8_t  cli_ch     = _DEF_UART_CLI;
uint32_t cli_baud   = 115200;

cmd_t cmd;


void apInit(void)
{
  bool run_boot = false;
  uint8_t reg;

  reg = rtcBackupRegRead(4);

  if (reg & (1<<0))
  {
    run_boot = true;
    reg &= ~(1<<0);
    rtcBackupRegWrite(4, reg);
  }


  if (resetGetCount() == 2)
  {
    run_boot = true;
  }

  if (run_boot == false)
  {
    if (buttonGetPressed(_DEF_BUTTON1) == false)
    {
      if (bootVerifyFw() == true && bootVerifyCrc() == true)
      {
        bootJumpToFw();
      }
    }
  }


  // reset 버튼 2번 연속 눌렸을경우 부트모드
  // user 버튼이 눌린 상태로 전원 ON시 부트모드
  //
  usbBegin(USB_CDC_MODE);

  //cliOpen(_DEF_UART2, 115200);

  cmdInit(&cmd);
  cmdOpen(&cmd, _DEF_UART2, 115200);
}

void apMain(void)
{
  uint32_t pre_time;


  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 100)
    {
      pre_time = millis();

      //ledToggle(_DEF_LED1);
      ledToggle(_DEF_LED2);
    }

    //cliMain();

    if (cmdReceivePacket(&cmd) == true)
    {
      bootProcessCmd(&cmd);
    }
  }
}
#endif
