/*
 * sdram.c
 *
 *  Created on: 2022. 1. 19.
 *      Author: hs
 */





#include "sdram.h"
#include "cli.h"



#ifdef _USE_HW_SDRAM


#define SDRAM_OK         ((uint8_t)0x00)
#define SDRAM_ERROR      ((uint8_t)0x01)

#define SDRAM_DEVICE_ADDR  SDRAM_MEM_ADDR
#define SDRAM_DEVICE_SIZE  SDRAM_MEM_SIZE




uint8_t BSP_SDRAM_Init(void);
uint8_t BSP_SDRAM_DeInit(void);
void    BSP_SDRAM_Initialization_sequence(uint32_t RefreshCount);
uint8_t BSP_SDRAM_ReadData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);
uint8_t BSP_SDRAM_ReadData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);
uint8_t BSP_SDRAM_WriteData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);
uint8_t BSP_SDRAM_WriteData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);
uint8_t BSP_SDRAM_Sendcmd(FMC_SDRAM_CommandTypeDef *SdramCmd);

/* These functions can be modified in case the current settings (e.g. DMA stream)
   need to be changed for specific application needs */
void    BSP_SDRAM_MspInit(SDRAM_HandleTypeDef  *hsdram, void *Params);
void    BSP_SDRAM_MspDeInit(SDRAM_HandleTypeDef  *hsdram, void *Params);


static bool is_init = false;


#ifdef _USE_HW_CLI
void cliSdram(cli_args_t *args);
#endif



bool sdramInit(void)
{
  bool ret = true;;

  if (BSP_SDRAM_Init() != 0x00)
  {
    ret = false;
  }

  if (ret == true)
  {
    logPrintf("SDRAM %dMB \t\t: OK\r\n", (int)(SDRAM_DEVICE_SIZE/1024/1024));
  }
  else
  {
    logPrintf("SDRAM  \t\t: Fail\r\n");
  }

#ifdef _USE_HW_CLI
  cliAdd("sdram", cliSdram);
#endif

  is_init = ret;

  return ret;
}

bool sdramIsInit(void)
{
  return is_init;
}

uint32_t sdramGetAddr(void)
{
  return SDRAM_DEVICE_ADDR;
}

uint32_t sdramGetLength(void)
{
  return SDRAM_DEVICE_SIZE;
}

bool sdramTest(void)
{
  uint32_t *p_data = (uint32_t *)SDRAM_DEVICE_ADDR;
  uint32_t i;


  for (i=0; i<SDRAM_DEVICE_SIZE/4; i++)
  {
    p_data[i] = i;
  }

  for (i=0; i<SDRAM_DEVICE_SIZE/4; i++)
  {
    if (p_data[i] != i)
    {
      return false;
    }
  }

  for (i=0; i<SDRAM_DEVICE_SIZE/4; i++)
  {
    p_data[i] = 0x5555AAAA;
  }
  for (i=0; i<SDRAM_DEVICE_SIZE/4; i++)
  {
    if (p_data[i] != 0x5555AAAA)
    {
      return false;
    }
  }

  for (i=0; i<SDRAM_DEVICE_SIZE/4; i++)
  {
    p_data[i] = 0xAAAA5555;
  }
  for (i=0; i<SDRAM_DEVICE_SIZE/4; i++)
  {
    if (p_data[i] != 0xAAAA5555)
    {
      return false;
    }
  }

  return true;
}




/* #define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_8  */
/* #define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_16 */
#define SDRAM_MEMORY_WIDTH            FMC_SDRAM_MEM_BUS_WIDTH_32

#define SDCLOCK_PERIOD                   FMC_SDRAM_CLOCK_PERIOD_2
/* #define SDCLOCK_PERIOD                   FMC_SDRAM_CLOCK_PERIOD_3 */

#define REFRESH_COUNT                    ((uint32_t)0x0603)   /* SDRAM refresh counter (100Mhz SD clock) */

#define SDRAM_TIMEOUT                    ((uint32_t)0xFFFF)

