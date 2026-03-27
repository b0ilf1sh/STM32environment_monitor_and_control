#include "includes.h"

TaskHandle_t g_TaskHandleBuzzer;
TaskHandle_t g_TaskHandleOLED;
TaskHandle_t g_TaskHandleColorLED;
TaskHandle_t g_TaskHandleESP01S;

EventGroupHandle_t g_xEventIR_Rec;//红外接收事件组
EventGroupHandle_t g_xEventESP01S;//ESP01S事件组
EventGroupHandle_t g_xEventDHT11;//DHT11事件组
EventGroupHandle_t g_xEventLightSensor;//LightSensor事件组

QueueHandle_t g_xQueue_W25Q64_to_Motor;//W25Q64给Motor数据的消息队列
QueueHandle_t g_xQueue_W25Q64_to_Buzzer;
QueueHandle_t g_xQueue_W25Q64_to_OLED;
QueueHandle_t g_xQueue_W25Q64_to_ESP01S;
QueueHandle_t g_xQueue_OLED_to_W25Q64;
QueueHandle_t g_xQueue_ESP01S_to_W25Q64;

SemaphoreHandle_t g_xMutex_SystemMode;
SemaphoreHandle_t g_xMutex_Set;

sysmode_t System_Mode=SysMODE_STBY;
sysdata_t System_Data;
uint8_t MQTT_CONNECT_FLAG=1;//MQTT连接标志位，0为连接，1为未连接
uint8_t temp[3];

void app_init(void)
{
	//事件组等全局变量
	g_xEventIR_Rec = xEventGroupCreate();
	g_xEventESP01S = xEventGroupCreate();
	g_xEventDHT11 = xEventGroupCreate();
	g_xEventLightSensor = xEventGroupCreate();
	
	g_xQueue_W25Q64_to_Motor = xQueueCreate(1, 3);
	g_xQueue_W25Q64_to_Buzzer = xQueueCreate(1, 3);
	g_xQueue_W25Q64_to_OLED = xQueueCreate(2, 3);
	g_xQueue_W25Q64_to_ESP01S = xQueueCreate(2, 3);
	g_xQueue_OLED_to_W25Q64 = xQueueCreate(2, 3);
	g_xQueue_ESP01S_to_W25Q64 = xQueueCreate(2, 3);
	
	g_xMutex_SystemMode = xSemaphoreCreateMutex();
	g_xMutex_Set = xSemaphoreCreateMutex();
	
	//外设初始化
	OLED_Init();
	OLED_ShowString(0, 0, "INIT START", OLED_8X16);
	OLED_Update();
	mdelay(1000);
	
	taskENTER_CRITICAL();
	DHT11_Init();
	taskEXIT_CRITICAL();
	
	BUZZER_Init();
	ColorLED_Init();
	IR_Init();
	Motor_Init();
	W25Q64_Init();
	AD_Init();
	
	mdelay(1000);
	
	W25Q64_ReadPage(W25Q64_TEMP_ADDRESS, temp, sizeof(temp));
	xQueueSend(g_xQueue_W25Q64_to_Motor,temp,0);
	W25Q64_ReadPage(W25Q64_VOLUME_ADDRESS, temp, sizeof(temp));
	xQueueSend(g_xQueue_W25Q64_to_Buzzer,temp,0);
	
	OLED_Clear();
	OLED_ShowString(0, 0, "INIT END", OLED_8X16);
	OLED_Update();
	mdelay(1000);
	
	OLED_Clear();
	OLED_Update();
	
	//创建任务
	xTaskCreate(task_ir_rec, "IRRec_Task", 100, NULL, osPriorityNormal3, NULL);
	xTaskCreate(task_esp01s, "Esp01s_Task", 256, NULL, osPriorityNormal3, &g_TaskHandleESP01S);
	
	xTaskCreate(task_dht11, "DHT11_Task", 100, NULL, osPriorityNormal2, NULL);
	xTaskCreate(task_lightsensor, "LightSensor_Task", 100, NULL, osPriorityNormal2, NULL);
	
	xTaskCreate(task_buzzer, "Buzzer_Task", 100, NULL, osPriorityNormal1, &g_TaskHandleBuzzer);
	xTaskCreate(task_colorled, "ColorLED_Task", 100, NULL, osPriorityNormal1, &g_TaskHandleColorLED);
	xTaskCreate(task_motor, "Motor_Task", 100, NULL, osPriorityNormal1, NULL);
	
	xTaskCreate(task_oled, "OLED_Task", 200, NULL, osPriorityNormal, &g_TaskHandleOLED);
	xTaskCreate(task_w25q64, "W25Q64_Task", 100, NULL, osPriorityNormal, NULL);
}
