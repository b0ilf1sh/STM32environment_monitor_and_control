#include "includes.h"

void task_w25q64(void *params)
{
	EventBits_t ir_rec_event;
	EventBits_t esp01s_event;
	
	uint8_t temp[3];//用来存储数据的临时数组
	
	while(1)
	{
		if(System_Mode == SysMODE_SET)
		{
			ir_rec_event = xEventGroupWaitBits(g_xEventIR_Rec, IRRec_Event_W25Q64, pdTRUE, pdTRUE, 0);
			esp01s_event = xEventGroupWaitBits(g_xEventESP01S, ESP01S_Event_W25Q64, pdTRUE, pdTRUE, 0);
			
			if((ir_rec_event & IRRec_Event_W25Q64) || (esp01s_event & ESP01S_Event_W25Q64))
			{
				W25Q64_ReadPage(W25Q64_TEMP_ADDRESS, temp, sizeof(temp));
				xQueueSend(g_xQueue_W25Q64_to_OLED,temp,0);
				xQueueSend(g_xQueue_W25Q64_to_ESP01S,temp,0);
				vTaskDelay(1);
				W25Q64_ReadPage(W25Q64_VOLUME_ADDRESS, temp, sizeof(temp));
				xQueueSend(g_xQueue_W25Q64_to_OLED,temp,0);
				xQueueSend(g_xQueue_W25Q64_to_ESP01S,temp,0);
			}
		}
		
		if(xQueueReceive(g_xQueue_OLED_to_W25Q64,temp,0)==pdPASS)
		{
			if(temp[0]==1)
			{
				W25Q64_EraseSector(W25Q64_TEMP_ADDRESS);
				W25Q64_WritePage(W25Q64_TEMP_ADDRESS, temp, sizeof(temp));
				xQueueSend(g_xQueue_W25Q64_to_Motor,temp,0);
			}
			else if(temp[0]==2)
			{
				W25Q64_EraseSector(W25Q64_VOLUME_ADDRESS);
				W25Q64_WritePage(W25Q64_VOLUME_ADDRESS, temp, sizeof(temp));
				xQueueSend(g_xQueue_W25Q64_to_Buzzer,temp,0);
			}
			xQueueSend(g_xQueue_W25Q64_to_ESP01S,temp,0);
		}
		
		if(xQueueReceive(g_xQueue_ESP01S_to_W25Q64,temp,0)==pdPASS)
		{
			if(temp[0]==1)
			{
				W25Q64_EraseSector(W25Q64_TEMP_ADDRESS);
				W25Q64_WritePage(W25Q64_TEMP_ADDRESS, temp, sizeof(temp));
				xQueueSend(g_xQueue_W25Q64_to_Motor,temp,0);
			}
			else if(temp[0]==2)
			{
				W25Q64_EraseSector(W25Q64_VOLUME_ADDRESS);
				W25Q64_WritePage(W25Q64_VOLUME_ADDRESS, temp, sizeof(temp));
				xQueueSend(g_xQueue_W25Q64_to_Buzzer,temp,0);
			}
			xQueueSend(g_xQueue_W25Q64_to_OLED,temp,0);
		}
		
		vTaskDelay(100);
	}
}
