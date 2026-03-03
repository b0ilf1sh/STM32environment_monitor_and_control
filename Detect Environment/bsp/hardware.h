#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#include "main.h"
#include "cmsis_os2.h"
#include "includes.h"

//添加工程需要用到的模块的头文件
#include "IR.h"
#include "DHT11.h"
#include "BUZZER.h"
#include "music.h"
#include "LightSensor.h"
#include "ColorLED.h"
#include "OLED.h"
#include "driver_timer.h"
#include "GPIO_IRQ.h"
#include "w25q64.h"
#include "Motor.h"
#include "esp01s.h"
#include "AD.h"
#include "Battery.h"

//外部变量声明
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

extern SPI_HandleTypeDef hspi2;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

#endif