#if 0
/* DMA definitions for SDRAM DMA transfer */
#define __DMAx_CLK_ENABLE                 __HAL_RCC_DMA2_CLK_ENABLE
#define __DMAx_CLK_DISABLE                __HAL_RCC_DMA2_CLK_DISABLE
#define SDRAM_DMAx_CHANNEL                DMA_CHANNEL_0
#define SDRAM_DMAx_STREAM                 DMA2_Stream0
#define SDRAM_DMAx_IRQn                   DMA2_Stream0_IRQn
#define BSP_SDRAM_DMA_IRQHandler          DMA2_Stream0_IRQHandler
#endif


/**
  * @brief  FMC SDRAM Mode definition register defines
  */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)




SDRAM_HandleTypeDef sdramHandle;
static FMC_SDRAM_TimingTypeDef Timing;
static FMC_SDRAM_CommandTypeDef Command;



uint8_t BSP_SDRAM_Init(void)
{
  static uint8_t sdramstatus = SDRAM_ERROR;
  /* SDRAM device configuration */
  sdramHandle.Instance = FMC_SDRAM_DEVICE;

  /* Timing configuration for 100Mhz(t=1/f 10ns) as SDRAM clock frequency (System clock is up to 200Mhz) */
  Timing.LoadToActiveDelay    = 2;  // tMRD  2*tCK
  Timing.ExitSelfRefreshDelay = 7;  // tXSR  70ns / 10ns = 7ns
  Timing.SelfRefreshTime      = 4;  // tRAS  42ns / 10ns = 4.2ns
  Timing.RowCycleDelay        = 7;  // tRC   70ns / 10ns = 7ns
  Timing.WriteRecoveryTime    = 2;  // tWR   14ns / 10ns = 1.4ns ??
  Timing.RPDelay              = 2;  // tRP   20ns / 10ns = 2ns
  Timing.RCDDelay             = 3;  // tCD   20ns / 10ns = 2ns ??


  sdramHandle.Init.SDBank             = FMC_SDRAM_BANK1;
  sdramHandle.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
  sdramHandle.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
  sdramHandle.Init.MemoryDataWidth    = SDRAM_MEMORY_WIDTH;
  sdramHandle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  sdramHandle.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
  sdramHandle.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  sdramHandle.Init.SDClockPeriod      = SDCLOCK_PERIOD;
  sdramHandle.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
  sdramHandle.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;

  /* SDRAM controller initialization */

  BSP_SDRAM_MspInit(&sdramHandle, NULL); /* __weak function can be rewritten by the application */

  if(HAL_SDRAM_Init(&sdramHandle, &Timing) != HAL_OK)
  {
    sdramstatus = SDRAM_ERROR;
  }
  else
  {
    sdramstatus = SDRAM_OK;
  }

  /* SDRAM initialization sequence */
  BSP_SDRAM_Initialization_sequence(REFRESH_COUNT);

  return sdramstatus;
}

/**
  * @brief  DeInitializes the SDRAM device.
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_DeInit(void)
{
  static uint8_t sdramstatus = SDRAM_ERROR;
  /* SDRAM device de-initialization */
  sdramHandle.Instance = FMC_SDRAM_DEVICE;

  if(HAL_SDRAM_DeInit(&sdramHandle) != HAL_OK)
  {
    sdramstatus = SDRAM_ERROR;
  }
  else
  {
    sdramstatus = SDRAM_OK;
  }

  /* SDRAM controller de-initialization */
  BSP_SDRAM_MspDeInit(&sdramHandle, NULL);

  return sdramstatus;
}

/**
  * @brief  Programs the SDRAM device.
  * @param  RefreshCount: SDRAM refresh counter value
  * @retval None
  */
void BSP_SDRAM_Initialization_sequence(uint32_t RefreshCount)
{
  __IO uint32_t tmpmrd = 0;

  /* Step 1: Configure a clock configuration enable command */
  Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

  /* Step 2: Insert 100 us minimum delay */
  /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
  HAL_Delay(1);

  /* Step 3: Configure a PALL (precharge all) command */
  Command.CommandMode            = FMC_SDRAM_CMD_PALL;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

  /* Step 4: Configure an Auto Refresh command */
  Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 8;
  Command.ModeRegisterDefinition = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

  /* Step 5: Program the external memory mode register */
  tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |\
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |\
                     SDRAM_MODEREG_CAS_LATENCY_3           |\
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |\
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
  Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber      = 1;
  Command.ModeRegisterDefinition = tmpmrd;

  /* Send the command */
  HAL_SDRAM_SendCommand(&sdramHandle, &Command, SDRAM_TIMEOUT);

  /* Step 6: Set the refresh rate counter */
  /* Set the device refresh rate */
  HAL_SDRAM_ProgramRefreshRate(&sdramHandle, RefreshCount);
}


