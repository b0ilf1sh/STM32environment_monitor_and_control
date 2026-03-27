#include "includes.h"

typedef enum{
	First_Line=0,
	Second_Line=16,
	Third_Line=32,
	Forth_Line=48,
}OLEDLine_t;

void task_oled(void *params)
{
	sysmode_t oled_state=SysMODE_STBY;
	
	EventBits_t dht11_event;
	uint8_t dht11_data[2]={0};
	
	EventBits_t lightsensor_event;
	uint8_t light_data=0;
	
	EventBits_t ir_rec_event;
	
	uint8_t motor_data;
	
	uint8_t ChangeModelu=0;
	uint8_t mode;
	OLEDLine_t select_module_y=Second_Line;
	OLEDLine_t Esp01s_y=Second_Line;
	uint8_t temp[3],tem[3],volume[3];
	tem[0]=1;
	volume[0]=2;
	uint8_t Radius=0;
	uint8_t tem_y=24,volume_y=24;
	uint8_t ChangeState=0;
	float Battery_Value;

	int16_t version_integer=0,version_decimals=0;//记录软件版本信息

	uint8_t version[8]={0};
    W25Q64_ReadPage(OTA_APP_LEN_ADDR+4, version, 8);
    version_integer = version[0] | (version[1]<<8) | (version[2]<<16) | (version[3]<<24);//整数部分
    version_decimals = version[4] | (version[5]<<8) | (version[6]<<16) | (version[7]<<24);//小数部分
	
	while(1)
	{
		OLED_Clear();
		
		if(oled_state != System_Mode)
		{
			if(System_Mode == SysMODE_SET)
			{
				mode = oled_state;
				ChangeModelu=0;
				select_module_y=Second_Line;
			}
			oled_state = System_Mode;
		}
		
		if(System_Mode == SysMODE_STBY)
		{
			OLED_ShowString(0, 0, "standy mode", OLED_8X16);
			OLED_Printf(0, 48, OLED_8X16, "V%d.%d",version_integer,version_decimals);
		}
		else if(System_Mode == SysMODE_NORM)
		{
			xTaskNotifyWait(0, 0, (uint32_t *)&motor_data, 0);
			
			dht11_event = xEventGroupWaitBits(g_xEventDHT11, DHT11_Event_OLED, pdTRUE, pdTRUE, 0);
			if(dht11_event & DHT11_Event_OLED)
			{
				vTaskSuspendAll();//暂停调度，防止接收到一半数据后跳转到别的任务，可能导致数据不是同时产生的
				dht11_data[0] = System_Data.dht11_hum;
				dht11_data[1] = System_Data.dht11_tem;
				xTaskResumeAll();
			}
			
			lightsensor_event = xEventGroupWaitBits(g_xEventLightSensor, LightSensor_Event_OLED, pdTRUE, pdTRUE, 0);
			if(lightsensor_event & LightSensor_Event_OLED)
			{
				light_data = System_Data.lightsensor_light;
			}
			

			OLED_Printf(0, 0, OLED_8X16, "hum:%d%%",dht11_data[0]);
			OLED_Printf(0, 16, OLED_8X16, "tem:%d",dht11_data[1]);
			
			OLED_Printf(0, 32, OLED_8X16, "light:%d%%",light_data);
			OLED_Printf(0, 48, OLED_8X16, "speed:%d%%",motor_data);
		}
		else if(System_Mode == SysMODE_SET)
		{
			ir_rec_event = xEventGroupWaitBits(g_xEventIR_Rec, IRRec_Event_OLED_ALL, pdTRUE, pdFALSE, 0);
			
			if(xQueueReceive(g_xQueue_W25Q64_to_OLED,temp,0)==pdPASS)
			{
				if(temp[0]==tem[0])
				{
					tem[1]=temp[1];
					tem[2]=temp[2];
				}
				else if(temp[0]==volume[0])
				{
					volume[1]=temp[1];
					volume[2]=temp[2];
				}
			}
				
			if(ChangeModelu == 0)
			{
				if(ir_rec_event & IRRec_Event_OLED_SetEnd)
				{
					System_Mode = mode ? SysMODE_NORM : SysMODE_STBY;
				}
				
				if(ir_rec_event & IRRec_Event_OLED_SetStart)
				{
					ChangeModelu=1;
					Esp01s_y = Second_Line;
					tem_y=24;
					volume_y=24;
					ChangeState=0;
					Radius=0;
				}
				
				if(ir_rec_event & IRRec_Event_OLED_SetNEXT)
				{
					select_module_y += 16;
				}
				
				if(ir_rec_event & IRRec_Event_OLED_SetPREVIOUS)
				{
					select_module_y -= 16;
				}
				
				if(select_module_y>48)
				{
					select_module_y=Second_Line;
				}
				else if(select_module_y<16)
				{
					select_module_y=Forth_Line;
				}
				
				OLED_ShowString(0, 0, "set mode", OLED_8X16);
				OLED_ShowString(0, 16, "tem set", OLED_8X16);
				OLED_ShowString(0, 32, "volume set", OLED_8X16);
				OLED_ShowString(0, 48, "ESP01S set", OLED_8X16);
				OLED_ReverseArea(0, select_module_y, 128, 16);
			}
			else if(ChangeModelu == 1)
			{
				if((ir_rec_event & IRRec_Event_OLED_SetStart) && select_module_y != Forth_Line)
				{
					ChangeState = !ChangeState;
					if(ChangeState == 0)
					{
						if(select_module_y == Second_Line)
						{
							xQueueSend(g_xQueue_OLED_to_W25Q64,tem,0);
						}
						else if(select_module_y == Third_Line)
						{
							xQueueSend(g_xQueue_OLED_to_W25Q64,volume,0);
						}
					}
				}
				
				if(select_module_y == Forth_Line)
				{
					if((ir_rec_event & IRRec_Event_OLED_SetEnd))
					{
						ChangeModelu=0;
					}
				}
				else
				{
					if((ir_rec_event & IRRec_Event_OLED_SetEnd) && ChangeState==0)
					{
						ChangeModelu=0;
					}
				}
				
				
				if(select_module_y == Second_Line)
				{
					if(ChangeState==0)
					{
						if((ir_rec_event & IRRec_Event_OLED_SetPREVIOUS) || (ir_rec_event & IRRec_Event_OLED_SetNEXT))
						{
							tem_y = (tem_y==24) ? 40 : 24;
						}
						
						Radius=4;
					}
					else if(ChangeState==1)
					{
						if(tem_y==24)
						{
							if(ir_rec_event & IRRec_Event_OLED_SetSetPlus)
							{
								tem[1]++;
							}
							
							if(ir_rec_event & IRRec_Event_OLED_SetSub)
							{
								tem[1]--;
							}
							
							if(tem[1] == 0)
							{
								tem[1]=1;
							}
							else if(tem[1]+1 >= tem[2])
							{
								tem[1] = tem[2]-2;
							}
						}
						else if(tem_y==40)
						{
							if(ir_rec_event & IRRec_Event_OLED_SetSetPlus)
							{
								tem[2]++;
							}
							
							if(ir_rec_event & IRRec_Event_OLED_SetSub)
							{
								tem[2]--;
							}
							
							if(tem[2]-1 <= tem[1]+1)
							{
								tem[2]=tem[1]+2;
							}
							else if(tem[2]+1 >= 50)
							{
								tem[2]=48;
							}
						}
						
						Radius = (Radius + 1) % 5;
					}
					OLED_ShowString(0, 0, "tem set", OLED_8X16);
					OLED_Printf(16, 16, OLED_8X16, "0 ~ %d",tem[1]);
					OLED_Printf(16, 32, OLED_8X16, "%d ~ %d",tem[1]+1,tem[2]);
					OLED_Printf(16, 48, OLED_8X16, "%d ~ 50",tem[2]+1);
					OLED_DrawCircle(8, tem_y, Radius, OLED_FILLED);
					
				}
				else if(select_module_y == Third_Line)
				{
					if(ChangeState==0)
					{
						if((ir_rec_event & IRRec_Event_OLED_SetPREVIOUS) || (ir_rec_event & IRRec_Event_OLED_SetNEXT))
						{
							volume_y = (volume_y==24) ? 40 : 24;
						}
						
						Radius=4;
					}
					else if(ChangeState==1)
					{
						if(volume_y==24)
						{
							if(ir_rec_event & IRRec_Event_OLED_SetSetPlus)
							{
								volume[1]++;
							}
							
							if(ir_rec_event & IRRec_Event_OLED_SetSub)
							{
								volume[1]--;
							}
							
							if(volume[1] == 255)
							{
								volume[1]=100;
							}								
							else if(volume[1] > 100)
							{
								volume[1]=0;
							}								
						}
						else if(volume_y==40)
						{
							if(ir_rec_event & IRRec_Event_OLED_SetSetPlus)
							{
								volume[2]++;
							}
							
							if(ir_rec_event & IRRec_Event_OLED_SetSub)
							{
								volume[2]--;
							}
							
							if(volume[2] == 255)
							{
								volume[2]=100;
							}								
							else if(volume[2] > 100)
							{
								volume[2]=0;
							}	
						}
						
						Radius = (Radius + 1) % 5;
					}
					OLED_ShowString(0, 0, "volume set", OLED_8X16);
					OLED_Printf(16, 16, OLED_8X16, "key volume:%d",volume[1]);
					OLED_Printf(16, 32, OLED_8X16, "alarm volume:%d",volume[2]);
					OLED_DrawCircle(8, volume_y, Radius, OLED_FILLED);
				}
				else if(select_module_y == Forth_Line)
				{
					if((ir_rec_event & IRRec_Event_OLED_SetPREVIOUS) || (ir_rec_event & IRRec_Event_OLED_SetNEXT))
					{
						Esp01s_y = (Esp01s_y==Second_Line) ? Third_Line : Second_Line;
					}
					
					if(Esp01s_y == Third_Line && (ir_rec_event & IRRec_Event_OLED_SetStart))
					{
						OLED_ShowString(0, 24, "reconnect...", OLED_8X16);
						OLED_Update();
						MQTT_CONNECT_FLAG = MQTT_Init();
						OLED_Clear();
					}
					
					OLED_ShowString(0, 0, "ESP01S set", OLED_8X16);
					OLED_ShowString(0, 16, "CONNECT?:", OLED_8X16);
					if(MQTT_CONNECT_FLAG == 0)
					{
						OLED_ShowString(72, 16, "yes", OLED_8X16);
					}
					else
					{
						OLED_ShowString(72, 16, "no", OLED_8X16);
					}
					OLED_ShowString(0, 32, "reconnect?", OLED_8X16);
					OLED_ReverseArea(0, Esp01s_y, 128, 16);
				}
			}
		}
		
		if(MQTT_CONNECT_FLAG == 0)
		{
			OLED_ShowImage(96, 0, 16, 16, WIFI);
		}
		
		Battery_Value = Battery_GetVoltage();
		
		if(Battery_Value >= 2.85)
		{
			OLED_ShowImage(112, 0, 16, 16, Battery_75to100);
		}
		else if(Battery_Value < 2.85 && Battery_Value >= 2.70)
		{
			OLED_ShowImage(112, 0, 16, 16, Battery_50to75);
		}
		else if(Battery_Value < 2.70 && Battery_Value >= 2.55)
		{
			OLED_ShowImage(112, 0, 16, 16, Battery_25to50);
		}
		else if(Battery_Value < 2.55)
		{
			OLED_ShowImage(112, 0, 16, 16, Battery_0to25);
		}
		
		OLED_Update();
		
		vTaskDelay(10);
	}
}
