/*
 * uart.c
 *
 *  Created on: 2021. 12. 1.
 *      Author: hs
 */




/*
 *
 * _DEF_UART1
 *      USART6
 *
 * _DEF_UART2
 *      USB CDC
 *
 * _DEF_UART3
 *      USART1
 *
 */



#include "uart.h"
#include "cdc.h"
#include "qbuffer.h"


#ifdef _USE_HW_UART



#define UART_RX_BUF_LENGTH      1024


typedef enum
{
  UART_HW_TYPE_STM32,
  UART_HW_TYPE_USB,
  //UART_HW_TYPE_BT_SPP,
} UartHwType_t;


typedef struct
{
  bool     is_open;
  uint32_t baud;
  UartHwType_t type;


  uint8_t  rx_buf[UART_RX_BUF_LENGTH];
  qbuffer_t qbuffer;
  UART_HandleTypeDef *p_huart;
  DMA_HandleTypeDef  *p_hdma_rx;

} uart_tbl_t;


//static __attribute__((section(".non_cache"))) uart_tbl_t uart_tbl[UART_MAX_CH];
uart_tbl_t uart_tbl[UART_MAX_CH];



UART_HandleTypeDef huart1;
UART_HandleTypeDef huart6;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart6_rx;



bool uartInit(void)
{
  for (int i=0; i<UART_MAX_CH; i++)
  {
    uart_tbl[i].is_open = false;
    uart_tbl[i].baud = 115200;
  }

  return true;
}

bool uartOpen(uint8_t ch, uint32_t baud)
{
  bool ret = false;


  if (uart_tbl[ch].is_open == true)
  {
    if (uart_tbl[ch].baud == baud)
    {
      return true;
    }
  }

  switch(ch)
  {
    case _DEF_UART1:
    case _DEF_UART3:
    case _DEF_UART4:
      uart_tbl[ch].type      = UART_HW_TYPE_STM32;
      uart_tbl[ch].baud      = baud;

      if (ch == _DEF_UART1)
      {
        uart_tbl[ch].p_huart   = &huart6;
        uart_tbl[ch].p_hdma_rx = &hdma_usart6_rx;
        uart_tbl[ch].p_huart->Instance    = USART6;
      }
      if (ch == _DEF_UART3)
      {
        uart_tbl[ch].p_huart   = &huart1;
        uart_tbl[ch].p_hdma_rx = &hdma_usart1_rx;
        uart_tbl[ch].p_huart->Instance    = USART1;
      }
      if (ch == _DEF_UART4)
      {
        //uart_tbl[ch].p_huart   = &huart7;
        //uart_tbl[ch].p_hdma_rx = &hdma_uart7_rx;
        //uart_tbl[ch].p_huart->Instance    = UART7;
      }

      uart_tbl[ch].p_huart->Init.BaudRate    = baud;
      uart_tbl[ch].p_huart->Init.WordLength  = UART_WORDLENGTH_8B;
      uart_tbl[ch].p_huart->Init.StopBits    = UART_STOPBITS_1;
      uart_tbl[ch].p_huart->Init.Parity      = UART_PARITY_NONE;
      uart_tbl[ch].p_huart->Init.Mode        = UART_MODE_TX_RX;
      uart_tbl[ch].p_huart->Init.HwFlowCtl   = UART_HWCONTROL_NONE;
      uart_tbl[ch].p_huart->Init.OverSampling= UART_OVERSAMPLING_16;
      uart_tbl[ch].p_huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
      uart_tbl[ch].p_huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;


      HAL_UART_DeInit(uart_tbl[ch].p_huart);

      qbufferCreate(&uart_tbl[ch].qbuffer, &uart_tbl[ch].rx_buf[0], UART_RX_BUF_LENGTH);

      if (HAL_UART_Init(uart_tbl[ch].p_huart) != HAL_OK)
      {
        ret = false;
      }
      else
      {
        ret = true;
        uart_tbl[ch].is_open = true;

        if(HAL_UART_Receive_DMA(uart_tbl[ch].p_huart, (uint8_t *)&uart_tbl[ch].rx_buf[0], UART_RX_BUF_LENGTH) != HAL_OK)
        {
          ret = false;
        }

        uart_tbl[ch].qbuffer.in  = uart_tbl[ch].qbuffer.len - ((DMA_Stream_TypeDef *)uart_tbl[ch].p_huart->hdmarx->Instance)->NDTR;
        uart_tbl[ch].qbuffer.out = uart_tbl[ch].qbuffer.in;
      }
      break;

    case _DEF_UART2:
      uart_tbl[ch].type    = UART_HW_TYPE_USB;
      uart_tbl[ch].baud    = baud;

      if (cdcIsInit() == true)
      {
        uart_tbl[ch].is_open = true;
      }
      ret = true;
      break;
  }

  return ret;
}