/**
  * @brief  Reads an amount of data from the SDRAM memory in polling mode.
  * @param  uwStartAddress: Read start address
  * @param  pData: Pointer to data to be read
  * @param  uwDataSize: Size of read data from the memory
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_ReadData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize)
{
  if(HAL_SDRAM_Read_32b(&sdramHandle, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    return SDRAM_ERROR;
  }
  else
  {
    return SDRAM_OK;
  }
}

/**
  * @brief  Reads an amount of data from the SDRAM memory in DMA mode.
  * @param  uwStartAddress: Read start address
  * @param  pData: Pointer to data to be read
  * @param  uwDataSize: Size of read data from the memory
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_ReadData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize)
{
  if(HAL_SDRAM_Read_DMA(&sdramHandle, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    return SDRAM_ERROR;
  }
  else
  {
    return SDRAM_OK;
  }
}

/**
  * @brief  Writes an amount of data to the SDRAM memory in polling mode.
  * @param  uwStartAddress: Write start address
  * @param  pData: Pointer to data to be written
  * @param  uwDataSize: Size of written data from the memory
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_WriteData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize)
{
  if(HAL_SDRAM_Write_32b(&sdramHandle, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    return SDRAM_ERROR;
  }
  else
  {
    return SDRAM_OK;
  }
}

/**
  * @brief  Writes an amount of data to the SDRAM memory in DMA mode.
  * @param  uwStartAddress: Write start address
  * @param  pData: Pointer to data to be written
  * @param  uwDataSize: Size of written data from the memory
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_WriteData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize)
{
  if(HAL_SDRAM_Write_DMA(&sdramHandle, (uint32_t *)uwStartAddress, pData, uwDataSize) != HAL_OK)
  {
    return SDRAM_ERROR;
  }
  else
  {
    return SDRAM_OK;
  }
}

/**
  * @brief  Sends command to the SDRAM bank.
  * @param  SdramCmd: Pointer to SDRAM command structure
  * @retval SDRAM status
  */
uint8_t BSP_SDRAM_Sendcmd(FMC_SDRAM_CommandTypeDef *SdramCmd)
{
  if(HAL_SDRAM_SendCommand(&sdramHandle, SdramCmd, SDRAM_TIMEOUT) != HAL_OK)
  {
    return SDRAM_ERROR;
  }
  else
  {
    return SDRAM_OK;
  }
}

/**
  * @brief  Initializes SDRAM MSP.
  * @param  hsdram SDRAM handle
  * @param  Params User parameters
  * @retval None
  */
