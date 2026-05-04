#ifndef _BOOTLOADER_H_
#define _BOOTLOADER_H_

#include "main.h"
#include "w25q64.h"
#include <stdio.h>
#include "OLED.h"
#include "IR.h"

#define BOOTLOADER_UPDATE_FLAG_ADDR 0x000000    //W25Q64中更新标志位和密钥地址
#define BOOTLOADER_APP_LEN_ADDR   0x001000    //APP长度地址
#define BOOTLOADER_UPDATE     0x01            //更新
#define BOOTLOADER_NO_UPDATE  0x02            //不更新
#define BOOTLOADER_RESET      0x03            //复位
#define BOOTLOADER_KEY        0x5A6B            //密钥   
#define BOOTLOADER_RESET_ADDR 0x08004000    //出厂程序地址
#define BOOTLOADER_APP_ADDR   0x08008000    //应用程序地址
#define BOOTLOADER_W25Q64_PAGE_LEN  256
#define BOOTLOADER_APP_W25Q64_Start_Address 0x010000   //APP在W25Q64中的起始地址

void Bootloader_Run(void);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);


#endif 
