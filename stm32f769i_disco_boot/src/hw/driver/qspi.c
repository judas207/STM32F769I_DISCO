/*
 * qspi.c
 *
 *  Created on: 2021. 12. 23.
 *      Author: hs
 */




#include "qspi.h"
#include "qspi/mx25l512.h"


#ifdef _USE_HW_QSPI



static bool is_init = false;

uint8_t BSP_QSPI_Init       (void);
uint8_t BSP_QSPI_DeInit     (void);
uint8_t BSP_QSPI_Read       (uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
uint8_t BSP_QSPI_Write      (uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
uint8_t BSP_QSPI_Erase_Block(uint32_t BlockAddress);
uint8_t BSP_QSPI_Erase_Chip (void);
uint8_t BSP_QSPI_GetStatus  (void);
uint8_t BSP_QSPI_GetInfo    (QSPI_Info_t* pInfo);
uint8_t BSP_QSPI_EnableMemoryMappedMode(void);
uint8_t BSP_QSPI_GetID(QSPI_Info_t* pInfo);
//uint8_t BSP_QSPI_Config(void);
uint8_t BSP_QSPI_Reset(void);

void BSP_QSPI_MspInit(QSPI_HandleTypeDef *hqspi, void *Params);
void BSP_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi, void *Params);

bool qspiInit(void)
{
  bool ret = true;
  QSPI_Info_t info;


  if (BSP_QSPI_Init() == QSPI_OK)
  {
    ret = true;
  }
  else
  {
    ret = false;
  }


  if (BSP_QSPI_GetID(&info) == QSPI_OK)
  {
    if (info.device_id[0] == 0x20 && info.device_id[1] ==0xBA && info.device_id[2] == 0x20)
    {
      logPrintf("QSPI mx25l512    \t: OK\r\n");
      ret = true;
    }
    else
    {
      logPrintf("QSPI mx25l512 ID \t: Fail %X %X %X\r\n", info.device_id[0], info.device_id[1], info.device_id[2]);
      ret = false;
    }
  }
  else
  {
    logPrintf("QSPI               \t: Fail\r\n");
    ret = false;
  }



  is_init = ret;

  return ret;
}

bool qspiReset(void)
{
  bool ret = false;



  if (is_init == true)
  {
    if (BSP_QSPI_Reset() == QSPI_OK)
    {
      ret = true;
    }
  }

  return ret;
}

bool qspiIsInit(void)
{
  return is_init;
}

bool qspiRead(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  uint8_t ret;

  if (addr >= qspiGetLength())
  {
    return false;
  }

  ret = BSP_QSPI_Read(p_data, addr, length);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  uint8_t ret;

  if (addr >= qspiGetLength())
  {
    return false;
  }

  ret = BSP_QSPI_Write(p_data, addr, length);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiEraseBlock(uint32_t block_addr)
{
  uint8_t ret;

  ret = BSP_QSPI_Erase_Block(block_addr);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiErase(uint32_t addr, uint32_t length)
{
  bool ret = true;
  uint32_t flash_length;
  uint32_t block_size;
  uint32_t block_begin;
  uint32_t block_end;
  uint32_t i;



  flash_length = MX25L512_FLASH_SIZE;
  block_size   = MX25L512_SUBSECTOR_SIZE;


  if ((addr > flash_length) || ((addr+length) > flash_length))
  {
    return false;
  }
  if (length == 0)
  {
    return false;
  }


  block_begin = addr / block_size;
  block_end   = (addr + length - 1) / block_size;


  for (i=block_begin; i<=block_end; i++)
  {
    ret = qspiEraseBlock(block_size*i);
    if (ret == false)
    {
      break;
    }
  }

  return ret;
}

bool qspiEraseChip(void)
{
  uint8_t ret;

  ret = BSP_QSPI_Erase_Chip();

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiGetStatus(void)
{
  uint8_t ret;

  ret = BSP_QSPI_GetStatus();

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiGetInfo(QSPI_Info_t* p_info)
{
  uint8_t ret;

  ret = BSP_QSPI_GetInfo((QSPI_Info_t *)p_info);

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool qspiEnableMemoryMappedMode(void)
{
  uint8_t ret;

  ret = BSP_QSPI_EnableMemoryMappedMode();

  if (ret == QSPI_OK)
  {
    return true;
  }
  else
  {
    return false;
  }
}

uint32_t qspiGetAddr(void)
{
  return QSPI_BASE_ADDRESS;
}

uint32_t qspiGetLength(void)
{
  return MX25L512_FLASH_SIZE;
}



QSPI_HandleTypeDef QSPIHandle;


/** @defgroup STM32F769I_DISCOVERY_QSPI_Private_Functions STM32F769I_DISCOVERY QSPI Private Functions
  * @{
  */
static uint8_t QSPI_ResetMemory(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_EnterFourBytesAddress(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_EnterMemory_QPI(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_ExitMemory_QPI(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_OutDrvStrengthCfg(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi);
static uint8_t QSPI_AutoPollingMemReady  (QSPI_HandleTypeDef *hqspi, uint32_t Timeout);

/**
  * @}
  */

/** @defgroup STM32F769I_DISCOVERY_QSPI_Exported_Functions STM32F769I_DISCOVERY QSPI Exported Functions
  * @{
  */

/**
  * @brief  Initializes the QSPI interface.
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_Init(void)
{
  QSPIHandle.Instance = QUADSPI;

  /* Call the DeInit function to reset the driver */
  if (HAL_QSPI_DeInit(&QSPIHandle) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* System level initialization */
  BSP_QSPI_MspInit(&QSPIHandle, NULL);

  /* QSPI initialization */
  /* QSPI freq = SYSCLK /(1 + ClockPrescaler) = 200 MHz/(1+1) = 100 Mhz */
  QSPIHandle.Init.ClockPrescaler     = 1;   /* QSPI freq = 200 MHz/(1+1) = 100 Mhz */
  QSPIHandle.Init.FifoThreshold      = 16;
  QSPIHandle.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  QSPIHandle.Init.FlashSize          = POSITION_VAL(MX25L512_FLASH_SIZE) - 1;  // 25
  QSPIHandle.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE; /* Min 30ns for nonRead */
  QSPIHandle.Init.ClockMode          = QSPI_CLOCK_MODE_0;
  QSPIHandle.Init.FlashID            = QSPI_FLASH_ID_1;
  QSPIHandle.Init.DualFlash          = QSPI_DUALFLASH_DISABLE;

  if (HAL_QSPI_Init(&QSPIHandle) != HAL_OK)
  {
    logPrintf("HAL_QSPI_Init() fail\n");
    return QSPI_ERROR;
  }

  /* QSPI memory reset */
  if (QSPI_ResetMemory(&QSPIHandle) != QSPI_OK)
  {
    logPrintf("QSPI_ResetMemory() fail\n");
    return QSPI_NOT_SUPPORTED;
  }

  /* Put QSPI memory in QPI mode */
  if( QSPI_EnterMemory_QPI( &QSPIHandle )!=QSPI_OK )
  {
    logPrintf("QSPI_EnterMemory_QPI() fail\n");
    return QSPI_NOT_SUPPORTED;
  }

  /* Set the QSPI memory in 4-bytes address mode */
  if (QSPI_EnterFourBytesAddress(&QSPIHandle) != QSPI_OK)
  {
    logPrintf("QSPI_EnterFourBytesAddress() fail\n");
    return QSPI_NOT_SUPPORTED;
  }

  /* Configuration of the dummy cycles on QSPI memory side */
  if (QSPI_DummyCyclesCfg(&QSPIHandle) != QSPI_OK)
  {
    logPrintf("QSPI_DummyCyclesCfg() fail\n");
    return QSPI_NOT_SUPPORTED;
  }

  /* Configuration of the Output driver strength on memory side */
  if( QSPI_OutDrvStrengthCfg( &QSPIHandle ) != QSPI_OK )
  {
    logPrintf("QSPI_OutDrvStrengthCfg() fail\n");
    return QSPI_NOT_SUPPORTED;
  }
#if 0
  if (BSP_QSPI_Config() != QSPI_OK)
  {
    logPrintf("QSPI_Config() fail\n");
    return QSPI_NOT_SUPPORTED;
  }
#endif
  return QSPI_OK;
}

uint8_t BSP_QSPI_Reset(void)
{
  if (QSPI_ResetMemory(&QSPIHandle) != QSPI_OK)
  {
    return QSPI_NOT_SUPPORTED;
  }

  return QSPI_OK;
}
#if 0
uint8_t BSP_QSPI_Config(void)
{
  uint8_t reg = 0;


  if (QSPI_ReadStatus(&hqspi, READ_STATUS_REG2_CMD, &reg) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  // QUAD MODE Enable
  if ((reg & (1<<1)) == 0x00)
  {
    reg |= (1<<1);
    if (QSPI_WriteStatus(&hqspi, WRITE_STATUS_REG2_CMD, reg) != QSPI_OK)
    {
      return QSPI_ERROR;
    }
  }


  return QSPI_OK;
}
#endif
/**
  * @brief  De-Initializes the QSPI interface.
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_DeInit(void)
{
  QSPIHandle.Instance = QUADSPI;

  /* Put QSPI memory in SPI mode */
  if( QSPI_ExitMemory_QPI(&QSPIHandle )!=QSPI_OK )
  {
    return QSPI_NOT_SUPPORTED;
  }

  /* Call the DeInit function to reset the driver */
  if (HAL_QSPI_DeInit(&QSPIHandle) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* System level De-initialization */
  BSP_QSPI_MspDeInit(&QSPIHandle, NULL);

  return QSPI_OK;
}

/**
  * @brief  Reads an amount of data from the QSPI memory.
  * @param  pData: Pointer to data to be read
  * @param  ReadAddr: Read start address
  * @param  Size: Size of data to read
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the read command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = QPI_READ_4_BYTE_ADDR_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.Address           = ReadAddr;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = MX25L512_DUMMY_CYCLES_READ_QUAD_IO;
  s_command.NbData            = Size;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Set S# timing for Read command */
  MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_1_CYCLE);

  /* Reception of the data */
  if (HAL_QSPI_Receive(&QSPIHandle, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Restore S# timing for nonRead commands */
  MODIFY_REG(QSPIHandle.Instance->DCR, QUADSPI_DCR_CSHT, QSPI_CS_HIGH_TIME_4_CYCLE);

  return QSPI_OK;
}

/**
  * @brief  Writes an amount of data to the QSPI memory.
  * @param  pData: Pointer to data to be written
  * @param  WriteAddr: Write start address
  * @param  Size: Size of data to write
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
{
  QSPI_CommandTypeDef s_command;
  uint32_t end_addr, current_size, current_addr;

  /* Calculation of the size between the write address and the end of the page */
  current_size = MX25L512_PAGE_SIZE - (WriteAddr % MX25L512_PAGE_SIZE);

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > Size)
  {
    current_size = Size;
  }

  /* Initialize the address variables */
  current_addr = WriteAddr;
  end_addr = WriteAddr + Size;

  /* Initialize the program command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = QPI_PAGE_PROG_4_BYTE_ADDR_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Perform the write page by page */
  do
  {
    s_command.Address = current_addr;
    s_command.NbData  = current_size;

    /* Enable write operations */
    if (QSPI_WriteEnable(&QSPIHandle) != QSPI_OK)
    {
      return QSPI_ERROR;
    }

    /* Configure the command */
    if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }

    /* Transmission of the data */
    if (HAL_QSPI_Transmit(&QSPIHandle, pData, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
      return QSPI_ERROR;
    }

    /* Configure automatic polling mode to wait for end of program */
    if (QSPI_AutoPollingMemReady(&QSPIHandle, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
    {
      return QSPI_ERROR;
    }

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    pData += current_size;
    current_size = ((current_addr + MX25L512_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : MX25L512_PAGE_SIZE;
  } while (current_addr < end_addr);

  return QSPI_OK;
}

/**
  * @brief  Erases the specified block of the QSPI memory.
  * @param  BlockAddress: Block address to erase
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_Erase_Block(uint32_t BlockAddress)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the erase command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = SUBSECTOR_ERASE_4_BYTE_ADDR_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.Address           = BlockAddress;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (QSPI_WriteEnable(&QSPIHandle) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady(&QSPIHandle, MX25L512_SUBSECTOR_ERASE_MAX_TIME) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  Erases the entire QSPI memory.
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_Erase_Chip(void)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the erase command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = BULK_ERASE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (QSPI_WriteEnable(&QSPIHandle) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of erase */
  if (QSPI_AutoPollingMemReady(&QSPIHandle, MX25L512_BULK_ERASE_MAX_TIME) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  Reads current status of the QSPI memory.
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_GetStatus(void)
{
  QSPI_CommandTypeDef s_command;
  uint8_t reg;

  /* Initialize the read flag status register command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = READ_STATUS_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(&QSPIHandle, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Check the value of the register*/
  if ((reg & MX25L512_SR_WIP) == 0)
  {
    return QSPI_OK;
  }
  else
  {
    return QSPI_BUSY;
  }
}

uint8_t BSP_QSPI_GetID(QSPI_Info_t* pInfo)
{
  QSPI_CommandTypeDef s_command;


  /* Initialize the read flag status register command */
#if 1
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = MULTIPLE_IO_READ_ID_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 20;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
#else
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = READ_ID_CMD2;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 20;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
#endif
  /* Configure the command */
  if (HAL_QSPI_Command(&QSPIHandle, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(&QSPIHandle, pInfo->device_id, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  Return the configuration of the QSPI memory.
  * @param  pInfo: pointer on the configuration structure
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_GetInfo(QSPI_Info_t* pInfo)
{
  /* Configure the structure with the memory configuration */
  pInfo->FlashSize          = MX25L512_FLASH_SIZE;
  pInfo->EraseSectorSize    = MX25L512_SUBSECTOR_SIZE;
  pInfo->EraseSectorsNumber = (MX25L512_FLASH_SIZE/MX25L512_SUBSECTOR_SIZE);
  pInfo->ProgPageSize       = MX25L512_PAGE_SIZE;
  pInfo->ProgPagesNumber    = (MX25L512_FLASH_SIZE/MX25L512_PAGE_SIZE);

  return QSPI_OK;
}

/**
  * @brief  Configure the QSPI in memory-mapped mode
  * @retval QSPI memory status
  */
uint8_t BSP_QSPI_EnableMemoryMappedMode(void)
{
  QSPI_CommandTypeDef      s_command;
  QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;

  /* Configure the command for the read instruction */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = QPI_READ_4_BYTE_ADDR_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_4_LINES;
  s_command.AddressSize       = QSPI_ADDRESS_32_BITS;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = MX25L512_DUMMY_CYCLES_READ_QUAD_IO;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the memory mapped mode */
  s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
  s_mem_mapped_cfg.TimeOutPeriod     = 0;

  if (HAL_QSPI_MemoryMapped(&QSPIHandle, &s_command, &s_mem_mapped_cfg) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

__weak void BSP_QSPI_MspInit(QSPI_HandleTypeDef *hqspi, void *Params)
{

  GPIO_InitTypeDef gpio_init_structure;

  if(hqspi->Instance==QUADSPI)
  {
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable the QuadSPI memory interface clock */
    QSPI_CLK_ENABLE();
    /* Reset the QuadSPI memory interface */
    QSPI_FORCE_RESET();
    QSPI_RELEASE_RESET();
    /* Enable GPIO clocks */
    QSPI_CS_GPIO_CLK_ENABLE();
    QSPI_CLK_GPIO_CLK_ENABLE();
    QSPI_D0_GPIO_CLK_ENABLE();
    QSPI_D1_GPIO_CLK_ENABLE();
    QSPI_D2_GPIO_CLK_ENABLE();
    QSPI_D3_GPIO_CLK_ENABLE();

    /**QUADSPI GPIO Configuration
    PE2     ------> QUADSPI_BK1_IO2
    PB6     ------> QUADSPI_BK1_NCS
    PC10     ------> QUADSPI_BK1_IO1
    PC9     ------> QUADSPI_BK1_IO0
    PB2     ------> QUADSPI_CLK
    PD13     ------> QUADSPI_BK1_IO3
    */
    /* QSPI CS GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_CS_PIN;
    gpio_init_structure.Alternate = QSPI_CS_PIN_AF;
    gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull      = GPIO_PULLUP;
    gpio_init_structure.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(QSPI_CS_GPIO_PORT, &gpio_init_structure);
    /* QSPI CLK GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_CLK_PIN;
    gpio_init_structure.Alternate = QSPI_CLK_PIN_AF;
    gpio_init_structure.Pull      = GPIO_NOPULL;
    HAL_GPIO_Init(QSPI_CLK_GPIO_PORT, &gpio_init_structure);
    /* QSPI D0 GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_D0_PIN;
    gpio_init_structure.Alternate = QSPI_D0_PIN_AF;
    HAL_GPIO_Init(QSPI_D0_GPIO_PORT, &gpio_init_structure);
    /* QSPI D1 GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_D1_PIN;
    gpio_init_structure.Alternate = QSPI_D1_PIN_AF;
    HAL_GPIO_Init(QSPI_D1_GPIO_PORT, &gpio_init_structure);
    /* QSPI D2 GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_D2_PIN;
    gpio_init_structure.Alternate = QSPI_D2_PIN_AF;
    HAL_GPIO_Init(QSPI_D2_GPIO_PORT, &gpio_init_structure);
    /* QSPI D3 GPIO pin configuration  */
    gpio_init_structure.Pin       = QSPI_D3_PIN;
    gpio_init_structure.Alternate = QSPI_D3_PIN_AF;
    HAL_GPIO_Init(QSPI_D3_GPIO_PORT, &gpio_init_structure);

  /* USER CODE BEGIN QUADSPI_MspInit 1 */

  /* USER CODE END QUADSPI_MspInit 1 */
  }
}

__weak void BSP_QSPI_MspDeInit(QSPI_HandleTypeDef *hqspi, void *Params)
{

  if(hqspi->Instance==QUADSPI)
  {
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();

    /**QUADSPI GPIO Configuration
    PE2     ------> QUADSPI_BK1_IO2
    PB6     ------> QUADSPI_BK1_NCS
    PC10     ------> QUADSPI_BK1_IO1
    PC9     ------> QUADSPI_BK1_IO0
    PB2     ------> QUADSPI_CLK
    PD13     ------> QUADSPI_BK1_IO3
    */
    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* De-Configure QSPI pins */
    HAL_GPIO_DeInit(QSPI_CS_GPIO_PORT, QSPI_CS_PIN);
    HAL_GPIO_DeInit(QSPI_CLK_GPIO_PORT, QSPI_CLK_PIN);
    HAL_GPIO_DeInit(QSPI_D0_GPIO_PORT, QSPI_D0_PIN);
    HAL_GPIO_DeInit(QSPI_D1_GPIO_PORT, QSPI_D1_PIN);
    HAL_GPIO_DeInit(QSPI_D2_GPIO_PORT, QSPI_D2_PIN);
    HAL_GPIO_DeInit(QSPI_D3_GPIO_PORT, QSPI_D3_PIN);

    /*##-3- Reset peripherals ##################################################*/
    /* Reset the QuadSPI memory interface */
    QSPI_FORCE_RESET();
    QSPI_RELEASE_RESET();

    /* Disable the QuadSPI memory interface clock */
    QSPI_CLK_DISABLE();
  }
}

/**
  * @brief  This function reset the QSPI memory.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_ResetMemory(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef      s_command;
  QSPI_AutoPollingTypeDef  s_config;
  uint8_t                  reg;

  /* Send command RESET command in QPI mode (QUAD I/Os) */
  /* Initialize the reset enable command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = RESET_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
  /* Send the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }
  /* Send the reset memory command */
  s_command.Instruction = RESET_MEMORY_CMD;
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Send command RESET command in SPI mode */
  /* Initialize the reset enable command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = RESET_ENABLE_CMD;
  /* Send the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }
  /* Send the reset memory command */
  s_command.Instruction = RESET_MEMORY_CMD;
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* After reset CMD, 1000ms requested if QSPI memory SWReset occured during full chip erase operation */
  HAL_Delay( 1000 );

  /* Configure automatic polling mode to wait the WIP bit=0 */
  s_config.Match           = 0;
  s_config.Mask            = MX25L512_SR_WIP;
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  s_command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction     = READ_STATUS_REG_CMD;
  s_command.DataMode        = QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Initialize the reading of status register */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = READ_STATUS_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Enable write operations, command in 1 bit */
  /* Enable write operations */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = WRITE_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for write enabling */
  s_config.Match           = MX25L512_SR_WREN;
  s_config.Mask            = MX25L512_SR_WREN;
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  s_command.Instruction    = READ_STATUS_REG_CMD;
  s_command.DataMode       = QSPI_DATA_1_LINE;

  if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Update the configuration register with new dummy cycles */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = WRITE_STATUS_CFG_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable the Quad IO on the QSPI memory (Non-volatile bit) */
  reg |= MX25L512_SR_QUADEN;

  /* Configure the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Transmission of the data */
  if (HAL_QSPI_Transmit(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* 40ms  Write Status/Configuration Register Cycle Time */
  HAL_Delay( 40 );

  return QSPI_OK;
}

/**
  * @brief  This function set the QSPI memory in 4-byte address mode
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_EnterFourBytesAddress(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = ENTER_4_BYTE_ADDR_MODE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Enable write operations */
  if (QSPI_WriteEnable(hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Send the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait the memory is ready */
  if (QSPI_AutoPollingMemReady(hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  This function configure the dummy cycles on memory side.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_DummyCyclesCfg(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef s_command;
  uint8_t reg[2];

  /* Initialize the reading of status register */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = READ_STATUS_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(hqspi, &(reg[0]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Initialize the reading of configuration register */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = READ_CFG_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(hqspi, &(reg[1]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Enable write operations */
  if (QSPI_WriteEnable(hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Update the configuration register with new dummy cycles */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = WRITE_STATUS_CFG_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 2;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* MX25L512_DUMMY_CYCLES_READ_QUAD = 3 for 10 cycles in QPI mode */
  MODIFY_REG( reg[1], MX25L512_CR_NB_DUMMY, (MX25L512_DUMMY_CYCLES_READ_QUAD << POSITION_VAL(MX25L512_CR_NB_DUMMY)));

  /* Configure the write volatile configuration register command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Transmission of the data */
  if (HAL_QSPI_Transmit(hqspi, &(reg[0]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* 40ms  Write Status/Configuration Register Cycle Time */
  HAL_Delay( 40 );

  return QSPI_OK;
}

/**
  * @brief  This function put QSPI memory in QPI mode (quad I/O).
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_EnterMemory_QPI( QSPI_HandleTypeDef *hqspi )
{
  QSPI_CommandTypeDef      s_command;
  QSPI_AutoPollingTypeDef  s_config;

  /* Initialize the QPI enable command */
  /* QSPI memory is supported to be in SPI mode, so CMD on 1 LINE */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = ENTER_QUAD_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait the QUADEN bit=1 and WIP bit=0 */
  s_config.Match           = MX25L512_SR_QUADEN;
  s_config.Mask            = MX25L512_SR_QUADEN|MX25L512_SR_WIP;
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = READ_STATUS_REG_CMD;
  s_command.DataMode          = QSPI_DATA_4_LINES;

  if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  This function put QSPI memory in SPI mode.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_ExitMemory_QPI( QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef      s_command;

  /* Initialize the QPI enable command */
  /* QSPI memory is supported to be in QPI mode, so CMD on 4 LINES */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = EXIT_QUAD_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Send the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  This function configure the Output driver strength on memory side.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_OutDrvStrengthCfg( QSPI_HandleTypeDef *hqspi )
{
  QSPI_CommandTypeDef s_command;
  uint8_t reg[2];

  /* Initialize the reading of status register */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = READ_STATUS_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(hqspi, &(reg[0]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Initialize the reading of configuration register */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = READ_CFG_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(hqspi, &(reg[1]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Enable write operations */
  if (QSPI_WriteEnable(&QSPIHandle) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Update the configuration register with new output driver strength */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = WRITE_STATUS_CFG_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 2;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Set Output Strength of the QSPI memory 15 ohms */
  MODIFY_REG( reg[1], MX25L512_CR_ODS, (MX25L512_CR_ODS_15 << POSITION_VAL(MX25L512_CR_ODS)));

  /* Configure the write volatile configuration register command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Transmission of the data */
  if (HAL_QSPI_Transmit(hqspi, &(reg[0]), HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  hqspi: QSPI handle
  * @retval None
  */
static uint8_t QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi)
{
  QSPI_CommandTypeDef     s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Enable write operations */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = WRITE_ENABLE_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_NONE;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for write enabling */
  s_config.Match           = MX25L512_SR_WREN;
  s_config.Mask            = MX25L512_SR_WREN;
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  s_command.Instruction    = READ_STATUS_REG_CMD;
  s_command.DataMode       = QSPI_DATA_4_LINES;

  if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}

/**
  * @brief  This function read the SR of the memory and wait the EOP.
  * @param  hqspi: QSPI handle
  * @param  Timeout
  * @retval None
  */
static uint8_t QSPI_AutoPollingMemReady(QSPI_HandleTypeDef *hqspi, uint32_t Timeout)
{
  QSPI_CommandTypeDef     s_command;
  QSPI_AutoPollingTypeDef s_config;

  /* Configure automatic polling mode to wait for memory ready */
  s_command.InstructionMode   = QSPI_INSTRUCTION_4_LINES;
  s_command.Instruction       = READ_STATUS_REG_CMD;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_4_LINES;
  s_command.DummyCycles       = 0;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  s_config.Match           = 0;
  s_config.Mask            = MX25L512_SR_WIP;
  s_config.MatchMode       = QSPI_MATCH_MODE_AND;
  s_config.StatusBytesSize = 1;
  s_config.Interval        = 0x10;
  s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  if (HAL_QSPI_AutoPolling(hqspi, &s_command, &s_config, Timeout) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}


#if 0
static uint8_t QSPI_ReadStatus(QSPI_HandleTypeDef *hqspi, uint8_t cmd, uint8_t *p_data)
{
  QSPI_CommandTypeDef s_command;
  uint8_t reg;

  /* Initialize the read flag status register command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = cmd;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  /* Configure the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Reception of the data */
  if (HAL_QSPI_Receive(hqspi, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  *p_data = reg;

  return QSPI_OK;
}

static uint8_t QSPI_WriteStatus(QSPI_HandleTypeDef *hqspi, uint8_t cmd, uint8_t data)
{
  QSPI_CommandTypeDef s_command;

  /* Initialize the program command */
  s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  s_command.Instruction       = cmd;
  s_command.AddressMode       = QSPI_ADDRESS_NONE;
  s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  s_command.DataMode          = QSPI_DATA_1_LINE;
  s_command.DummyCycles       = 0;
  s_command.NbData            = 1;
  s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
  s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;



  /* Enable write operations */
  if (QSPI_WriteEnable(hqspi) != QSPI_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure the command */
  if (HAL_QSPI_Command(hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Transmission of the data */
  if (HAL_QSPI_Transmit(hqspi, &data, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  /* Configure automatic polling mode to wait for end of program */
  if (QSPI_AutoPollingMemReady(hqspi, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != QSPI_OK)
  {
    return QSPI_ERROR;
  }


  return QSPI_OK;
}
#endif


#endif

