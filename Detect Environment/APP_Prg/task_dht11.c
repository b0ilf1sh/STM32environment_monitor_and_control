#include "includes.h"

void task_dht11(void *params)
{
	while(1)
	{
		if(System_Mode == SysMODE_NORM)
		{
			taskENTER_CRITICAL();//禁用中断
			System_Data.dht11_flag= DHT11_GetData(&System_Data.dht11_hum, &System_Data.dht11_tem);
			xEventGroupSetBits(g_xEventDHT11, DHT11_Event_All);
			taskEXIT_CRITICAL();//开启中断
		}
		else
		{
			DHT11_OFF();
		}
		
		vTaskDelay(500);
	}
}
