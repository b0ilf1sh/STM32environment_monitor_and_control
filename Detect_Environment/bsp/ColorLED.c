#include "ColorLED.h"

void ColorLED_Init(void)
{
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
	ColorLED_SetColor(0, 0, 0);
}

void ColorLED_SetColor(uint8_t R, uint8_t G, uint8_t B)
{
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, R);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, G);
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, B);
}

void ColorLED_OFF(void)
{
	ColorLED_SetColor(0, 0, 255);
}

void ColorLED_Alarm(void)
{
	ColorLED_SetColor(255, 0, 0);
	vTaskDelay(100);
	ColorLED_SetColor(0, 0, 0);
	vTaskDelay(100);
}
