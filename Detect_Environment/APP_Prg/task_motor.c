#include "includes.h"

void task_motor(void *params)
{
	EventBits_t dht11_event;
	uint8_t dht11_data[2]={0};
	
	uint8_t uSpeed;
	uint8_t w25q64_msg[3]={0};
	int16_t k=0,b=0;
	
	while(1)
	{
		if(xQueueReceive(g_xQueue_W25Q64_to_Motor,w25q64_msg,0)==pdPASS)
		{
			k=85/(w25q64_msg[2]-w25q64_msg[1]-1);
			b=100-85*w25q64_msg[2]/(w25q64_msg[2]-w25q64_msg[1]-1);
		}
		
		if(System_Mode == SysMODE_STBY)
		{
			Motor_SetSpeed(0);
		}
		else if(System_Mode == SysMODE_NORM)
		{
			dht11_event = xEventGroupWaitBits(g_xEventDHT11, DHT11_Event_Motor, pdTRUE, pdTRUE, 0);
			if(dht11_event & DHT11_Event_Motor)
			{
				vTaskSuspendAll();
				dht11_data[0] = System_Data.dht11_hum;
				dht11_data[1] = System_Data.dht11_tem;
				xTaskResumeAll();
			}
						
			if(dht11_data[1]<=w25q64_msg[1])
			{
				uSpeed = 0;
			}
			else if((dht11_data[1]>=w25q64_msg[1]+1) && (dht11_data[1]<=w25q64_msg[2]))
			{
				uSpeed=dht11_data[1]*k+b+dht11_data[0]*0.2;
				if(uSpeed < 15)
				{
					uSpeed = 15;
				}
			}
			else if((dht11_data[1]>=w25q64_msg[2]+1) && dht11_data[1]<=50)
			{
				uSpeed = 100;
			}
			
			xTaskNotify(g_TaskHandleBuzzer, uSpeed, eSetValueWithOverwrite);
			xTaskNotify(g_TaskHandleOLED, uSpeed, eSetValueWithOverwrite);
			xTaskNotify(g_TaskHandleColorLED, uSpeed, eSetValueWithOverwrite);
			xTaskNotify(g_TaskHandleESP01S, uSpeed, eSetValueWithOverwrite);

			Motor_SetSpeed(uSpeed);

		}
		else if(System_Mode == SysMODE_SET)
		{
			Motor_SetSpeed(0);
		}

		vTaskDelay(1000);
	}
}
