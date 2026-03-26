#include "includes.h"

void task_dht11(void *params)
{
	while(1)
	{
		if(System_Mode == SysMODE_NORM)
		{
			vTaskSuspendAll();//暂停调度
			DHT11_Run(&System_Data.dht11_hum, &System_Data.dht11_tem);
			xEventGroupSetBits(g_xEventDHT11, DHT11_Event_All);
			xTaskResumeAll();//恢复调度
		}
		else
		{
			DHT11_OFF();
		}
		
		vTaskDelay(1000);
	}
}
