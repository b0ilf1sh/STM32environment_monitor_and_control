#ifndef _INCLUDES_H_
#define _INCLUDES_H_

#include "hardware.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h" 
#include "event_groups.h" 
#include "queue.h"
#include "semphr.h"

#define IRRec_Event_Buzzer        (1<<0)
#define IRRec_Event_W25Q64        (1<<1)
#define IRRec_Event_OLED_SetStart     (1<<2)
#define IRRec_Event_OLED_SetEnd       (1<<3)
#define IRRec_Event_OLED_SetPREVIOUS  (1<<4)
#define IRRec_Event_OLED_SetNEXT      (1<<5)
#define IRRec_Event_OLED_SetSetPlus   (1<<6)
#define IRRec_Event_OLED_SetSub       (1<<7)
#define IRRec_Event_OLED_ALL     (IRRec_Event_OLED_SetStart | IRRec_Event_OLED_SetEnd \
							    | IRRec_Event_OLED_SetPREVIOUS | IRRec_Event_OLED_SetNEXT \
							    | IRRec_Event_OLED_SetSetPlus | IRRec_Event_OLED_SetSub)

#define ESP01S_Event_Buzzer       (1<<0)
#define ESP01S_Event_W25Q64       (1<<1)

#define DHT11_Event_OLED          (1<<0)
#define DHT11_Event_ESP01S        (1<<1)
#define DHT11_Event_Motor         (1<<2)
#define DHT11_Event_All	   (DHT11_Event_OLED | DHT11_Event_ESP01S | DHT11_Event_Motor)

#define LightSensor_Event_OLED      (1<<0)
#define LightSensor_Event_ColorLED  (1<<1)
#define LightSensor_Event_ESP01S    (1<<2)
#define LightSensor_Event_All   (LightSensor_Event_OLED | LightSensor_Event_ColorLED | LightSensor_Event_ESP01S)

#define W25Q64_TEMP_ADDRESS      0x002000    //温度存储地址
#define W25Q64_VOLUME_ADDRESS    0x003000    //音量存储地址

typedef enum{
	SysMODE_STBY=0,
	SysMODE_NORM=1,
	SysMODE_SET=2,
}sysmode_t;

typedef struct{
	uint8_t dht11_hum;
	uint8_t dht11_tem;
	uint8_t lightsensor_light;
}sysdata_t;

//添加全局变量
extern TaskHandle_t g_TaskHandleBuzzer;
extern TaskHandle_t g_TaskHandleOLED;
extern TaskHandle_t g_TaskHandleColorLED;
extern TaskHandle_t g_TaskHandleESP01S;

extern EventGroupHandle_t g_xEventIR_Rec;
extern EventGroupHandle_t g_xEventESP01S;
extern EventGroupHandle_t g_xEventDHT11;
extern EventGroupHandle_t g_xEventLightSensor;

extern QueueHandle_t g_xQueue_W25Q64_to_Motor;
extern QueueHandle_t g_xQueue_W25Q64_to_Buzzer;
extern QueueHandle_t g_xQueue_W25Q64_to_OLED;
extern QueueHandle_t g_xQueue_W25Q64_to_ESP01S;
extern QueueHandle_t g_xQueue_OLED_to_W25Q64;
extern QueueHandle_t g_xQueue_ESP01S_to_W25Q64;

extern SemaphoreHandle_t g_xMutex_SystemMode;
extern SemaphoreHandle_t g_xMutex_Set;

extern sysmode_t System_Mode;
extern sysdata_t System_Data;
extern uint8_t esp01s_flag;

//初始化任务函数
void app_init(void);
//模块任务函数
void task_esp01s(void *params);
void task_buzzer(void *params);
void task_lightsensor(void *params);
void task_dht11(void *params);
void task_colorled(void *params);
void task_ir_rec(void *params);
void task_oled(void *params);
void task_w25q64(void *params);
void task_motor(void *params);

#endif
