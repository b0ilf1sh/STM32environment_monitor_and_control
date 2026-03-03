#ifndef _COLORLED_H_
#define	_COLORLED_H_

#include "hardware.h"

void ColorLED_Init(void);
void ColorLED_SetColor(uint8_t R, uint8_t G, uint8_t B);
void ColorLED_OFF(void);
void ColorLED_Alarm(void);

#endif
