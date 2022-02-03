/*
 * flash.c
 *
 *  Created on: 2021. 12. 17.
 *      Author: hs
 */




#include "flash.h"
#include "qspi.h"
#include "cli.h"


#ifdef _USE_HW_FLASH


//#define DUAL_BANK   // Use the ST-Link Utility (Target -> Options Bytes) to specify the FLASH mode: single bank or dual bank

#define FLASH_MAX_BANK            1

#if defined(DUAL_BANK)
#define FLASH_MAX_SECTOR          24
#else  // SINGLE_BANK
#define FLASH_MAX_SECTOR          12
#endif /* DUAL_BANK */

typedef struct
{
  int16_t  index;
  uint32_t bank;
  uint32_t addr;
  uint32_t length;
} flash_tbl_t;


const flash_tbl_t flash_tbl_bank1[FLASH_MAX_SECTOR] =
    {
#if defined(DUAL_BANK)
        { 0, FLASH_BANK_1, 0x08000000,  16*1024}, /* Base address of Sector  0,  16 Kbytes */
        { 1, FLASH_BANK_1, 0x08004000,  16*1024}, /* Base address of Sector  1,  16 Kbytes */
        { 2, FLASH_BANK_1, 0x08008000,  16*1024}, /* Base address of Sector  2,  16 Kbytes */
        { 3, FLASH_BANK_1, 0x0800C000,  16*1024}, /* Base address of Sector  3,  16 Kbytes */
        { 4, FLASH_BANK_1, 0x08010000,  64*1024}, /* Base address of Sector  4,  64 Kbytes */
        { 5, FLASH_BANK_1, 0x08020000, 128*1024}, /* Base address of Sector  5, 128 Kbytes */
        { 6, FLASH_BANK_1, 0x08040000, 128*1024}, /* Base address of Sector  6, 128 Kbytes */
        { 7, FLASH_BANK_1, 0x08060000, 128*1024}, /* Base address of Sector  7, 128 Kbytes */
        { 8, FLASH_BANK_1, 0x08080000, 128*1024}, /* Base address of Sector  8, 128 Kbytes */
        { 9, FLASH_BANK_1, 0x080A0000, 128*1024}, /* Base address of Sector  9, 128 Kbytes */
        {10, FLASH_BANK_1, 0x080C0000, 128*1024}, /* Base address of Sector 10, 128 Kbytes */
        {11, FLASH_BANK_1, 0x080E0000, 128*1024}, /* Base address of Sector 11, 128 Kbytes */
        {12, FLASH_BANK_1, 0x08100000,  16*1024}, /* Base address of Sector 12,  16 Kbytes */
        {13, FLASH_BANK_1, 0x08104000,  16*1024}, /* Base address of Sector 13,  16 Kbytes */
        {14, FLASH_BANK_1, 0x08108000,  16*1024}, /* Base address of Sector 14,  16 Kbytes */
        {15, FLASH_BANK_1, 0x0810C000,  16*1024}, /* Base address of Sector 15,  16 Kbytes */
        {16, FLASH_BANK_1, 0x08110000,  64*1024}, /* Base address of Sector 16,  64 Kbytes */
        {17, FLASH_BANK_1, 0x08120000, 128*1024}, /* Base address of Sector 17, 128 Kbytes */
        {18, FLASH_BANK_1, 0x08140000, 128*1024}, /* Base address of Sector 18, 128 Kbytes */
        {19, FLASH_BANK_1, 0x08160000, 128*1024}, /* Base address of Sector 19, 128 Kbytes */
        {20, FLASH_BANK_1, 0x08180000, 128*1024}, /* Base address of Sector 20, 128 Kbytes */
        {21, FLASH_BANK_1, 0x081A0000, 128*1024}, /* Base address of Sector 21, 128 Kbytes */
        {22, FLASH_BANK_1, 0x081C0000, 128*1024}, /* Base address of Sector 22, 128 Kbytes */
        {23, FLASH_BANK_1, 0x081E0000, 128*1024}, /* Base address of Sector 23, 128 Kbytes */
#else
        { 0, FLASH_BANK_1, 0x08000000,  32*1024}, /* Base address of Sector  0,  32 Kbytes */
        { 1, FLASH_BANK_1, 0x08008000,  32*1024}, /* Base address of Sector  1,  32 Kbytes */
        { 2, FLASH_BANK_1, 0x08010000,  32*1024}, /* Base address of Sector  2,  32 Kbytes */
        { 3, FLASH_BANK_1, 0x08018000,  32*1024}, /* Base address of Sector  3,  32 Kbytes */
        { 4, FLASH_BANK_1, 0x08020000, 128*1024}, /* Base address of Sector  4, 128 Kbytes */
        { 5, FLASH_BANK_1, 0x08040000, 256*1024}, /* Base address of Sector  5, 256 Kbytes */
        { 6, FLASH_BANK_1, 0x08080000, 256*1024}, /* Base address of Sector  6, 256 Kbytes */
        { 7, FLASH_BANK_1, 0x080C0000, 256*1024}, /* Base address of Sector  7, 256 Kbytes */
        { 8, FLASH_BANK_1, 0x08100000, 256*1024}, /* Base address of Sector  8, 256 Kbytes */
        { 9, FLASH_BANK_1, 0x08140000, 256*1024}, /* Base address of Sector  9, 256 Kbytes */
        {10, FLASH_BANK_1, 0x08180000, 256*1024}, /* Base address of Sector 10, 256 Kbytes */
        {11, FLASH_BANK_1, 0x081C0000, 256*1024}, /* Base address of Sector 11, 256 Kbytes */
#endif /* DUAL_BANK */
    };
