#include "Bootloader.h"

uint8_t Boorloader_UpdateFlag=BOOTLOADER_NO_UPDATE;//更新程序标志位

//检查是否恢复出厂程序
void Bootloader_CheckFactoryFlag(void)
{
    HAL_Delay(3000);
    return;
}

//检查更新标志位
void Bootloader_CheckUpdateFlag(void)
{
    //读取更新标志位和密钥
    uint8_t UpdateData[3];
    W25Q64_ReadPage(BOOTLOADER_UPDATE_FLAG_ADDR, UpdateData, sizeof(UpdateData));
    uint16_t Key = UpdateData[1] | (UpdateData[2]<<8);
    //比较密钥
    if(Key!=BOOTLOADER_KEY)
    {
        UpdateData[0]=BOOTLOADER_NO_UPDATE;
        UpdateData[1]=(uint8_t)(BOOTLOADER_KEY&0xFF);
        UpdateData[2]=(uint8_t)((BOOTLOADER_KEY>>8)&0xFF);
        W25Q64_EraseSector(BOOTLOADER_UPDATE_FLAG_ADDR);
        W25Q64_WritePage(BOOTLOADER_UPDATE_FLAG_ADDR, UpdateData, sizeof(UpdateData));
    }
    else
    {
        Boorloader_UpdateFlag=UpdateData[0];
    }
}

//擦除flash
void Bootloader_Erase_OLD_APP(void)
{
    //声明一个擦除结构体
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES; //擦除类型为页擦除
    EraseInitStruct.Banks = FLASH_BANK_1; //选择flash的bank
    EraseInitStruct.PageAddress = BOOTLOADER_APP_ADDR; //擦除起始地址
    EraseInitStruct.NbPages = 96; //擦除页数

    uint32_t PageError = 0;

    //擦除flash
    HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    printf("erase flash ok\r\n");
}

//将新程序写入flash中
void Bootloader_Write_New_APP(void)
{
    printf("start write flash\r\n");
    //先获得新程序长度
    uint8_t APP_Size[4] = {0};
    W25Q64_ReadPage(BOOTLOADER_APP_LEN_ADDR, APP_Size, sizeof(APP_Size));
    uint32_t Bootloader_New_App_Len = APP_Size[0] | (APP_Size[1]<<8) | (APP_Size[2]<<16) | (APP_Size[3]<<24);
    printf("new app len=%d\r\n",Bootloader_New_App_Len);
    //写入新程序
    for(uint16_t i=0; i<(Bootloader_New_App_Len/BOOTLOADER_W25Q64_PAGE_LEN)+1; i++)
    {
		OLED_ShowString(1, 1, "Update:");
		if(i!=Bootloader_New_App_Len/BOOTLOADER_W25Q64_PAGE_LEN)
		{
			OLED_ShowNum(1, 8, i*100/(Bootloader_New_App_Len/BOOTLOADER_W25Q64_PAGE_LEN), 3);
		}
		else
		{
			OLED_ShowNum(1, 8, 100, 3);
		}
		OLED_ShowChar(1, 11, '%');
        //先将数据从W25Q64中读出中读出来
        uint8_t TxData[256];
        W25Q64_ReadPage(BOOTLOADER_APP_W25Q64_Start_Address+i*BOOTLOADER_W25Q64_PAGE_LEN, TxData, sizeof(TxData));
        //再将数据写入flash
        uint16_t data=0;
        for(uint16_t j=0; j<BOOTLOADER_W25Q64_PAGE_LEN; j+=2)
        {
            data = TxData[j] | (TxData[j+1]<<8);
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, BOOTLOADER_APP_ADDR + i*BOOTLOADER_W25Q64_PAGE_LEN + j, data);
        } 
        printf("i=%d\r\n",i);
    }
    printf("end write flash\r\n");
}

//跳转到程序
void Bootloader_JumpToApp(uint32_t AppAddr)
{
    typedef void(*pFunc)(void);

    uint32_t APP_Stack_ptr = *(volatile uint32_t*)(AppAddr);
    uint32_t APP_Reset_Handler = *(volatile uint32_t*)(AppAddr+4);
    
    //2.注销bootloader程序
    NVIC_DisableIRQ(EXTI15_10_IRQn);//关闭中断
    //关闭systick中断
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    //2.1注销hal库设置
    HAL_DeInit();//注销外设配置，不会注销内核

    //2.2关闭中断，关闭后NVIC寄存器就不能正常写入了
    __disable_irq();

    //2.3设置堆栈指针
    __set_MSP(APP_Stack_ptr);

    //2.4重定向中断向量表
    SCB->VTOR = AppAddr;

    //2.5跳转至APP复位中断
    pFunc Jump_To_Application = (pFunc)APP_Reset_Handler;
    Jump_To_Application();
}

//Bootloader应用程序
void Bootloader_Run(void)
{
    //1.先检查是否恢复出厂设置
    Bootloader_CheckFactoryFlag();
    if(Boorloader_UpdateFlag == BOOTLOADER_RESET)
    {
        printf("reset\r\n");
        Bootloader_JumpToApp(BOOTLOADER_RESET_ADDR);//跳转到复位程序
    }
    else
    {
        //2.检查更新标志位
        Bootloader_CheckUpdateFlag();
        if(Boorloader_UpdateFlag == BOOTLOADER_NO_UPDATE)//如果没有更新
        {
            printf("no updata\r\n");
            Bootloader_JumpToApp(BOOTLOADER_APP_ADDR);//跳转到APP程序
        }
        else if(Boorloader_UpdateFlag == BOOTLOADER_UPDATE)//如果需要更新
        {
            printf("updata\r\n");
            HAL_FLASH_Unlock();
            //先将存放APP区域的FLASH擦除
            Bootloader_Erase_OLD_APP();
            //再将新程序写入FLASH
            Bootloader_Write_New_APP();
            HAL_FLASH_Lock();

            //写完后清零更新标志位
            uint8_t UpdateData[3];
            UpdateData[0]=BOOTLOADER_NO_UPDATE;
            UpdateData[1]=(uint8_t)(BOOTLOADER_KEY&0xFF);
            UpdateData[2]=(uint8_t)((BOOTLOADER_KEY>>8)&0xFF);
            W25Q64_EraseSector(BOOTLOADER_UPDATE_FLAG_ADDR);
            W25Q64_WritePage(BOOTLOADER_UPDATE_FLAG_ADDR, UpdateData, sizeof(UpdateData));

            printf("start jump to new app\r\n");

            Bootloader_JumpToApp(BOOTLOADER_APP_ADDR);
        }
    }
}
