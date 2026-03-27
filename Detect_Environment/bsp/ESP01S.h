#ifndef _ESP01S_H_
#define _ESP01S_H_

#include "main.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h" 
#include "driver_timer.h"

#define ESP01S_REC_BUFF_LEN  1536
#define ESP01S_READ_BUFF_LEN   512
#define ESP01S_REC_RING_BUFF_LEN  2048
#define ESP01S_MQTT_REC_BUFF_LEN 128

#define WIFI_SSID      "czh'sWIFI"
#define WIFI_PWD       "12345432"

#define tcp_host     "\"iot-api.heclouds.com\""
#define tcp_port     80

#define mqtt_host     "\"mqtts.heclouds.com\""
#define mqtt_port     1883

#define tcp_client_id    "test"
#define tcp_username     "2jKwfO4v2X"
#define tcp_password     "version=2018-10-31&res=products%2F2jKwfO4v2X%2Fdevices%2Ftest&et=1800327473&method=md5&sign=izmSTkjpGu%2BkShY%2BpDyp2g%3D%3D"

#define mqtt_client_id    "\"test\""
#define mqtt_username     "\"2jKwfO4v2X\""
#define mqtt_password     "\"version=2018-10-31&res=products%2F2jKwfO4v2X%2Fdevices%2Ftest&et=1800327473&method=md5&sign=izmSTkjpGu%2BkShY%2BpDyp2g%3D%3D\""

extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern uint8_t MQTT_ConnectState;
extern uint8_t ESP01S_PROTOCOL_FLAG;//用来确定当前以什么协议连接ONENET，0为OTA，1为MQTT
extern uint8_t OTA_UPDATE_STATE;

void ESP01S_Printf(const char* format, ...);
void ESP01S_CLEAR_READ_BUFF(void);
void ESP01S_UART_Init(void);
uint8_t ESP01S_WaitAck(char *string, uint16_t timeout_ms);
void ESP01S_INFORMATION(void);
uint8_t ESP01S_Init(void);

#endif
