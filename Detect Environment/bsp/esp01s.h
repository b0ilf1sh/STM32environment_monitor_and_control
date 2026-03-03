#ifndef _ESP01S_H_
#define _ESP01S_H_

#include "hardware.h"
#include "stdio.h"
#include "string.h"

#define  Null_Mode      0
#define  Station_Mode   1
#define  SoftAP_Mode    2
#define  STAandAP_Mode  3

#define WIFI_SSID      "czh'sWIFI"
#define WIFI_PWD       "12345432"

#define client_id    "\"test\""
#define username     "\"2jKwfO4v2X\""
#define password     "\"version=2018-10-31&res=products%2F2jKwfO4v2X%2Fdevices%2Ftest&et=1800327473&method=md5&sign=izmSTkjpGu%2BkShY%2BpDyp2g%3D%3D\""

#define host     "\"mqtts.heclouds.com\""
#define port     1883

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

void esp01s_sendstring(char *string);
void esp01s_RX_IT_Start(void);
uint8_t esp01s_SetWifiMode(uint8_t WifiMode);
uint8_t esp01s_ConnectOnenet(void);
uint8_t esp01s_Init(void);
void esp01s_senddata(char *tag, uint32_t data);
void esp01s_send_normalmode_data(uint8_t hum, uint8_t tem, uint8_t light, uint8_t speed);
void esp01s_send_setmode_temdata(uint8_t tem1, uint8_t tem2);
void esp01s_send_setmode_volumedata(uint8_t volume1, uint8_t volume2);
uint8_t esp01s_getdata(char *tag, uint8_t *data);
uint8_t esp01s_gettemdata(uint8_t *tem1,uint8_t *tem2);
uint8_t esp01s_getvolumedata(uint8_t *volume1,uint8_t *volume2);
uint8_t Esp01s_GetConnectState(void);

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

#endif