bool uartClose(uint8_t ch)
{
  uart_tbl[ch].is_open = false;
  return true;
}

uint32_t uartAvailable(uint8_t ch)
{
  uint32_t ret = 0;

  if (uart_tbl[ch].is_open != true) return 0;


  switch(ch)
  {
    case _DEF_UART1:
    case _DEF_UART3:
    case _DEF_UART4:
      uart_tbl[ch].qbuffer.in = (uart_tbl[ch].qbuffer.len - ((DMA_Stream_TypeDef *)uart_tbl[ch].p_hdma_rx->Instance)->NDTR);
      ret = qbufferAvailable(&uart_tbl[ch].qbuffer);
      break;

    case _DEF_UART2:
      ret = cdcAvailable();
      break;
  }

  return ret;
}

bool uartFlush(uint8_t ch)
{
  uint32_t pre_time;

  pre_time = millis();
  while(uartAvailable(ch))
  {
    if (millis()-pre_time >= 10)
    {
      break;
    }
    uartRead(ch);
  }

  return true;
}

uint8_t uartRead(uint8_t ch)
{
  uint8_t ret = 0;

  switch(ch)
  {
    case _DEF_UART1:
    case _DEF_UART3:
    case _DEF_UART4:
      qbufferRead(&uart_tbl[ch].qbuffer, &ret, 1);
      break;

    case _DEF_UART2:
      ret = cdcRead();
      break;
  }

  return ret;
}

uint32_t uartWrite(uint8_t ch, uint8_t *p_data, uint32_t length)
{
  uint32_t ret = 0;

  switch(ch)
  {
    case _DEF_UART1:
    case _DEF_UART3:
    case _DEF_UART4:
      if (HAL_UART_Transmit(uart_tbl[ch].p_huart, p_data, length, 100) == HAL_OK)
      {
        ret = length;
      }
      break;

    case _DEF_UART2:
      ret = cdcWrite(p_data, length);
      break;
  }

  return ret;
}

uint32_t uartPrintf(uint8_t ch, const char *fmt, ...)
{
  char buf[256];
  va_list args;
  int len;
  uint32_t ret;

  va_start(args, fmt);
  len = vsnprintf(buf, 256, fmt, args);

  ret = uartWrite(ch, (uint8_t *)buf, len);

  va_end(args);


  return ret;
}

uint32_t uartGetBaud(uint8_t ch)
{
  uint32_t ret = 0;

  if (uart_tbl[ch].type == UART_HW_TYPE_USB)
  {
    ret = cdcGetBaud();
  }
  else
  {
    ret = uart_tbl[ch].baud;
  }

  return ret;
}




void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART1)
  {
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#if 0
  if (huart->Instance == USART1)
  {
    qbufferWrite(&qbuffer[_DEF_UART2], &rx_data[_DEF_UART2], 1);

    HAL_UART_Receive_IT(&huart1, (uint8_t *)&rx_data[_DEF_UART2], 1);
  }
#endif
}


void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};


  if(uartHandle->Instance==USART1)
  {
    __HAL_RCC_DMA2_CLK_ENABLE();

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /**USART1 GPIO Configuration
    PA10     ------> USART1_RX
    PA9      ------> USART1_TX
    */
    GPIO_InitStruct.Pin       = GPIO_PIN_10|GPIO_PIN_9;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    /* USART1_RX Init */
    hdma_usart1_rx.Instance                 = DMA2_Stream2;
    hdma_usart1_rx.Init.Channel             = DMA_CHANNEL_4;
    hdma_usart1_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode                = DMA_NORMAL;
    hdma_usart1_rx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_usart1_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

    /* DMA2_Stream2_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  }
  else if(uartHandle->Instance==USART6)
  {
    __HAL_RCC_DMA2_CLK_ENABLE();

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART6;
    PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_USART6_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /**USART6 GPIO Configuration
    PC7     ------> USART6_RX
    PC6     ------> USART6_TX
    */
    GPIO_InitStruct.Pin       = GPIO_PIN_7|GPIO_PIN_6;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* USART6 DMA Init */
    /* USART6_RX Init */
    hdma_usart6_rx.Instance                 = DMA2_Stream1;
    hdma_usart6_rx.Init.Channel             = DMA_CHANNEL_5;
    hdma_usart6_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_usart6_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_usart6_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_usart6_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart6_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_usart6_rx.Init.Mode                = DMA_CIRCULAR;
    hdma_usart6_rx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_usart6_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_usart6_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart6_rx);

    /* DMA2_Stream1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
  if(uartHandle->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10|GPIO_PIN_9);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
  }
  else if(uartHandle->Instance==USART6)
  {
    __HAL_RCC_USART6_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_7|GPIO_PIN_6);

    /* USART6 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
  }
}



#endif
