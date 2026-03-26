#ifndef _OTA_H__
#define _OTA_H__

#include "main.h"
#include "ESP01S.h"
#include <stdlib.h>
#include "W25Q64.h"

#define OTA_NEW_APP_BUFF_LEN   1280  //新程序接收缓存大小

// #define OTA_CHECK_UPDATE_TIME   3600000   //每隔一小时检查一次更新
#define OTA_CHECK_UPDATE_TIME   120000   //每隔两分钟检查一次更新
#define OTA_APP_W25Q64_Start_Address 0x010000 //APP在W25Q64中的首地址
#define OTA_APP_W25Q64_Sector_Size   0x1000  //W25Q64扇区大小
#define OTA_APP_W25Q64_SECTOR_NUM    32    //需要擦除的扇区数量
#define OTA_W25Q64_PAGE_LEN  256  //W25Q64的页大小
#define OTA_NEW_APP_SIZE    1024  //每次接收新程序大小
#define OTA_APP_Start_Address   0x08008000   //程序在单片机flash的起始地址
#define OTA_APP_End_Address     0x08020000     //程序在单片机flash的结尾地址
#define OTA_APP_UPDATE_FLAG_ADDR 0x000000    //W25Q64中更新标志位和密钥地址
#define OTA_APP_LEN_ADDR   0x001000    //APP长度地址
#define OTA_APP_UPDATE     0x01            //更新
#define OTA_APP_NO_UPDATE  0x02            //不更新
#define OTA_APP_KEY        0x5A6B            //密钥   


typedef enum{
    OTA_IDLE,
    OTA_INIT,
    OTA_REPORT_VERSION,
    OTA_CHECK_UPDATE,
    OTA_APP_DOWN,
    OTA_APP_CHECK,
    OTA_RESET,
    OTA_FINISH,
}OTA_STATE_t;

extern uint8_t ESP01S_REC_RING_BUFF[ESP01S_REC_RING_BUFF_LEN];//环形缓冲区
extern uint16_t Read_Index;  //读索引
extern uint8_t ESP01S_READ_BUFF[ESP01S_READ_BUFF_LEN];//读缓存
extern uint8_t ESP01S_REC_BUFF_FLAG;//确定将数据存入哪个数组中，0为OTA，1为MQTT
extern uint8_t MQTT_ConnectState;

void OTA_Run(void);
uint8_t get_length_lookup(uint16_t num);

#endif 