__weak void BSP_SDRAM_MspInit(SDRAM_HandleTypeDef  *hsdram, void *Params)
{
  //static DMA_HandleTypeDef dma_handle;
  GPIO_InitTypeDef gpio_init_structure;

  /* Enable FMC clock */
  __HAL_RCC_FMC_CLK_ENABLE();

  /* Enable chosen DMAx clock */
  //__DMAx_CLK_ENABLE();

  /* Enable GPIOs clock */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();


  /** FMC GPIO Configuration
  PE1   ------> FMC_NBL1
  PE0   ------> FMC_NBL0
  PG15  ------> FMC_SDNCAS
  PD0   ------> FMC_D2
  PI4   ------> FMC_NBL2
  PD1   ------> FMC_D3
  PI3   ------> FMC_D27
  PI2   ------> FMC_D26
  PF0   ------> FMC_A0
  PI5   ------> FMC_NBL3
  PI7   ------> FMC_D29
  PI10  ------> FMC_D31
  PI6   ------> FMC_D28
  PH15  ------> FMC_D23
  PI1   ------> FMC_D25
  PF1   ------> FMC_A1
  PI9   ------> FMC_D30
  PH13  ------> FMC_D21
  PH14  ------> FMC_D22
  PI0   ------> FMC_D24
  PF2   ------> FMC_A2
  PF3   ------> FMC_A3
  PG8   ------> FMC_SDCLK
  PF4   ------> FMC_A4
  PH5   ------> FMC_SDNWE
  PH3   ------> FMC_SDNE0
  PF5   ------> FMC_A5
  PH2   ------> FMC_SDCKE0
  PD15  ------> FMC_D1
  PD10  ------> FMC_D15
  PD14  ------> FMC_D0
  PD9   ------> FMC_D14
  PD8   ------> FMC_D13
  PF12  ------> FMC_A6
  PG1   ------> FMC_A11
  PF15  ------> FMC_A9
  PG2   ------> FMC_A12
  PH12  ------> FMC_D20
  PF13  ------> FMC_A7
  PG0   ------> FMC_A10
  PE8   ------> FMC_D5
  PG5   ------> FMC_BA1
  PG4   ------> FMC_BA0
  PH9   ------> FMC_D17
  PH11  ------> FMC_D19
  PF14  ------> FMC_A8
  PF11  ------> FMC_SDNRAS
  PE9   ------> FMC_D6
  PE11  ------> FMC_D8
  PE14  ------> FMC_D11
  PH8   ------> FMC_D16
  PH10  ------> FMC_D18
  PE7   ------> FMC_D4
  PE10  ------> FMC_D7
  PE12  ------> FMC_D9
  PE15  ------> FMC_D12
  PE13  ------> FMC_D10
  */
  /* GPIO_InitStruct */
  /* Common GPIO configuration */
  gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull      = GPIO_PULLUP;
  gpio_init_structure.Speed     = GPIO_SPEED_HIGH;
  gpio_init_structure.Alternate = GPIO_AF12_FMC;

  /* GPIOD configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 |
                              GPIO_PIN_14 | GPIO_PIN_15;


  HAL_GPIO_Init(GPIOD, &gpio_init_structure);

  /* GPIOE configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_7  | GPIO_PIN_8  | GPIO_PIN_9  |
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                              GPIO_PIN_15;

  HAL_GPIO_Init(GPIOE, &gpio_init_structure);

  /* GPIOF configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3  | GPIO_PIN_4  |
                              GPIO_PIN_5  | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                              GPIO_PIN_15;

  HAL_GPIO_Init(GPIOF, &gpio_init_structure);

  /* GPIOG configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2   | GPIO_PIN_4 |
                              GPIO_PIN_5  | GPIO_PIN_8  | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOG, &gpio_init_structure);

  /* GPIOH configuration */
  gpio_init_structure.Pin   = GPIO_PIN_2  | GPIO_PIN_3  | GPIO_PIN_5  | GPIO_PIN_8  | GPIO_PIN_9  |
                              GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                              GPIO_PIN_15;
  HAL_GPIO_Init(GPIOH, &gpio_init_structure);

  /* GPIOI configuration */
  gpio_init_structure.Pin   = GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3  | GPIO_PIN_4  |
                              GPIO_PIN_5  | GPIO_PIN_6  | GPIO_PIN_7  | GPIO_PIN_9  | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOI, &gpio_init_structure);

#if 0
  /* Configure common DMA parameters */
  dma_handle.Init.Channel             = SDRAM_DMAx_CHANNEL;
  dma_handle.Init.Direction           = DMA_MEMORY_TO_MEMORY;
  dma_handle.Init.PeriphInc           = DMA_PINC_ENABLE;
  dma_handle.Init.MemInc              = DMA_MINC_ENABLE;
  dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  dma_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
  dma_handle.Init.Mode                = DMA_NORMAL;
  dma_handle.Init.Priority            = DMA_PRIORITY_HIGH;
  dma_handle.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  dma_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  dma_handle.Init.MemBurst            = DMA_MBURST_SINGLE;
  dma_handle.Init.PeriphBurst         = DMA_PBURST_SINGLE;

  dma_handle.Instance = SDRAM_DMAx_STREAM;

   /* Associate the DMA handle */
  __HAL_LINKDMA(hsdram, hdma, dma_handle);

  /* Deinitialize the stream for new transfer */
  HAL_DMA_DeInit(&dma_handle);

  /* Configure the DMA stream */
  HAL_DMA_Init(&dma_handle);

  /* NVIC configuration for DMA transfer complete interrupt */
  HAL_NVIC_SetPriority(SDRAM_DMAx_IRQn, 0x0F, 0);
  HAL_NVIC_EnableIRQ(SDRAM_DMAx_IRQn);
