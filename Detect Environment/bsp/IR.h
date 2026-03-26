#ifndef _IR_H_
#define _IR_H_

#include "main.h"

#define IR_POWER		0xA2
#define IR_MENU			0xE2
#define IR_TEST			0x22
#define IR_ADD	        0x02
#define IR_BACK		    0xC2
#define IR_PREVIOUS		0xE0
#define IR_START_STOP	0xA8
#define IR_NEXT			0x90
#define IR_SUB	        0x98
#define IR_C		    0xB0
#define IR_0			0x68
#define IR_1			0x30
#define IR_2			0x18
#define IR_3			0x7A
#define IR_4			0x10
#define IR_5			0x38
#define IR_6			0x5A
#define IR_7			0x42
#define IR_8			0x4A
#define IR_9			0x52

extern TIM_HandleTypeDef htim1;

void IR_Init(void);
uint8_t IR_GetData(void);
uint8_t IR_GetRepeatFlag(void);
void IR_EXTI_Callback(void);

#endif
