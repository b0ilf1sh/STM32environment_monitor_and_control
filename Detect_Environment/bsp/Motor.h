#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "main.h"

extern TIM_HandleTypeDef htim3;

void Motor_Init(void);
void Motor_SetSpeed(uint8_t Speed);

#endif
