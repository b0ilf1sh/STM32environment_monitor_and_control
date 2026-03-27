#ifndef _W25Q64_H_
#define _W25Q64_H_

#include "main.h"

#define W25Q64_JEDEC_ID           0x9F
#define W25Q64_StatusRegister1	  0x05
#define W25Q64_Write_Enable       0x06
#define W25Q64_Sector_Erase       0x20
#define W25Q64_Page_Program       0x02
#define W25Q64_Read_Data          0x03

extern SPI_HandleTypeDef hspi2;

void W25Q64_Init(void);
void W25Q64_ReadID(uint8_t *M_ID, uint16_t *D_ID);
void W25Q64_EraseSector(uint32_t Address);
void W25Q64_WritePage(uint32_t Address, uint8_t *TxData, uint16_t Size);
void W25Q64_ReadPage(uint32_t Address, uint8_t *RxData, uint16_t Size);

#endif
