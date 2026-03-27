#ifndef _AD_H_
#define _AD_H_

#include "main.h"

#define AD_DATA_LEN  2

extern ADC_HandleTypeDef hadc1;

void AD_Init(void);
uint16_t AD_GetNeedValue(uint16_t i);

#endif
