#include "esp01s.h"

char esp01s_txtext[256];
char esp01s_rxtext[128];
char esp01s_datatext[128];
uint8_t data_flag=0;
uint8_t esp01s_datapointer=0;
uint8_t esp01s_rxpointer=0;
uint8_t rxdata;
uint8_t esp01s_connectstate=0;

void esp01s_sendstring(char *string)
{
	HAL_UART_Transmit(&huart2, (const uint8_t *)string, strlen(string), HAL_MAX_DELAY);
}

void esp01s_RX_IT_Start(void)
{
	HAL_UART_Receive_IT(&huart2, &rxdata, 1);
}

void esp01s_ClearRxData(void)
{
	memset(esp01s_rxtext, 0, sizeof(esp01s_rxtext)); 
	esp01s_rxpointer=0;
}

uint8_t esp01s_process(void)
{
	if(strstr((char *)esp01s_rxtext, "OK"))
	{
		return 1;
	}
	else if(strstr((char *)esp01s_rxtext, "ERROR"))
	{
		return 0;
	}
	return 0;
}

uint8_t esp01s_Reset(void)
{
	uint16_t t=20000;
	esp01s_ClearRxData();
	esp01s_sendstring("AT+RST\r\n");
	while(t)
	{
		if(esp01s_process()) break;
		t--;
	}
	if(t>0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t esp01s_Test(void)
{
	uint16_t t=5000;
	esp01s_ClearRxData();
	esp01s_sendstring("AT\r\n");
	while(t)
	{
		if(esp01s_process()) break;
		t--;
	}
	if(t>0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t esp01s_CloseEcho(void)
{
	uint16_t t=5000;
	esp01s_ClearRxData();
	esp01s_sendstring("ATE0\r\n");
	while(t)
	{
		if(esp01s_process()) break;
		t--;
	}
	if(t>0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t esp01s_SetWifiMode(uint8_t WifiMode)
{
	uint16_t t=5000;
	esp01s_ClearRxData();
	sprintf(esp01s_txtext,"AT+CWMODE=%d\r\n",WifiMode);
	esp01s_sendstring(esp01s_txtext);
	while(t)
	{
		if(esp01s_process()) break;
		t--;
	}
	if(t>0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t esp01s_ConnectWifi(void)
{
	uint16_t t=50000;
	esp01s_ClearRxData();
	sprintf(esp01s_txtext,"AT+CWJAP=\"%s\",\"%s\"\r\n",WIFI_SSID,WIFI_PWD);
	esp01s_sendstring(esp01s_txtext);
	while(t)
	{
		if(esp01s_process()) break;
		t--;
	}
	if(t>0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t esp01s_ConnectOnenet(void)
{
	uint16_t t=5000;
	esp01s_ClearRxData();
	sprintf(esp01s_txtext,"AT+MQTTUSERCFG=0,1,%s,%s,%s,0,0,\"\"\r\n",client_id,username,password);//设置用户配置
	esp01s_sendstring(esp01s_txtext);
	while(t)
	{
		if(esp01s_process()) break;
		t--;
	}
	if(t>0)
	{
		printf("Set UserConfig OK\r\n");
	}
	else
	{
		printf("Set UserConfig Err\r\n");
		return 1;
	}
	
	t=60000;
	esp01s_ClearRxData();
	sprintf(esp01s_txtext,"AT+MQTTCONN=0,%s,%d,0\r\n",host,port);//连接服务器代理
	esp01s_sendstring(esp01s_txtext);
	while(t)
	{
		if(esp01s_process()) break;
		t--;
	}
	if(t>0)
	{
		printf("Connect to MQTT Broker OK\r\n");
	}
	else
	{
		printf("Connect to MQTT Broker Err\r\n");
		return 2;
	}
	
	return 0;
}

uint8_t esp01s_Onenet_PostandSet(void)
{
	uint16_t t=5000;
	esp01s_ClearRxData();
	sprintf(esp01s_txtext,"AT+MQTTSUB=0,%s,0\r\n",Post_Topic);//订阅Post
	esp01s_sendstring(esp01s_txtext);
	while(t)
	{
		if(esp01s_process()) break;
		t--;
	}
	if(t>0)
	{
		printf("Onenet Post OK\r\n");
	}
	else
	{
		printf("Onenet Post Err\r\n");
		return 1;
	}
	
	t=20000;
	esp01s_ClearRxData();
	sprintf(esp01s_txtext,"AT+MQTTSUB=0,%s,0\r\n",Set_Topic);//订阅Set
	esp01s_sendstring(esp01s_txtext);
	while(t)
	{
		if(esp01s_process()) break;
		t--;
	}
	if(t>0)
	{
		printf("Onenet Set OK\r\n");
	}
	else
	{
		printf("Onenet Set Err\r\n");
		return 1;
	}
	
	return 0;
}

uint8_t esp01s_Init(void)
{
	esp01s_RX_IT_Start();//开启esp01s的串口接收中断
	
	if(esp01s_Reset()==0)//测试esp01s是否启动成功
	{
		printf("Reset Esp Err\r\n");
		return 0;
	}
	printf("Reset Esp OK\r\n");
	
	mdelay(1000);
	esp01s_ClearRxData(); 
	memset(esp01s_datatext, 0, sizeof(esp01s_datatext)); 
	esp01s_datapointer=0;
	
	if(esp01s_Test()==0)//测试esp01s是否启动成功
	{
		printf("Find Esp Err\r\n");
		return 0;
	}
	printf("Find Esp OK\r\n");

	if(esp01s_CloseEcho()==0)//esp01s关闭回显
	{
		printf("Close Echo Err\r\n");
		return 0;
	}
	printf("Close Echo OK\r\n");
	
	if(esp01s_SetWifiMode(Station_Mode)==0)//选择esp01sWIFI模式
	{
		printf("Set Wifi-Mode Err\r\n");
		return 0;
	}
	printf("Set Wifi-Mode OK\r\n");
	
	if(esp01s_ConnectWifi()==0)//esp01s连接WIFI
	{
		printf("Connect Wifi Err\r\n");
		return 0;
	}
	printf("Connect Wifi OK\r\n");
	
	if(esp01s_ConnectOnenet()!=0)
	{
		return 0;
	}
	printf("Connect Onenet OK\r\n");
	
	if(esp01s_Onenet_PostandSet()!=0)
	{
		return 0;
	}
	printf("Sub Onenet OK\r\n");
	
	printf("esp01s init OK\r\n");
	esp01s_connectstate = 1;
	
	return 1;
}

void esp01s_senddata(char *tag, uint32_t data)
{
	sprintf(esp01s_txtext,"AT+MQTTPUB=0,%s,"
						"\"{"
							"\\\"id\\\":\\\"123\\\"\\\,"
							"\\\"params\\\":{"
										 "%s:{\\\"value\\\":%d\\\}"
										"}"
                            "}\""
	       ",0,0\r\n",Post_Topic,tag,data);//发送信息
	esp01s_sendstring(esp01s_txtext);
	osDelay(10);
//	printf(esp01s_rxtext);
//	printf("\r\n");
	if(esp01s_process())
	{
		esp01s_ClearRxData();
	}
	osDelay(100);
}

void esp01s_send_normalmode_data(uint8_t hum, uint8_t tem, uint8_t light, uint8_t speed)
{
	sprintf(esp01s_txtext,"AT+MQTTPUB=0,%s,"
						"\"{"
							"\\\"id\\\":\\\"123\\\"\\\,"
							"\\\"params\\\":{"
										 "%s:{\\\"value\\\":%d\\\}\\\,"
	                                     "%s:{\\\"value\\\":%d\\\}\\\,"
	                                     "%s:{\\\"value\\\":%d\\\}\\\,"
	                                     "%s:{\\\"value\\\":%d\\\}"
										"}"
                            "}\""
	       ",0,0\r\n",Post_Topic,APP_HUM,hum,APP_TEM,tem,APP_BRT,light,APP_SPEED,speed);//发送信息
	esp01s_sendstring(esp01s_txtext);
	osDelay(5);
//	printf(esp01s_rxtext);
//	printf("\r\n");
	if(esp01s_process())
	{
		esp01s_ClearRxData();
	}
	osDelay(100);
}

void esp01s_send_setmode_temdata(uint8_t tem1, uint8_t tem2)
{
	sprintf(esp01s_txtext,"AT+MQTTPUB=0,%s,"
						"\"{"
							"\\\"id\\\":\\\"123\\\"\\\,"
							"\\\"params\\\":{"
										 "%s:{\\\"value\\\":%d\\\}\\\,"
	                                     "%s:{\\\"value\\\":%d\\\}"
										"}"
                            "}\""
	       ",0,0\r\n",Post_Topic,APP_TEM1,tem1,APP_TEM2,tem2);//发送信息
	esp01s_sendstring(esp01s_txtext);
	osDelay(5);
//	printf(esp01s_rxtext);
//	printf("\r\n");
	if(esp01s_process())
	{
		esp01s_ClearRxData();
	}
	osDelay(100);
}

void esp01s_send_setmode_volumedata(uint8_t volume1, uint8_t volume2)
{
	sprintf(esp01s_txtext,"AT+MQTTPUB=0,%s,"
						"\"{"
							"\\\"id\\\":\\\"123\\\"\\\,"
							"\\\"params\\\":{"
										 "%s:{\\\"value\\\":%d\\\}\\\,"
	                                     "%s:{\\\"value\\\":%d\\\}"
										"}"
                            "}\""
	       ",0,0\r\n",Post_Topic,APP_VOLUME1,volume1,APP_VOLUME2,volume2);//发送信息
	esp01s_sendstring(esp01s_txtext);
	osDelay(5);
//	printf(esp01s_rxtext);
//	printf("\r\n");
	if(esp01s_process())
	{
		esp01s_ClearRxData();
	}
	osDelay(100);
}

uint8_t esp01s_getdata(char *tag, uint8_t *data)
{
	char *data_ptr = strstr((char *)esp01s_datatext, tag);
	if(data_ptr)
	{
		if(sscanf(data_ptr+strlen(tag)+1,"%u",data))
		{
//			printf(esp01s_datatext);
//			printf("\r\n");
			memset(esp01s_datatext, 0, sizeof(esp01s_datatext)); 
			esp01s_datapointer=0;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
//		printf("err\r\n");
		return 0;
	}
}

uint8_t esp01s_gettemdata(uint8_t *tem1,uint8_t *tem2)
{
	char *data_ptr = strstr((char *)esp01s_datatext, APP_TEM1_Get);
	if(data_ptr)
	{
		if(sscanf(data_ptr+strlen(APP_TEM1_Get)+1,"%u",tem1))
		{
			data_ptr = strstr((char *)esp01s_datatext, APP_TEM2_Get);
			if(data_ptr)
			{
				if(sscanf(data_ptr+strlen(APP_TEM2_Get)+1,"%u",tem2))
				{
//					printf(esp01s_datatext);
//					printf("\r\n");
					memset(esp01s_datatext, 0, sizeof(esp01s_datatext)); 
					esp01s_datapointer=0;
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
//		printf("err\r\n");
		return 0;
	}
}

uint8_t esp01s_getvolumedata(uint8_t *volume1,uint8_t *volume2)
{
	char *data_ptr = strstr((char *)esp01s_datatext, APP_VOLUME1_Get);
	if(data_ptr)
	{
		if(sscanf(data_ptr+strlen(APP_VOLUME1_Get)+1,"%u",volume1))
		{
			data_ptr = strstr((char *)esp01s_datatext, APP_VOLUME2_Get);
			if(data_ptr)
			{
				if(sscanf(data_ptr+strlen(APP_VOLUME2_Get)+1,"%u",volume2))
				{
//					printf(esp01s_datatext);
//					printf("\r\n");
					memset(esp01s_datatext, 0, sizeof(esp01s_datatext)); 
					esp01s_datapointer=0;
					return 1;
				}
				else
				{
					return 0;
				}
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	else
	{
//		printf("err\r\n");
		return 0;
	}
}

uint8_t Esp01s_GetConnectState(void)
{
	if(strstr((char *)esp01s_rxtext, "WIFI DISCONNECT") || strstr((char *)esp01s_rxtext, "MQTTDISCONNECTED"))
	{
		esp01s_ClearRxData();
		esp01s_connectstate=0;
	}
	return esp01s_connectstate;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
//	static uint8_t s=0;
	if(huart==&huart2)
	{
		if(rxdata == '{')
		{
			data_flag=1;
		}
		else if(rxdata == '}')
		{
			data_flag=0;
		}
		
		if(data_flag == 0)
		{
			esp01s_rxtext[esp01s_rxpointer]=rxdata;
			esp01s_rxpointer++;
			if(esp01s_rxpointer>=sizeof(esp01s_rxtext))
			{
				esp01s_rxpointer=0;
			}
		}
		else if(data_flag == 1)
		{
			esp01s_datatext[esp01s_datapointer]=rxdata;
			esp01s_datapointer++;
			if(esp01s_datapointer>=sizeof(esp01s_datatext))
			{
				esp01s_datapointer=0;
			}
		}

		esp01s_RX_IT_Start();
	}
}
