#include "BUZZER.h"

void BUZZER_Init(void)
{
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	BUZZER_OFF();
}

void BUZZER_SetVolume(uint8_t Volume)
{
	__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, Volume);
}

uint8_t BUZZER_GetVolume(void)
{
	return __HAL_TIM_GetCompare(&htim2, TIM_CHANNEL_2);
}

void BUZZER_OFF(void)
{	
	BUZZER_SetVolume(0);
}

void BUZZER_Music(uint8_t Volume, uint64_t Pre, uint64_t time)
{
	if(Pre)
	{
		__HAL_TIM_SET_PRESCALER(&htim2, Pre);
		BUZZER_SetVolume(Volume);
		vTaskDelay(time);
		BUZZER_OFF();
	}
	else
	{
		BUZZER_OFF();
		vTaskDelay(time);
	}
	//mdelay(5);	
}

void Buzzer_HintVoice(uint8_t volumn)
{
	BUZZER_Music(volumn, M1, 100);
	BUZZER_Music(volumn, M6, 100);
}

void Buzzer_Alarm(uint8_t volumn)
{	
	BUZZER_Music(volumn, M6, 500);
	BUZZER_Music(volumn, M4, 500);
}
