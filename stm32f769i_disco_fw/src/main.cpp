/*
 * main.cpp
 *
 *  Created on: Nov 26, 2021
 *      Author: hs
 */




#include "main.h"




static void mainThread(void const *argument);


int main(void)
{
  bspInit();


  osThreadDef(mainThread, mainThread, _HW_DEF_RTOS_THREAD_PRI_MAIN, 0, _HW_DEF_RTOS_THREAD_MEM_MAIN);
  if (osThreadCreate(osThread(mainThread), NULL) == NULL)
  {
    logPrintf("threadMain \t\t: Fail\r\n");
    ledInit();

    while(1)
    {
      ledOn(_DEF_LED1);
      ledOff(_DEF_LED2);
      delay(100);
      ledOff(_DEF_LED1);
      ledOn(_DEF_LED2);
      delay(100);
    }
  }

  osKernelStart();

  return 0;
}

void mainThread(void const *argument)
{
  UNUSED(argument);

  hwInit();
  apInit();
  apMain();
}

#if 0
int main(void)
{
  bspInit();

  hwInit();
  apInit();

  apMain();

  return 0;
}
#endif

