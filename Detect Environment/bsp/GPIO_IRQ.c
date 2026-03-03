#include "GPIO_IRQ.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_10)
	{
		IR_EXTI_Callback();
	}
}
