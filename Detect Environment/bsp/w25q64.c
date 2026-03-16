#include "w25q64.h"

void W25Q64_Init(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

void W25Q64_Start(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
}

void W25Q64_Stop(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
}

void W25Q64_SendData(uint8_t Data)
{
	HAL_SPI_Transmit(&hspi2, &Data, 1, HAL_MAX_DELAY);
}

uint8_t W25Q64_GetData(void)
{
	uint8_t Data;
	HAL_SPI_Receive(&hspi2, &Data, 1, HAL_MAX_DELAY);
	return Data;
}

void W25Q64_ReadID(uint8_t *M_ID, uint16_t *D_ID)
{
	W25Q64_Start();
	W25Q64_SendData(W25Q64_JEDEC_ID);
	uint8_t data1,data2,data3;
	data1 = W25Q64_GetData();
	data2 = W25Q64_GetData();
	data3 = W25Q64_GetData();
	*M_ID = data1;
	*D_ID = (data2<<8) | data3;
	W25Q64_Stop();
}

void W25Q64_WaitBusy(void)
{
	W25Q64_Start();
	W25Q64_SendData(W25Q64_StatusRegister1);
	while((W25Q64_GetData()&0x01)==0x01);
	W25Q64_Stop();
}

void W25Q64_WriteEnable(void)
{
	W25Q64_Start();
	W25Q64_SendData(W25Q64_Write_Enable);
	W25Q64_Stop();
}

void W25Q64_EraseSector(uint32_t Address)
{
	uint8_t i;
	
	W25Q64_WriteEnable();
	
	W25Q64_Start();
	W25Q64_SendData(W25Q64_Sector_Erase);
	for(i=0;i<3;i++)
	{
		W25Q64_SendData((Address >> (8*(3-i-1))));
	}
	W25Q64_Stop();
	
	W25Q64_WaitBusy();
}

void W25Q64_WritePage(uint32_t Address, uint8_t *TxData, uint16_t Size)
{
	uint16_t i;
	
	W25Q64_WriteEnable();
	
	W25Q64_Start();
	W25Q64_SendData(W25Q64_Page_Program);
	for(i=0;i<3;i++)
	{
		W25Q64_SendData((Address >> (8*(3-i-1))));
	}
	
	if(Size>256)
	{
		Size=256;
	}
	
	for(i=0;i<Size;i++)
	{
		W25Q64_SendData(TxData[i]);
	}
	W25Q64_Stop();
	
	W25Q64_WaitBusy();
}

void W25Q64_ReadPage(uint32_t Address, uint8_t *RxData, uint16_t Size)
{
	uint16_t i;
	W25Q64_Start();
	W25Q64_SendData(W25Q64_Read_Data);
	for(i=0;i<3;i++)
	{
		W25Q64_SendData((Address >> (8*(3-i-1))));
	}
	
	for(i=0;i<Size;i++)
	{
		RxData[i] = W25Q64_GetData();
	}
	W25Q64_Stop();
}
