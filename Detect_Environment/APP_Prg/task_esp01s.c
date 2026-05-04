#include "includes.h"

#define reconnect_time  60000

void task_esp01s(void *params)
{
	EventBits_t dht11_event;
	uint8_t dht11_data[2]={0};
	
	EventBits_t lightsensor_event;
	uint8_t light_data=0;

	uint8_t motor_data;
	uint8_t esp01s_state=0;
	sysmode_t esp01s_mode=SysMODE_STBY;
	
	uint8_t temp[3];//临时数组

	uint32_t connect_time=0;
	
	MQTT_SEND_DATA(APP_MODE_Send, System_Mode);
	
	while(1)
	{
		if(ESP01S_PROTOCOL_FLAG)
		{
			MQTT_CONNECT_FLAG = MQTT_GET_CONNECT_STATE();//获取连接状态

			if(MQTT_CONNECT_FLAG == 0)//连接上了服务器
			{
				if(System_Mode != SysMODE_UPDATE)
				{
					xSemaphoreTake(g_xMutex_SystemMode, portMAX_DELAY);
			
					if(MQTT_GET_DATA(APP_MODE_Get, &esp01s_mode))
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
				}
				
				lightsensor_event = xEventGroupWaitBits(g_xEventLightSensor, LightSensor_Event_ESP01S, pdTRUE, pdTRUE, 0);
				if(lightsensor_event & LightSensor_Event_ESP01S)
				{
					light_data = System_Data.lightsensor_light;
				}
				
				if(System_Mode == SysMODE_STBY)
				{
					MQTT_SEND_DATA(APP_BRT, light_data);
				}
				else if(System_Mode == SysMODE_NORM)
				{
					xTaskNotifyWait(0, 0, (uint32_t *)&motor_data, 0);
					
					dht11_event = xEventGroupWaitBits(g_xEventDHT11, DHT11_Event_ESP01S, pdTRUE, pdTRUE, 0);
					if(dht11_event & DHT11_Event_ESP01S)
					{
						vTaskSuspendAll();
						dht11_data[0] = System_Data.dht11_hum;
						dht11_data[1] = System_Data.dht11_tem;
						xTaskResumeAll();
					}
					MQTT_SEND_NORMAL_MODE_DATA(dht11_data[0], dht11_data[1], light_data, motor_data);	
				}
				else if(System_Mode == SysMODE_SET)
				{
					xSemaphoreTake(g_xMutex_Set, portMAX_DELAY);
					
					if(xQueueReceive(g_xQueue_W25Q64_to_ESP01S,temp,0)==pdPASS)
					{
						if(temp[0]==1)
						{
							MQTT_SEND_SetMode_TemData(temp[1], temp[2]);
						}
						else if(temp[0]==2)
						{
							MQTT_SEND_SetMode_VolumeData(temp[1], temp[2]);
						}
					}
					
					if(MQTT_GET_TemData(&temp[1], &temp[2]) == 1)
					{
						temp[0] = 1;
						xQueueSend(g_xQueue_ESP01S_to_W25Q64,temp,0);
						MQTT_SEND_SetMode_TemData(temp[1], temp[2]);
					}
					
					if(MQTT_GET_VolumeData(&temp[1], &temp[2]) == 1)
					{
						temp[0] = 2;
						xQueueSend(g_xQueue_ESP01S_to_W25Q64,temp,0);
						MQTT_SEND_SetMode_VolumeData(temp[1], temp[2]);
					}
					
					xSemaphoreGive(g_xMutex_Set);
					
					MQTT_SEND_DATA(APP_BRT, light_data);
				}
				
				if(System_Mode != esp01s_state)
				{
					esp01s_state = System_Mode;
					MQTT_SEND_DATA(APP_MODE_Send, System_Mode);
				}	
			}
			else//没有连接上服务器
			{
				if(connect_time == 0)//第一次重连
				{
					for(uint8_t i=0; i<3; i++)
					{
						//三次重连
						if(MQTT_Init()==0)
						{
							break;
						}

						//三次都没有成功
						if(i==2)
						{
							connect_time = HAL_GetTick();//记录当前时间
						}
					}
				}

				if((HAL_GetTick() - connect_time > reconnect_time) && connect_time != 0)
				{
					for(uint8_t i=0; i<3; i++)
					{
						//三次重连
						if(MQTT_Init()==0)
						{
							break;
						}

						//三次都没有成功
						if(i==2)
						{
							connect_time = HAL_GetTick();//记录当前时间
						}
					}
				}
			}
		}

		OTA_Run();//不断执行OTA程序

		vTaskDelay(1000);
	}
}