#endif
}

/**
  * @brief  DeInitializes SDRAM MSP.
  * @param  hsdram SDRAM handle
  * @param  Params User parameters
  * @retval None
  */
__weak void BSP_SDRAM_MspDeInit(SDRAM_HandleTypeDef  *hsdram, void *Params)
{
#if 0
  static DMA_HandleTypeDef dma_handle;

  /* Disable NVIC configuration for DMA interrupt */
  HAL_NVIC_DisableIRQ(SDRAM_DMAx_IRQn);

  /* Deinitialize the stream for new transfer */
  dma_handle.Instance = SDRAM_DMAx_STREAM;
  HAL_DMA_DeInit(&dma_handle);
#endif

  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_DISABLE();


  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 |
                         GPIO_PIN_14 | GPIO_PIN_15);

  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_7  | GPIO_PIN_8  | GPIO_PIN_9  |
                         GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                         GPIO_PIN_15);

  HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3  | GPIO_PIN_4  |
                         GPIO_PIN_5  | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                         GPIO_PIN_15);

  HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_4  |
                         GPIO_PIN_5  | GPIO_PIN_8  | GPIO_PIN_15);

  HAL_GPIO_DeInit(GPIOH, GPIO_PIN_2  | GPIO_PIN_3  | GPIO_PIN_5  | GPIO_PIN_8  | GPIO_PIN_9  |
                         GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                         GPIO_PIN_15);

  HAL_GPIO_DeInit(GPIOI, GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3  | GPIO_PIN_4  |
                         GPIO_PIN_5  | GPIO_PIN_6  | GPIO_PIN_7  | GPIO_PIN_9  | GPIO_PIN_10);
}





#ifdef _USE_HW_CLI
void cliSdram(cli_args_t *args)
{
  bool ret = true;
  uint8_t number;
  uint32_t i;
  uint32_t pre_time;


  if (args->argc == 2)
  {
    if(args->isStr(0, "test") == true)
    {
      uint32_t *p_data = (uint32_t *)SDRAM_DEVICE_ADDR;

      number = (uint8_t)args->getData(1);

      while(number > 0)
      {
        pre_time = millis();
        for (i=0; i<SDRAM_DEVICE_SIZE/4; i++)
        {
          p_data[i] = i;
        }
        cliPrintf( "Write : %d MB/s\n", SDRAM_DEVICE_SIZE / (millis()-pre_time) / 1000 );


        volatile uint32_t data_sum = 0;
        pre_time = millis();
        for (i=0; i<SDRAM_DEVICE_SIZE/4; i++)
        {
          data_sum += p_data[i];
        }
        cliPrintf( "Read  : %d MB/s\n", SDRAM_DEVICE_SIZE / 1000 / (millis()-pre_time) );


        for (i=0; i<SDRAM_DEVICE_SIZE/4; i++)
        {
          if (p_data[i] != i)
          {
            cliPrintf( "%d : 0x%X fail\n", i, p_data[i]);
            break;
          }
        }

        if (i == SDRAM_DEVICE_SIZE/4)
        {
          cliPrintf( "Count %d\n", number);
          cliPrintf( "Sdram %d MB OK\n\n", SDRAM_DEVICE_SIZE/1024/1024);
          for (i=0; i<SDRAM_DEVICE_SIZE/4; i++)
          {
            p_data[i] = 0x5555AAAA;
          }
        }

        number--;

        if (cliAvailable() > 0)
        {
          cliPrintf( "Stop test...\n");
          break;
        }
      }
    }
    else
    {
      ret = false;
    }
  }
  else
  {
    ret = false;
  }


  if (ret == false)
  {
    cliPrintf( "sdram test 1~100 \n");
  }
}
#endif


#endif

