/*
 * button.c
 *
 *  Created on: 2021. 12. 8.
 *      Author: hs
 */


#include "button.h"
#include "cli.h"



#ifdef _USE_HW_BUTTON


typedef struct
{
  GPIO_TypeDef  *port;
  uint32_t       pin;
  uint32_t       mode;
  GPIO_PinState  on_state;
} button_tbl_t;


button_tbl_t button_tbl[BUTTON_MAX_CH] =
    {
        {GPIOA, GPIO_PIN_0, _DEF_INPUT, GPIO_PIN_SET},  // 0 : user
    };


#ifdef _USE_HW_CLI
static void cliButton(cli_args_t *args);
#endif


bool buttonInit(void)
{
  bool ret = true;


  __HAL_RCC_GPIOA_CLK_ENABLE();


  for (int i=0; i<BUTTON_MAX_CH; i++)
  {
    buttonPinMode(i, button_tbl[i].mode);
  }

#ifdef _USE_HW_CLI
  cliAdd("button", cliButton);
#endif

  return ret;
}

bool buttonPinMode(uint8_t ch, uint32_t mode)
{
  bool ret = false;
  GPIO_InitTypeDef GPIO_InitStruct = {0};


  switch(mode)
  {
    case _DEF_INPUT:
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      break;
    case _DEF_INPUT_PULLUP:
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      break;
    case _DEF_INPUT_PULLDOWN:
      GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
      break;
    case _DEF_OUTPUT:
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      break;
    case _DEF_OUTPUT_PULLUP:
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      break;
    case _DEF_OUTPUT_PULLDOWN:
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_PULLDOWN;
      break;

  }

  GPIO_InitStruct.Pin = button_tbl[ch].pin;
  HAL_GPIO_Init(button_tbl[ch].port, &GPIO_InitStruct);

  return ret;
}

bool buttonGetPressed(uint8_t ch)
{
  bool ret = false;

  if (ch >= BUTTON_MAX_CH)
  {
    return false;
  }

  if (HAL_GPIO_ReadPin(button_tbl[ch].port, button_tbl[ch].pin) == button_tbl[ch].on_state)
  {
    ret = true;
  }

  return ret;
}

void buttonObjCreate(button_obj_t *p_obj, uint8_t ch, uint32_t repeat_time)
{
  p_obj->ch = ch;
  p_obj->state = 0;
  p_obj->pre_time = millis();
  p_obj->repeat_time = repeat_time;
}

bool buttonObjGetClicked(button_obj_t *p_obj, uint32_t pressed_time)
{
  bool ret = false;


  switch(p_obj->state)
  {
    case 0:
      if (buttonGetPressed(p_obj->ch) == true)
      {
        p_obj->state = 1;
        p_obj->pre_time = millis();
      }
      break;

    case 1:
      if (buttonGetPressed(p_obj->ch) == true)
      {
        if (millis()-p_obj->pre_time >= pressed_time)
        {
          ret = true; // 버튼 클릭됨
          p_obj->state = 2;
          p_obj->pre_time = millis();
        }
      }
      else
      {
        p_obj->state = 0;
      }
      break;

    case 2:
      if (buttonGetPressed(p_obj->ch) == true)
      {
        if (millis()-p_obj->pre_time >= p_obj->repeat_time)
        {
          p_obj->state = 1;
          p_obj->pre_time = millis();
        }
      }
      else
      {
        p_obj->state = 0;
      }
      break;
  }

  return ret;
}



#ifdef _USE_HW_CLI
void cliButton(cli_args_t *args)
{
  bool ret = false;


  if (args->argc == 2 && args->isStr(0, "show") == true)
  {
    uint8_t ch;

    ch = (uint8_t)args->getData(1);

    while(cliKeepLoop())
    {
      cliPrintf("%d\n", buttonGetPressed(ch));
      delay(500);
    }

    ret = true;
  }

  if (args->argc == 2 && args->isStr(0, "read") == true)
  {
    button_obj_t btn_push;
    uint32_t btn_cnt = 0;
    uint8_t ch;

    ch = (uint8_t)args->getData(1);

    buttonObjCreate(&btn_push, ch, 1000);

    while(cliKeepLoop())
    {
      if (buttonObjGetClicked(&btn_push, 100) == true)
      {
        btn_cnt++;
        cliPrintf("button read %dch : %d\n", ch, btn_cnt);
      }
    }

    ret = true;
  }


  if (ret != true)
  {
    cliPrintf("button show ch[0~%d]\n", BUTTON_MAX_CH-1);
    cliPrintf("button read ch[0~%d]\n", BUTTON_MAX_CH-1);
  }
}
#endif


#endif
