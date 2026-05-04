#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include "main.h"
#include <stdio.h>
#include <string.h>

#define Bootloader_RecBuf_LEN 512
#define APP_Start_Address   0x08008000
#define APP_End_Address     0x08020000
#define Top_Stack_Address   0x20000000

extern UART_HandleTypeDef huart1;

void Bootloader_EraseFlash(void);
void Bootloader_UARTInit(void);
uint8_t Bootloader_JumpToApp(void);
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size);

#endif 
