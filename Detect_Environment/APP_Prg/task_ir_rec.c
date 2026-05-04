#include "includes.h"

void task_ir_rec(void *params)
{
	uint8_t IR_KeyVal;
	uint8_t IR_RepeatFlag;
	
	while(1)
	{
		IR_KeyVal=IR_GetData();
		IR_RepeatFlag=IR_GetRepeatFlag();
		
		if(IR_KeyVal && IR_RepeatFlag==0)
		{
			xEventGroupSetBits(g_xEventIR_Rec, IRRec_Event_Buzzer);
		}

		if(System_Mode != SysMODE_UPDATE)
		{
			xSemaphoreTake(g_xMutex_SystemMode, portMAX_DELAY);
		
			if(IR_KeyVal==IR_POWER && IR_RepeatFlag==0)
			{
				if(System_Mode == SysMODE_STBY)
				{
					System_Mode = SysMODE_NORM;
				}
				else
				{
					System_Mode = SysMODE_STBY;
				}
			}
			else if(IR_KeyVal==IR_MENU && IR_RepeatFlag==0)
			{
				if(System_Mode != SysMODE_SET)
				{
					System_Mode = SysMODE_SET;
					
					xEventGroupSetBits(g_xEventIR_Rec, IRRec_Event_W25Q64);
				}
			}

			xSemaphoreGive(g_xMutex_SystemMode);
		}
		
		if(System_Mode == SysMODE_SET)
		{
			xSemaphoreTake(g_xMutex_Set, portMAX_DELAY);
			
			if(IR_KeyVal==IR_START_STOP && IR_RepeatFlag==0)
			{
				xEventGroupSetBits(g_xEventIR_Rec, IRRec_Event_OLED_SetStart);
			}
			else if(IR_KeyVal==IR_BACK && IR_RepeatFlag==0)
			{
				xEventGroupSetBits(g_xEventIR_Rec, IRRec_Event_OLED_SetEnd);
			}
			else if(IR_KeyVal==IR_PREVIOUS && IR_RepeatFlag==0)
			{
				xEventGroupSetBits(g_xEventIR_Rec, IRRec_Event_OLED_SetPREVIOUS);
			}
			else if(IR_KeyVal==IR_NEXT && IR_RepeatFlag==0)
			{
				xEventGroupSetBits(g_xEventIR_Rec, IRRec_Event_OLED_SetNEXT);
			}
			else if(IR_KeyVal==IR_ADD && IR_RepeatFlag==0)
			{
				xEventGroupSetBits(g_xEventIR_Rec, IRRec_Event_OLED_SetSetPlus);
			}
			else if(IR_KeyVal==IR_SUB && IR_RepeatFlag==0)
			{
				xEventGroupSetBits(g_xEventIR_Rec, IRRec_Event_OLED_SetSub);
			}
			
			xSemaphoreGive(g_xMutex_Set);
		}
		else if(System_Mode == SysMODE_UPDATE)
		{
			if(IR_KeyVal==IR_PREVIOUS && IR_RepeatFlag==0)
			{
				xEventGroupSetBits(g_xEventIR_Rec, IRRec_Event_ESP01S_PREVIOUS);
			}
			else if(IR_KeyVal==IR_NEXT && IR_RepeatFlag==0)
			{
				xEventGroupSetBits(g_xEventIR_Rec, IRRec_Event_ESP01S_NEXT);
			}
			else if(IR_KeyVal==IR_START_STOP && IR_RepeatFlag==0)
			{
				xEventGroupSetBits(g_xEventIR_Rec, IRRec_Event_ESP01S_COFIRM);
			}
		}

		vTaskDelay(50);
	}
}
