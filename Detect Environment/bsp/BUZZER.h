#ifndef _BUZZER_H_
#define _BUZZER_H_

#include "hardware.h"

#define P    0

#define L1   305-1
#define L1_  289-1
#define L2   272-1
#define L2_  257-1
#define L3   242-1
#define L4   229-1
#define L4_  216-1
#define L5   204-1
#define L5_  193-1
#define L6   182-1
#define L6_  172-1
#define L7   162-1

#define M1   153-1
#define M1_  144-1
#define M2   136-1
#define M2_  129-1
#define M3   121-1
#define M4   115-1
#define M4_  108-1
#define M5   102-1
#define M5_  96-1
#define M6   91-1
#define M6_  86-1
#define M7   81-1

#define Buzzer_Speed   500

void BUZZER_Init(void);
void BUZZER_SetVolume(uint8_t Volume);
uint8_t BUZZER_GetVolume(void);
void BUZZER_OFF(void);
void BUZZER_Music(uint8_t Volume, uint64_t Pre, uint64_t time);
void Buzzer_HintVoice(uint8_t volumn);
void Buzzer_Alarm(uint8_t volumn);

#endif
