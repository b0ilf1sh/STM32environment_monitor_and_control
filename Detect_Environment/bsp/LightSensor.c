#include "LightSensor.h"

uint8_t LightSensor_GetLight(void)
{
	uint16_t LightSensor_ADValue = AD_GetNeedValue(1);
	uint8_t light;
	light = 100 * (4096-LightSensor_ADValue) / 4096;
	return light;
}
