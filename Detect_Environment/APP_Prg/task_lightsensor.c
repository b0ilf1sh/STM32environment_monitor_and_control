#include "includes.h"

void task_lightsensor(void *params)
{
	while(1)
	{
		
		System_Data.lightsensor_light=LightSensor_GetLight();
		
		if(System_Mode == SysMODE_NORM)
		{
			xEventGroupSetBits(g_xEventLightSensor, LightSensor_Event_All);
		}
		else
		{
			xEventGroupSetBits(g_xEventLightSensor, LightSensor_Event_ColorLED | LightSensor_Event_ESP01S);
		}
		
		vTaskDelay(1000);
	}
}
