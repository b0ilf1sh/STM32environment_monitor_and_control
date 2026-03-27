#include "MQTT.h"

uint8_t MQTT_ConnectState=1;//用来确定是否有连接到WIFI或者服务器，0为连接上，1为未连接上

//连接MQTT服务器
uint8_t MQTT_CONNECT(void)
{
	//MQTT用户配置
	ESP01S_CLEAR_READ_BUFF();
    ESP01S_Printf("AT+MQTTUSERCFG=0,1,%s,%s,%s,0,0,\"\"\r\n",mqtt_client_id,mqtt_username,mqtt_password);
    if(ESP01S_WaitAck("OK", 5000)==1)
    {
        printf("MQTT USE CONFIG OK\r\n");
    }
    else
    {
        printf("MQTT USE CONFIG ERROR\r\n");
        return 1;
    }

	//连接MQTT服务器
	ESP01S_CLEAR_READ_BUFF();
	ESP01S_Printf("AT+MQTTCONN=0,%s,%d,0\r\n",mqtt_host,mqtt_port);
	if(ESP01S_WaitAck("OK", 5000)==1)
    {
        printf("MQTT CONNECT OK\r\n");
    }
    else
    {
        printf("MQTT CONNECT ERROR\r\n");
        return 2;
    }

	//设备上报数据
	ESP01S_CLEAR_READ_BUFF();
	ESP01S_Printf("AT+MQTTSUB=0,%s,0\r\n",Post_Topic);
	if(ESP01S_WaitAck("OK", 5000)==1)
    {
        printf("MQTT POST OK\r\n");
    }
    else
    {
        printf("MQTT POST ERROR\r\n");
        return 3;
    }

	//设备接收数据
	ESP01S_CLEAR_READ_BUFF();
	ESP01S_Printf("AT+MQTTSUB=0,%s,0\r\n",Set_Topic);
	if(ESP01S_WaitAck("OK", 5000)==1)
    {
        printf("MQTT SET OK\r\n");
    }
    else
    {
        printf("MQTT SET ERROR\r\n");
        return 4;
    }

	return 0;
}

//MQTT连接ONENET初始化
uint8_t MQTT_Init(void)
{
	ESP01S_REC_BUFF_FLAG = 0;//改变接收数据存储的数组为TCP的数组
	//ESP01S初始化
	if(ESP01S_Init())
    {
        printf("ESP01S INIT ERROR\r\n");
		MQTT_ConnectState = 1;//WIFI和服务器连接失败
        return 1;
    }

	//MQTT连接ONENET
	if(MQTT_CONNECT())
	{
		printf("MQTT CONNECT ERROR\r\n");
		MQTT_ConnectState = 1;//WIFI和服务器连接失败
		return 2;
	}

	printf("MQTT INIT OK\r\n");
	MQTT_ConnectState = 0;//WIFI和服务器连接成功
	ESP01S_REC_BUFF_FLAG = 1;//改变接收数据存储的数组为MQTT的数组
	return 0;
}

//清空接收数据
void MQTT_Clear_RecBuff(void)
{
	memset(ESP01S_MQTT_REC_BUFF, 0, ESP01S_MQTT_REC_BUFF_LEN);
}

//检测ESP01S是否连接上WIFI或者服务器
uint8_t MQTT_GET_CONNECT_STATE(void)
{
	return MQTT_ConnectState;
}

//发送单个数据
void MQTT_SEND_DATA(char *tag, uint32_t data)
{
	ESP01S_Printf("AT+MQTTPUB=0,%s,"
						"\"{"
							"\\\"id\\\":\\\"123\\\"\\,"
							"\\\"params\\\":{"
										     "%s:{\\\"value\\\":%d\\}"
										   "}"
                          "}\""
	       ",0,0\r\n",Post_Topic,tag,data);//发送正常模式下的信息
	//延时10ms
	vTaskDelay(10);
}

