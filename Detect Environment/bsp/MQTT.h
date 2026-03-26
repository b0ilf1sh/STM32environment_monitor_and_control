#ifndef _MQTT_H_
#define _MQTT_H_

#include "main.h"
#include "stdio.h"
#include "string.h"
#include "ESP01S.h"
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h" 

#define Post_Topic    "\"$sys/2jKwfO4v2X/test/thing/property/post\""
#define Set_Topic     "\"$sys/2jKwfO4v2X/test/thing/property/set\""

#define APP_HUM          "\\\"hum\\\""
#define APP_TEM          "\\\"tem\\\""
#define APP_BRT          "\\\"brt\\\""
#define APP_SPEED        "\\\"speed\\\""
#define APP_MODE_Send    "\\\"mode\\\""
#define APP_MODE_Get     "\"mode\""
#define APP_TEM1         "\\\"tem1\\\""
#define APP_TEM2		 "\\\"tem2\\\""
#define APP_VOLUME1      "\\\"volume1\\\""
#define APP_VOLUME2      "\\\"volume2\\\""
#define APP_TEM1_Get         "\"tem1\""
#define APP_TEM2_Get		 "\"tem2\""
#define APP_VOLUME1_Get      "\"volume1\""
#define APP_VOLUME2_Get      "\"volume2\""

extern uint8_t ESP01S_MQTT_REC_BUFF[ESP01S_MQTT_REC_BUFF_LEN];
extern uint8_t ESP01S_REC_BUFF_FLAG;

uint8_t MQTT_Init(void);
uint8_t MQTT_GET_CONNECT_STATE(void);
void MQTT_SEND_DATA(char *tag, uint32_t data);
void MQTT_SEND_NORMAL_MODE_DATA(uint8_t hum, uint8_t tem, uint8_t light, uint8_t speed);
void MQTT_SEND_SetMode_TemData(uint8_t tem1, uint8_t tem2);
void MQTT_SEND_SetMode_VolumeData(uint8_t volume1, uint8_t volume2);
uint8_t MQTT_GET_DATA(char *tag, uint8_t *data);
uint8_t MQTT_GET_TemData(uint8_t *tem1,uint8_t *tem2);
uint8_t MQTT_GET_VolumeData(uint8_t *volume1,uint8_t *volume2);

#endif
