/*
 * qspi.h
 *
 *  Created on: 2021. 12. 23.
 *      Author: hs
 */

#ifndef SRC_COMMON_HW_INCLUDE_QSPI_H_
#define SRC_COMMON_HW_INCLUDE_QSPI_H_

#ifdef __cplusplus
 extern "C" {
#endif


#include "hw_def.h"


#ifdef _USE_HW_QSPI



/* QSPI Error codes */
#define QSPI_OK            ((uint8_t)0x00)
#define QSPI_ERROR         ((uint8_t)0x01)
#define QSPI_BUSY          ((uint8_t)0x02)
#define QSPI_NOT_SUPPORTED ((uint8_t)0x04)
#define QSPI_SUSPENDED     ((uint8_t)0x08)


/* QSPI Base Address */
#define QSPI_BASE_ADDRESS          0x90000000


/* QSPI Info */
typedef struct {
 uint32_t FlashSize;          /*!< Size of the flash */
 uint32_t EraseSectorSize;    /*!< Size of sectors for the erase operation */
 uint32_t EraseSectorsNumber; /*!< Number of sectors for the erase operation */
 uint32_t ProgPageSize;       /*!< Size of pages for the program operation */
 uint32_t ProgPagesNumber;    /*!< Number of pages for the program operation */

 uint8_t  device_id[20];
} QSPI_Info_t;


/* Definition for QSPI clock resources */
#define QSPI_CLK_ENABLE()          __HAL_RCC_QSPI_CLK_ENABLE()
#define QSPI_CLK_DISABLE()         __HAL_RCC_QSPI_CLK_DISABLE()
#define QSPI_CS_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_CLK_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#define QSPI_D0_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()
#define QSPI_D1_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOC_CLK_ENABLE()
#define QSPI_D2_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOE_CLK_ENABLE()
#define QSPI_D3_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOD_CLK_ENABLE()

#define QSPI_FORCE_RESET()         __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET()       __HAL_RCC_QSPI_RELEASE_RESET()

/* Definition for QSPI Pins */
/* QSPI_CS */
#define QSPI_CS_PIN                GPIO_PIN_6
#define QSPI_CS_GPIO_PORT          GPIOB
#define QSPI_CS_PIN_AF             GPIO_AF10_QUADSPI
/* QSPI_CLK */
#define QSPI_CLK_PIN               GPIO_PIN_2
#define QSPI_CLK_GPIO_PORT         GPIOB
#define QSPI_CLK_PIN_AF            GPIO_AF9_QUADSPI
/* QSPI_D0 */
#define QSPI_D0_PIN                GPIO_PIN_9
#define QSPI_D0_GPIO_PORT          GPIOC
#define QSPI_D0_PIN_AF             GPIO_AF9_QUADSPI
/* QSPI_D1 */
#define QSPI_D1_PIN                GPIO_PIN_10
#define QSPI_D1_GPIO_PORT          GPIOC
#define QSPI_D1_PIN_AF             GPIO_AF9_QUADSPI
/* QSPI_D2 */
#define QSPI_D2_PIN                GPIO_PIN_2
#define QSPI_D2_GPIO_PORT          GPIOE
#define QSPI_D2_PIN_AF             GPIO_AF9_QUADSPI
/* QSPI_D3 */
#define QSPI_D3_PIN                GPIO_PIN_13
#define QSPI_D3_GPIO_PORT          GPIOD
#define QSPI_D3_PIN_AF             GPIO_AF9_QUADSPI


bool qspiInit(void);
bool qspiIsInit(void);
bool qspiReset(void);

bool qspiRead(uint32_t addr, uint8_t *p_data, uint32_t length);
bool qspiWrite(uint32_t addr, uint8_t *p_data, uint32_t length);
bool qspiErase(uint32_t addr, uint32_t length);
bool qspiEraseBlock(uint32_t block_addr);
bool qspiEraseSector(uint32_t sector_addr);
bool qspiEraseChip(void);
bool qspiGetStatus(void);
bool qspiGetInfo(QSPI_Info_t* p_info);
bool qspiEnableMemoryMappedMode(void);

uint32_t qspiGetAddr(void);
uint32_t qspiGetLength(void);


#endif


#ifdef __cplusplus
}
#endif

#endif /* SRC_COMMON_HW_INCLUDE_QSPI_H_ */
