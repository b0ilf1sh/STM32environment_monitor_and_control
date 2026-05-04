#include "includes.h"

void task_colorled(void *params)
{
	EventBits_t lightsensor_event;
	uint8_t light_data=0;
	
	uint8_t motor_data=0;
	uint16_t lum=0;
	
	while(1)
	{
		lightsensor_event = xEventGroupWaitBits(g_xEventLightSensor, LightSensor_Event_ColorLED, pdTRUE, pdTRUE, 0);
		if(lightsensor_event & LightSensor_Event_ColorLED)
		{
			light_data = System_Data.lightsensor_light;
			if(light_data<33)
			{
				lum=85;
			}
			else if(light_data>=33 && light_data<66)
			{
				lum=170;
			}
			else if(light_data>=66 && light_data<100)
			{
				lum=255;
			}
		}
		
		if(System_Mode == SysMODE_NORM)
		{
			xTaskNotifyWait(0, 0, (uint32_t *)&motor_data, 0);
			
			if(motor_data)
			{
				ColorLED_Alarm();
			}
			else
			{
				ColorLED_SetColor(0, lum, 0);//正常运行时为绿色
				vTaskDelay(1000);
			}
		}
		else if(System_Mode == SysMODE_SET)
		{
			ColorLED_SetColor(lum, lum, 0);
			vTaskDelay(1000);
		}
		else if(System_Mode == SysMODE_STBY)
		{
			ColorLED_SetColor(0, 0, lum);
			vTaskDelay(1000);
		}
		else//升级模式
		{
			ColorLED_SetColor(lum, 0, lum);
			vTaskDelay(1000);
		}
	}
}
