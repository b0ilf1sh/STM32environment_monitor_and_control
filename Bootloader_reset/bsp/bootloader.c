#include "bootloader.h"

uint8_t Bootloader_RecBuff[Bootloader_RecBuf_LEN] = {0};
uint16_t Bootloader_RecLen=0;//每次串口中断接收到的数据长度
uint16_t Real_REC_LEN = 0;//实际接收到的数据长度，用来校验
uint32_t Flash_Address_offset=0;//flash位置偏移
uint8_t last_byte=0;//如果最后剩一个字节没写，存起来下次写
uint8_t last_byte_flag=0;//判断是否还剩一个字节
uint32_t Bootloader_RecTime=0; //记录接收时的时间

void Bootloader_EraseFlash(void)
{
    //flash解锁
    HAL_FLASH_Unlock();

    //声明一个擦除结构体
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES; //擦除类型为页擦除
    EraseInitStruct.Banks = FLASH_BANK_1; //选择flash的bank
    EraseInitStruct.PageAddress = APP_Start_Address; //擦除起始地址
    EraseInitStruct.NbPages = 96; //擦除页数

    uint32_t PageError = 0;

    //擦除flash
    HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

    //flash加锁
    HAL_FLASH_Lock();
}

//串口初始化
void Bootloader_UARTInit(void)
{
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1); 
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, Bootloader_RecBuff, Bootloader_RecBuf_LEN);
}

//跳转到APP
uint8_t Bootloader_JumpToApp(void)
{
    typedef void(*pFunc)(void);
    //1.校验
    uint32_t APP_Stack_ptr = *(volatile uint32_t*)(APP_Start_Address);
    uint32_t APP_Reset_Handler = *(volatile uint32_t*)(APP_Start_Address+4);

    //1.1校验栈顶地址
    if((APP_Stack_ptr & 0xFFFF0000) != Top_Stack_Address)
    {
        printf("APP Stack ptr error!\r\n");
        return 1;
    }

    //1.2校验复位中断地址
    if (APP_Reset_Handler < APP_Start_Address || APP_Reset_Handler > APP_End_Address)
    {
        printf("APP Reset Handler error!\r\n");
        return 1;
    }
    
    //2.注销bootloader程序
    NVIC_DisableIRQ(USART1_IRQn);//关闭串口中断
    //NVIC_DisableIRQ(EXTI3_IRQn);
    //关闭systick中断
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    //2.1注销hal库设置
    HAL_DeInit();//注销外设配置，不会注销内核

    //2.2关闭中断
    __disable_irq();

    //2.3设置堆栈指针
    __set_MSP(APP_Stack_ptr);

    //2.4重定向中断向量表
    SCB->VTOR = APP_Start_Address;

    //2.5跳转至APP复位中断
    pFunc Jump_To_Application = (pFunc)APP_Reset_Handler;
    Jump_To_Application();
    return 0;
}

static void Bootloader_WriteFlash(void)
{
    uint16_t data=0;
    if((Bootloader_RecLen+last_byte_flag)%2==0)//有偶数个字节
    {
        if(last_byte_flag)//这次为奇数，上次为奇数
        {
            for(uint16_t i=0;i<Bootloader_RecLen;i+=2)
            {
                if(i==0)
                {
                    data = last_byte | (Bootloader_RecBuff[i]<<8);
                }
                else
                {
                    data = Bootloader_RecBuff[i-1] | (Bootloader_RecBuff[i]<<8);
                }
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_Start_Address + Flash_Address_offset + i, data);
            }
            Flash_Address_offset += Bootloader_RecLen+1;
            last_byte_flag = 0;
        }
        else//这次为偶数，上次为偶数
        {
            for(uint16_t i=0;i<Bootloader_RecLen;i+=2)
            {
                data = Bootloader_RecBuff[i] | (Bootloader_RecBuff[i+1]<<8);
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_Start_Address + Flash_Address_offset + i, data);
            }
            Flash_Address_offset += Bootloader_RecLen;
            last_byte_flag = 0;
        }
    }                       
    else//有奇数个字节
    {
        if(last_byte_flag)//这次为偶数，上次为奇数
        {
            for(uint16_t i=0;i<Bootloader_RecLen;i+=2)
            {
                if(i==0)
                {
                    data = last_byte | (Bootloader_RecBuff[i]<<8);
                }
                else
                {
                    if(i+1<Bootloader_RecLen)
                    {
                        data = Bootloader_RecBuff[i-1] | (Bootloader_RecBuff[i]<<8);
                    }    
                }
                HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_Start_Address + Flash_Address_offset + i, data); 
            }
            Flash_Address_offset += Bootloader_RecLen;
        }
        else//这次为奇数，上次为偶数
        {
            for(uint16_t i=0; i<Bootloader_RecLen; i+=2)
            {
                if(i+1<Bootloader_RecLen)
                {
                    data = Bootloader_RecBuff[i] | (Bootloader_RecBuff[i+1]<<8);
                    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, APP_Start_Address + Flash_Address_offset + i, data);
                }
            }
            Flash_Address_offset += Bootloader_RecLen-1;
        }
        last_byte = Bootloader_RecBuff[Bootloader_RecLen-1];
        last_byte_flag = 1;
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart->Instance == USART1)
    {
        Bootloader_RecLen = Size;
        Real_REC_LEN += Bootloader_RecLen;

        // printf("Real_REC_LEN:%d\n",Real_REC_LEN);

        //将接收到的数据存入FLASH
        //flash解锁
        HAL_FLASH_Unlock();

         //写入flash
        Bootloader_WriteFlash();

        //flash加锁
        HAL_FLASH_Lock();

        //清空接收缓冲区
        memset(Bootloader_RecBuff,0,Bootloader_RecBuf_LEN);

        //重新打开中断
        __HAL_UART_CLEAR_OREFLAG(&huart1);
        __HAL_UART_CLEAR_IDLEFLAG(&huart1); 
        HAL_UARTEx_ReceiveToIdle_IT(&huart1, Bootloader_RecBuff, Bootloader_RecBuf_LEN);

        //记录时间
        Bootloader_RecTime = HAL_GetTick();
    }
}
