/*
 * lcd.cpp
 *
 *  Created on: 2022. 3. 4.
 *      Author: hs
 */




#include "lcd.h"




static const char *thread_name = "lcd         ";
static thread_t *thread = NULL;
static bool is_sdcard = false;


static void lcdThread(void const *argument);
static bool lcdEvent(Event_t event);




bool lcdThreadInit(thread_t *p_thread)
{
  bool ret = false;

  thread = p_thread;

  thread->name = thread_name;
  thread->onEvent = lcdEvent;

  osThreadDef(lcdThread, lcdThread, _HW_DEF_RTOS_THREAD_PRI_LCD, 0, _HW_DEF_RTOS_THREAD_MEM_LCD);
  if (osThreadCreate(osThread(lcdThread), NULL) != NULL)
  {
    ret = true;
    logPrintf("lcdThread  \t\t: OK\r\n");
  }
  else
  {
    logPrintf("lcdThread  \t\t: Fail\r\n");
  }

  p_thread->is_init = ret;

  return ret;
}

void lcdThread(void const *argument)
{
  (void)argument;
  bool is_touch = false;
  bool is_touch_pre = false;

  thread->is_start = true;

  //button_obj_t btn_push;

  //buttonObjCreate(&btn_push, 0, 1000);


  while(1)
  {
    //if (buttonObjGetClicked(&btn_push, 100) == true)buttonGetPressed(ch)
    if (buttonGetPressed(0) == true)
    {
      is_touch = true;
    }
    else
    {
      is_touch = false;
    }
    if (is_touch != is_touch_pre)
    {
      if (is_touch)
      {
        thread->notify(EVENT_TOUCH_PRESSED);
      }
      else
      {
        thread->notify(EVENT_TOUCH_RELEASED);
      }
    }
    is_touch_pre = is_touch;


    delay(5);
    thread->hearbeat++;
  }
}

bool lcdEvent(Event_t event)
{
  bool ret = true;


  switch(event)
  {
    case EVENT_SDCARD_CONNECTED:
      logPrintf("EVENT_SDCARD_CONNECTED : %s,%d\n", __FILE__, __LINE__);
      is_sdcard = true;
      break;

    case EVENT_SDCARD_DISCONNECTED:
      logPrintf("EVENT_SDCARD_DISCONNECTED : %s,%d\n", __FILE__, __LINE__);
      is_sdcard = false;
      break;

    default:
      ret = false;
      break;
  }

  return ret;
}

