/**
  ******************************************************************************
  * @file    sdmmc.c
  * @brief   This file provides code for the configuration
  *          of the SDMMC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sdmmc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SD_HandleTypeDef hsd2;
DMA_HandleTypeDef hdma_sdmmc2_rx;
DMA_HandleTypeDef hdma_sdmmc2_tx;

/* SDMMC2 init function */

void MX_SDMMC2_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC2_Init 0 */

  /* USER CODE END SDMMC2_Init 0 */

  /* USER CODE BEGIN SDMMC2_Init 1 */

  /* USER CODE END SDMMC2_Init 1 */
  hsd2.Instance = SDMMC2;
  hsd2.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd2.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd2.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd2.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd2.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd2.Init.ClockDiv = 0;
  if (HAL_SD_Init(&hsd2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_SD_ConfigWideBusOperation(&hsd2, SDMMC_BUS_WIDE_4B) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SDMMC2_Init 2 */

  /* USER CODE END SDMMC2_Init 2 */

}

void HAL_SD_MspInit(SD_HandleTypeDef* sdHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(sdHandle->Instance==SDMMC2)
  {
  /* USER CODE BEGIN SDMMC2_MspInit 0 */
    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();
  /* USER CODE END SDMMC2_MspInit 0 */
    /* SDMMC2 clock enable */
    __HAL_RCC_SDMMC2_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    /**SDMMC2 GPIO Configuration
    PB4     ------> SDMMC2_D3
    PB3     ------> SDMMC2_D2
    PD7     ------> SDMMC2_CMD
    PD6     ------> SDMMC2_CK
    PG10     ------> SDMMC2_D1
    PG9     ------> SDMMC2_D0
    */
    GPIO_InitStruct.Pin = uSD_D3_Pin|uSD_D2_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_SDMMC2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = uSD_CMD_Pin|uSD_CLK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_SDMMC2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = uSD_D1_Pin|uSD_D0_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_SDMMC2;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* SDMMC2 DMA Init */
    /* SDMMC2_RX Init */
    hdma_sdmmc2_rx.Instance = DMA2_Stream0;
    hdma_sdmmc2_rx.Init.Channel = DMA_CHANNEL_11;
    hdma_sdmmc2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_sdmmc2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdmmc2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdmmc2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdmmc2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdmmc2_rx.Init.Mode = DMA_PFCTRL;
    hdma_sdmmc2_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_sdmmc2_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdmmc2_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdmmc2_rx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdmmc2_rx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&hdma_sdmmc2_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(sdHandle,hdmarx,hdma_sdmmc2_rx);

    /* SDMMC2_TX Init */
    hdma_sdmmc2_tx.Instance = DMA2_Stream5;
    hdma_sdmmc2_tx.Init.Channel = DMA_CHANNEL_11;
    hdma_sdmmc2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_sdmmc2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_sdmmc2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_sdmmc2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_sdmmc2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_sdmmc2_tx.Init.Mode = DMA_PFCTRL;
    hdma_sdmmc2_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_sdmmc2_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    hdma_sdmmc2_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_sdmmc2_tx.Init.MemBurst = DMA_MBURST_INC4;
    hdma_sdmmc2_tx.Init.PeriphBurst = DMA_PBURST_INC4;
    if (HAL_DMA_Init(&hdma_sdmmc2_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(sdHandle,hdmatx,hdma_sdmmc2_tx);

    /* SDMMC2 interrupt Init */
    HAL_NVIC_SetPriority(SDMMC2_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(SDMMC2_IRQn);
  /* USER CODE BEGIN SDMMC2_MspInit 1 */
    /* DMA interrupt init */
    /* DMA2_Stream0_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    /* DMA2_Stream5_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
  /* USER CODE END SDMMC2_MspInit 1 */
  }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef* sdHandle)
{

  if(sdHandle->Instance==SDMMC2)
  {
  /* USER CODE BEGIN SDMMC2_MspDeInit 0 */

  /* USER CODE END SDMMC2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SDMMC2_CLK_DISABLE();

    /**SDMMC2 GPIO Configuration
    PB4     ------> SDMMC2_D3
    PB3     ------> SDMMC2_D2
    PD7     ------> SDMMC2_CMD
    PD6     ------> SDMMC2_CK
    PG10     ------> SDMMC2_D1
    PG9     ------> SDMMC2_D0
    */
    HAL_GPIO_DeInit(GPIOB, uSD_D3_Pin|uSD_D2_Pin);

    HAL_GPIO_DeInit(GPIOD, uSD_CMD_Pin|uSD_CLK_Pin);

    HAL_GPIO_DeInit(GPIOG, uSD_D1_Pin|uSD_D0_Pin);

    /* SDMMC2 DMA DeInit */
    HAL_DMA_DeInit(sdHandle->hdmarx);
    HAL_DMA_DeInit(sdHandle->hdmatx);

    /* SDMMC2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(SDMMC2_IRQn);
  /* USER CODE BEGIN SDMMC2_MspDeInit 1 */

  /* USER CODE END SDMMC2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