//发送正常模式下的数据
void MQTT_SEND_NORMAL_MODE_DATA(uint8_t hum, uint8_t tem, uint8_t light, uint8_t speed)
{
 	ESP01S_Printf("AT+MQTTPUB=0,%s,"
						"\"{"
							"\\\"id\\\":\\\"123\\\"\\,"
							"\\\"params\\\":{"
										     "%s:{\\\"value\\\":%d\\}\\,"
	                                     	 "%s:{\\\"value\\\":%d\\}\\,"
	                                         "%s:{\\\"value\\\":%d\\}\\,"
	                                         "%s:{\\\"value\\\":%d\\}"
										   "}"
                          "}\""
	       ",0,0\r\n",Post_Topic,APP_HUM,hum,APP_TEM,tem,APP_BRT,light,APP_SPEED,speed);//发送正常模式下的信息
	//延时10ms
	vTaskDelay(10);
}

//发送设置模式的温度信息
void MQTT_SEND_SetMode_TemData(uint8_t tem1, uint8_t tem2)
{
	ESP01S_Printf("AT+MQTTPUB=0,%s,"
						"\"{"
							"\\\"id\\\":\\\"123\\\"\\,"
							"\\\"params\\\":{"
										     "%s:{\\\"value\\\":%d\\}\\,"
	                                         "%s:{\\\"value\\\":%d\\}"
										   "}"
                          "}\""
	       ",0,0\r\n",Post_Topic,APP_TEM1,tem1,APP_TEM2,tem2);//发送设置的温度信息
	//延时10ms
	vTaskDelay(10);
}

//发送设置模式的音量信息
void MQTT_SEND_SetMode_VolumeData(uint8_t volume1, uint8_t volume2)
{
	ESP01S_Printf("AT+MQTTPUB=0,%s,"
						"\"{"
							"\\\"id\\\":\\\"123\\\"\\,"
							"\\\"params\\\":{"
										 "%s:{\\\"value\\\":%d\\}\\,"
	                                     "%s:{\\\"value\\\":%d\\}"
										"}"
                            "}\""
	       ",0,0\r\n",Post_Topic,APP_VOLUME1,volume1,APP_VOLUME2,volume2);//发送设置的音量信息
	//延时10ms
	vTaskDelay(10);
}

//获取单个数据
uint8_t MQTT_GET_DATA(char *tag, uint8_t *data)
{
	char *data_ptr = strstr((char *)ESP01S_MQTT_REC_BUFF, tag);
	if(data_ptr)
	{
		if(sscanf(data_ptr+strlen(tag)+1,"%hhu",data))
		{
			MQTT_Clear_RecBuff();
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

//获取温度设置信息
uint8_t MQTT_GET_TemData(uint8_t *tem1,uint8_t *tem2)
{
	char *data_ptr = strstr((char *)ESP01S_MQTT_REC_BUFF, APP_TEM1_Get);
	if(data_ptr)
	{
		if(sscanf(data_ptr+strlen(APP_TEM1_Get)+1,"%hhu",tem1))
		{
			data_ptr = strstr((char *)ESP01S_MQTT_REC_BUFF, APP_TEM2_Get);
			if(data_ptr)
			{
				if(sscanf(data_ptr+strlen(APP_TEM2_Get)+1,"%hhu",tem2))
				{
					MQTT_Clear_RecBuff();
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
		return 0;
	}
}

//获取音量设置信息
uint8_t MQTT_GET_VolumeData(uint8_t *volume1,uint8_t *volume2)
{
	char *data_ptr = strstr((char *)ESP01S_MQTT_REC_BUFF, APP_VOLUME1_Get);
	if(data_ptr)
	{
		if(sscanf(data_ptr+strlen(APP_VOLUME1_Get)+1,"%hhu",volume1))
		{
			data_ptr = strstr((char *)ESP01S_MQTT_REC_BUFF, APP_VOLUME2_Get);
			if(data_ptr)
			{
				if(sscanf(data_ptr+strlen(APP_VOLUME2_Get)+1,"%hhu",volume2))
				{
					MQTT_Clear_RecBuff();
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
		return 0;
	}
}

