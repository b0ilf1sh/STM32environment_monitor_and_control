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

		//打印出当前任务剩余空间
		// TaskHandle_t xTaskHandle = xTaskGetCurrentTaskHandle();
		// UBaseType_t freenum = uxTaskGetStackHighWaterMark(xTaskHandle);
		// printf("freestack=%d\r\n", freenum);

		vTaskDelay(1000);
	}
}
