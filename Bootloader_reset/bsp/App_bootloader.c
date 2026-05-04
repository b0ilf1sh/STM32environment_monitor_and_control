#include "App_bootloader.h"

uint8_t RxData[64]={0};
uint16_t RxLen=0;
uint16_t App_Len=0;
APP_BOOTLOADER_STATE App_Bootloader_State=APP_BOOTLOADER_STATE_Wait_START_SIGNAL;

//1.接收开始传输信号
void App_Bootloader_RecStartSignal(void)
{
    printf("reset bootloader start\r\n");
    printf("wait user send data\r\n");
    printf("send 'start:len' to start\r\n");
    //接收开始信号
	RxLen=0;
    HAL_UARTEx_ReceiveToIdle(&huart1, RxData, 64, &RxLen,HAL_MAX_DELAY);
    //接收到信号
    if(RxLen>0)
    {
        //判断是否为开始信号
        char *data=strstr((char *)RxData,"start:");
        if(data!=NULL)//接收到开始信号
        {
            printf("Get start signal\r\n");
            App_Len=atoi((char *)data+6);
			if(App_Len>0)
			{
				printf("total len:%d\r\n",App_Len);
				App_Bootloader_State=APP_BOOTLOADER_STATE_REC_NEW_PROGRAM;
			}
			else
			{
				printf("len err\r\n");
			}
        }
        else
        {
            printf("Get wrong signal\r\n");
        }
    }
}

//2.开始接收新程序
void App_Bootloader_RecNewProgram(void)
{
    //擦除flash
    Bootloader_EraseFlash();
    printf("Flash erased\r\n");
    //开启中断接收
    Bootloader_UARTInit();
    printf("UART init\r\n");
    App_Bootloader_State=APP_BOOTLOADER_STATE_WAIT_REC_END;
}

//3.判断接收是否结束
void App_Bootloader_WaitRecEnd(void)
{
    if(Bootloader_RecTime!=0)
    {
        if(HAL_GetTick()-Bootloader_RecTime>2000)
        {
            printf("Rec end\r\n");
            App_Bootloader_State=APP_BOOTLOADER_STATE_CHECK_REC_DATA;
        }
    }
}

//4.检查接收到的数据
uint8_t App_Bootloader_CheckRecData(void)
{
    if(App_Len == Real_REC_LEN)
    {
        printf("Rec data ok\r\n");
        App_Bootloader_State=APP_BOOTLOADER_STATE_JUMP_TO_NEW_APP;
        return 0;
    }
    else
    {
        printf("Rec data error\r\n");
        return 1;
    }
}

//5.跳转到新程序
uint8_t App_Bootloader_JumpToNewApp(void)
{
    uint8_t ret = Bootloader_JumpToApp();
    return ret;
}

void App_Bootloader_Run(void)
{
   switch(App_Bootloader_State)
   {
    case APP_BOOTLOADER_STATE_Wait_START_SIGNAL:
        App_Bootloader_RecStartSignal();
        break;
    case APP_BOOTLOADER_STATE_REC_NEW_PROGRAM:
        App_Bootloader_RecNewProgram();
        break;
    case APP_BOOTLOADER_STATE_WAIT_REC_END:
        App_Bootloader_WaitRecEnd();
        break;
    case APP_BOOTLOADER_STATE_CHECK_REC_DATA:
        if(App_Bootloader_CheckRecData())
        {
            printf("app rec err, system reset\r\n");
            NVIC_SystemReset();
        }
        break;
    case APP_BOOTLOADER_STATE_JUMP_TO_NEW_APP:
        if(App_Bootloader_JumpToNewApp())
        {
            printf("jump app err, system reset\r\n");
            NVIC_SystemReset();
        }
        break;
    default:
        break;
   }
}
