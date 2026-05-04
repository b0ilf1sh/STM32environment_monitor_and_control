#ifndef _APP_BOOTLOADER_H_
#define _APP_BOOTLOADER_H_

#include "main.h"
#include "bootloader.h"
#include <stdlib.h>

extern uint32_t Bootloader_RecTime;
extern uint16_t Real_REC_LEN;

typedef enum
{
    APP_BOOTLOADER_STATE_Wait_START_SIGNAL,
    APP_BOOTLOADER_STATE_REC_NEW_PROGRAM,
    APP_BOOTLOADER_STATE_WAIT_REC_END,
    APP_BOOTLOADER_STATE_CHECK_REC_DATA,
    APP_BOOTLOADER_STATE_JUMP_TO_NEW_APP,
}APP_BOOTLOADER_STATE;

void App_Bootloader_Run(void);

#endif 
