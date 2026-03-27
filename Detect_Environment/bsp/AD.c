#include "AD.h"

uint16_t AD_Data[AD_DATA_LEN]={0};

void AD_Init(void)
{
	HAL_ADCEx_Calibration_Start(&hadc1);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)AD_Data, AD_DATA_LEN);
}

uint16_t AD_GetNeedValue(uint16_t i)
{
	return AD_Data[i];
}