const flash_tbl_t flash_tbl_bank2[FLASH_MAX_SECTOR] =
    {
    };


#ifdef _USE_HW_CLI
static void cliFlash(cli_args_t *args);
#endif





bool flashInit(void)
{


#ifdef _USE_HW_CLI
  cliAdd("flash", cliFlash);
#endif
  return true;
}

bool flashErase(uint32_t addr, uint32_t length)
{
  bool ret = false;

  int32_t start_sector = -1;
  int32_t end_sector = -1;
  uint32_t banks;
  const flash_tbl_t *flash_tbl;


#ifdef _USE_HW_QSPI
  if (addr >= qspiGetAddr() && addr < (qspiGetAddr() + qspiGetLength()))
  {
    ret = qspiErase(addr - qspiGetAddr(), length);
    return ret;
  }
#endif


  for (banks = 0; banks < FLASH_MAX_BANK; banks++)
  {
    start_sector = -1;
    end_sector = -1;

    if (banks == 0)
    {
      flash_tbl = flash_tbl_bank1;
    }
    else
    {
      flash_tbl = flash_tbl_bank2;
    }

    for (int i=0; i<FLASH_MAX_SECTOR; i++)
    {
      bool update = false;
      uint32_t start_addr;
      uint32_t end_addr;


      start_addr = flash_tbl[i].addr;
      end_addr   = flash_tbl[i].addr + flash_tbl[i].length - 1;

      if (start_addr >= addr && start_addr < (addr+length))
      {
        update = true;
      }
      if (end_addr >= addr && end_addr < (addr+length))
      {
        update = true;
      }

      if (addr >= start_addr && addr <= end_addr)
      {
        update = true;
      }
      if ((addr+length-1) >= start_addr && (addr+length-1) <= end_addr)
      {
        update = true;
      }


      if (update == true)
      {
        if (start_sector < 0)
        {
          start_sector = i;
        }
        end_sector = i;
      }
    }

    if (start_sector >= 0)
    {
      FLASH_EraseInitTypeDef EraseInit;
      uint32_t SectorError;
      HAL_StatusTypeDef status;
      FLASH_OBProgramInitTypeDef    OBInit;

      HAL_FLASH_Unlock();

      /* Allow Access to option bytes sector */
      HAL_FLASH_OB_Unlock();
      /* Get the Dual bank configuration status */
      HAL_FLASHEx_OBGetConfig(&OBInit);
    #if defined(DUAL_BANK)
      /* FLASH is configured in Dual Bank mode */
      if((OBInit.USERConfig & OB_NDBANK_SINGLE_BANK) == OB_NDBANK_SINGLE_BANK)
    #else
      /* FLASH is configured in Single Bank mode */
      if((OBInit.USERConfig & OB_NDBANK_SINGLE_BANK) == OB_NDBANK_DUAL_BANK)
    #endif
      {
        HAL_FLASH_Lock();
        return ret;
      }

      EraseInit.Sector       = start_sector;
      EraseInit.NbSectors    = (end_sector - start_sector) + 1;
      EraseInit.TypeErase    = FLASH_TYPEERASE_SECTORS;
      EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
      EraseInit.Banks        = flash_tbl[start_sector].bank;

      status = HAL_FLASHEx_Erase(&EraseInit, &SectorError);
      if (status == HAL_OK)
      {
#if 0
        uint32_t erase_len = 0;

        for (int i=start_sector; i<=end_sector; i++)
        {
          erase_len += flash_tbl[i].length;
        }

        SCB_InvalidateDCache_by_Addr((void *)flash_tbl[start_sector].addr, erase_len);
#endif
        ret = true;
      }
    }
  }

  HAL_FLASH_Lock();

  return ret;
}

