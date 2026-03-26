#include "Motor.h"

void Motor_Init(void)
{
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
}

//Speed范围在0到100之间
void Motor_SetSpeed(uint8_t Speed)
{
	if(Speed > 100)
	{
		Speed = 100;
	}
	
	Speed = Speed * 255 /100;
	__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_4, Speed);
	if(Speed)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	}
}
