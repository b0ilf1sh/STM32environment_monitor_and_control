#include "includes.h"

void task_esp01s(void *params)
{
	EventBits_t dht11_event;
	uint8_t dht11_data[3]={0};
	
	EventBits_t lightsensor_event;
	uint8_t light_data=0;

	uint8_t motor_data;
	uint8_t esp01s_state=0;
	sysmode_t esp01s_mode=SysMODE_STBY;
	
	uint8_t temp[3];//临时数组
	
	esp01s_senddata(APP_MODE_Send, System_Mode);
	
	while(1)
	{
		esp01s_flag = Esp01s_GetConnectState();//检测WIFI是否断开连接 
		if(esp01s_flag)
		{
			xSemaphoreTake(g_xMutex_SystemMode, portMAX_DELAY);
			
			if(esp01s_getdata(APP_MODE_Get, &esp01s_mode))
			{
				if(System_Mode != esp01s_mode)
				{
					xEventGroupSetBits(g_xEventESP01S, ESP01S_Event_Buzzer);
					
					System_Mode = esp01s_mode;
					if(esp01s_mode == SysMODE_SET)
					{
						xEventGroupSetBits(g_xEventESP01S, ESP01S_Event_W25Q64);
					}
				}
			}

			xSemaphoreGive(g_xMutex_SystemMode);
			
			lightsensor_event = xEventGroupWaitBits(g_xEventLightSensor, LightSensor_Event_ESP01S, pdTRUE, pdTRUE, 0);
			if(lightsensor_event & LightSensor_Event_ESP01S)
			{
				light_data = System_Data.lightsensor_light;
			}
			
			if(System_Mode == SysMODE_STBY)
			{
				esp01s_senddata(APP_BRT, light_data);
			}
			else if(System_Mode == SysMODE_NORM)
			{
				xTaskNotifyWait(0, 0, (uint32_t *)&motor_data, 0);
				
				dht11_event = xEventGroupWaitBits(g_xEventDHT11, DHT11_Event_ESP01S, pdTRUE, pdTRUE, 0);
				if(dht11_event & DHT11_Event_ESP01S)
				{
					vTaskSuspendAll();
					dht11_data[0] = System_Data.dht11_flag;
					dht11_data[1] = System_Data.dht11_hum;
					dht11_data[2] = System_Data.dht11_tem;
					xTaskResumeAll();
				}
				
				esp01s_send_normalmode_data(dht11_data[1], dht11_data[2], light_data, motor_data);
				
			}
			else if(System_Mode == SysMODE_SET)
			{
				xSemaphoreTake(g_xMutex_Set, portMAX_DELAY);
				
				if(xQueueReceive(g_xQueue_W25Q64_to_ESP01S,temp,0)==pdPASS)
				{
					if(temp[0]==1)
					{
						esp01s_send_setmode_temdata(temp[1], temp[2]);
					}
					else if(temp[0]==2)
					{
						esp01s_send_setmode_volumedata(temp[1], temp[2]);
					}
				}
				
				if(esp01s_gettemdata(&temp[1], &temp[2]) == 1)
				{
					temp[0] = 1;
					xQueueSend(g_xQueue_ESP01S_to_W25Q64,temp,0);
					esp01s_send_setmode_temdata(temp[1], temp[2]);
				}
				
				if(esp01s_getvolumedata(&temp[1], &temp[2]) == 1)
				{
					temp[0] = 2;
					xQueueSend(g_xQueue_ESP01S_to_W25Q64,temp,0);
					esp01s_send_setmode_volumedata(temp[1], temp[2]);
				}
				
				xSemaphoreGive(g_xMutex_Set);
				
				esp01s_senddata(APP_BRT, light_data);
			}
			
			if(System_Mode != esp01s_state)
			{
				esp01s_state = System_Mode;
				esp01s_senddata(APP_MODE_Send, System_Mode);
			}	
		}
		
		vTaskDelay(1000);
	}
}