bool flashWrite(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  HAL_StatusTypeDef status;


#ifdef _USE_HW_QSPI
  if (addr >= qspiGetAddr() && addr < (qspiGetAddr() + qspiGetLength()))
  {
    ret = qspiWrite(addr - qspiGetAddr(), p_data, length);
    return ret;
  }
#endif


  if (addr%4 != 0)
  {
    return false;
  }

  HAL_FLASH_Unlock();

  for (int i=0; i<length; i++)//i+=4)
  {
    uint8_t data;

    data  = p_data[i];
    //data  = p_data[i+0] << 0;
    //data |= p_data[i+1] << 8;
    //data |= p_data[i+2] <<16;
    //data |= p_data[i+3] <<24;

    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr + i, (uint64_t)data);
    if (status != HAL_OK)
    {
      ret = false;
      break;
    }
  }

  HAL_FLASH_Lock();

  return ret;
}

bool flashRead(uint32_t addr, uint8_t *p_data, uint32_t length)
{
  bool ret = true;
  uint8_t *p_byte = (uint8_t *)addr;


#ifdef _USE_HW_QSPI
  if (addr >= qspiGetAddr() && addr < (qspiGetAddr() + qspiGetLength()))
  {
    ret = qspiRead(addr - qspiGetAddr(), p_data, length);
    return ret;
  }
#endif

  for (int i=0; i<length; i++)
  {
    p_data[i] = p_byte[i];
  }

  return ret;
}



#ifdef _USE_HW_CLI
void cliFlash(cli_args_t *args)
{
  bool ret = true;
  uint32_t i;
  uint32_t addr;
  uint32_t length;
  uint8_t  data;
  uint32_t pre_time;
  bool flash_ret;
  const flash_tbl_t *flash_tbl;

  flash_tbl = flash_tbl_bank1;

  if (args->argc == 1)
  {
    if(args->isStr(0, "info") == true)
    {
      cliPrintf("flash addr  : 0x%X\n", flash_tbl[0].addr);
#ifdef _USE_HW_QSPI
      cliPrintf("qspi  addr  : 0x%X\n", QSPI_BASE_ADDRESS);
#endif
    }
    else
    {
      ret = false;
    }
  }
  else if (args->argc == 3)
  {
    if(args->isStr(0, "read") == true)
    {
      addr   = (uint32_t)args->getData(1);
      length = (uint32_t)args->getData(2);

      for (i=0; i<length; i++)
      {
        flash_ret = flashRead(addr+i, &data, 1);

        if (flash_ret == true)
        {
          cliPrintf( "addr : 0x%X\t 0x%02X (%c)\n", addr+i, data, data);
        }
        else
        {
          cliPrintf( "addr : 0x%X\t Fail\n", addr+i);
        }
      }
    }
    else if(args->isStr(0, "erase") == true)
    {
      addr   = (uint32_t)args->getData(1);
      length = (uint32_t)args->getData(2);

      pre_time = millis();
      flash_ret = flashErase(addr, length);

      cliPrintf( "addr : 0x%X\t len : %d %d ms\n", addr, length, (millis()-pre_time));
      if (flash_ret)
      {
        cliPrintf("OK\n");
      }
      else
      {
        cliPrintf("FAIL\n");
      }
    }
    else if(args->isStr(0, "write") == true)
    {
      addr = (uint32_t)args->getData(1);
      data = (uint8_t )args->getData(2);

      pre_time = millis();
      flash_ret = flashWrite(addr, &data, 1);

      cliPrintf( "addr : 0x%X\t 0x%02X (%c) %dms\n", addr, data, data, millis()-pre_time);
      if (flash_ret)
      {
        cliPrintf("OK\n");
      }
      else
      {
        cliPrintf("FAIL\n");
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
    cliPrintf( "flash info\n");
    cliPrintf( "flash read  [addr] [length]\n");
    cliPrintf( "flash erase [addr] [length]\n");
    cliPrintf( "flash write [addr] [data]\n");
  }

}
#endif


#endif
