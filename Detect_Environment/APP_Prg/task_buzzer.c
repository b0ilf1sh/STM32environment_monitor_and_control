#include "includes.h"

void task_buzzer(void *params)
{
	EventBits_t ir_rec_event;
	EventBits_t esp01s_event;
	
	uint8_t volumn[3]={0};
	uint8_t motor_data=0;
	
	while(1)
	{

		xQueueReceive(g_xQueue_W25Q64_to_Buzzer,volumn,0);
	
		ir_rec_event = xEventGroupWaitBits(g_xEventIR_Rec, IRRec_Event_Buzzer, pdTRUE, pdTRUE, 0);
		esp01s_event = xEventGroupWaitBits(g_xEventESP01S, ESP01S_Event_Buzzer, pdTRUE, pdTRUE, 0);
		
		if(System_Mode == SysMODE_NORM)
		{
			xTaskNotifyWait(~0, ~0, (uint32_t *)&motor_data, 0);
			
			if(motor_data)
			{
				Buzzer_Alarm(volumn[2]);
			}
			else 
			{
				if((ir_rec_event & IRRec_Event_Buzzer) || (esp01s_event & ESP01S_Event_Buzzer))
				{
					Buzzer_HintVoice(volumn[1]);
				}
				
				vTaskDelay(500);
			}
		}
		else if(System_Mode == SysMODE_UPDATE)
		{
			if((ir_rec_event & IRRec_Event_Buzzer))
			{
				ir_rec_event=0;
			}

			if((esp01s_event & ESP01S_Event_Buzzer))
			{
				esp01s_event=0;
			}

			vTaskDelay(1000);
		}
		else
		{
			if((ir_rec_event & IRRec_Event_Buzzer) || (esp01s_event & ESP01S_Event_Buzzer))
			{
				Buzzer_HintVoice(volumn[1]);
			}
			
			vTaskDelay(500);
		}
	}
}
