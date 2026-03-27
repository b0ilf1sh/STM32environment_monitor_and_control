#include "Battery.h"

float Battery_GetVoltage(void)
{
	uint16_t Battery_ADValue = AD_GetNeedValue(0);
	float Voltage = Battery_ADValue / 4096.0 * 3.3;
	return Voltage;
}
