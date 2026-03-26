#include "ESP01S.h"

uint8_t ESP01S_REC_RING_BUFF[ESP01S_REC_RING_BUFF_LEN]={0};//环形缓冲区
uint8_t ESP01S_REC_BUFF[ESP01S_REC_BUFF_LEN]={0};//串口中断接收缓存
uint8_t ESP01S_READ_BUFF[ESP01S_READ_BUFF_LEN]={0};//读取缓存
uint16_t Write_Index=0;//环形缓冲区写索引
uint16_t Read_Index=0;//环形缓冲区读索引
uint16_t ESP01S_Data_Length=0;//记录读了多少缓冲区数据
uint8_t ESP01S_MQTT_REC_BUFF[ESP01S_MQTT_REC_BUFF_LEN]={0};//MQTT接收函数
uint8_t ESP01S_PROTOCOL_FLAG=0;//用来确定当前以什么协议连接ONENET，0为OTA，1为MQTT
uint8_t ESP01S_REC_BUFF_FLAG=0;//确定将数据存入哪个数组中，0为OTA，1为MQTT

//环形缓冲区初始化
void ESP01S_RING_BUFFER_Init(void)
{
    Write_Index=0;//环形缓冲区写索引
    Read_Index=0;//环形缓冲区读索引
    memset(ESP01S_REC_RING_BUFF, 0, ESP01S_REC_RING_BUFF_LEN);//清空环形缓冲区
}

//计算环形缓冲区中数据数量
uint16_t ESP01S_RING_BUFFER_GET_DATA_LENGTH(void)
{
    //如果写索引差一个数据等于读索引，此时可以认为缓冲区满了
    if((Write_Index + 1) % ESP01S_REC_RING_BUFF_LEN == Read_Index)
    {
        return ESP01S_REC_RING_BUFF_LEN;
    }

    if(Write_Index > Read_Index)//写索引大于读索引
    {
        return Write_Index - Read_Index;
    }
    else if(Write_Index < Read_Index)//写索引小于读索引
    {
        return ESP01S_REC_RING_BUFF_LEN + Write_Index - Read_Index;
    }
    else//写索引等于读索引，此时缓冲区为空
    {
        return 0;
    }
}

//计算缓冲区剩余空间大小
uint16_t ESP01S_RING_BUFFER_GET_IDLE_LENGTH(void)
{
    return ESP01S_REC_RING_BUFF_LEN - ESP01S_RING_BUFFER_GET_DATA_LENGTH();
}

//写环形缓冲区
void ESP01S_WRITE_RING_BUFFER(uint8_t *data,uint16_t size)
{
    if(ESP01S_RING_BUFFER_GET_IDLE_LENGTH() <= size)//剩余空间小于等于写入数据长度，则将数据丢弃
    {
        return;
    }
    else  //剩余空间大于数据长度，则将数据保存
    {
        if(Write_Index + size < ESP01S_REC_RING_BUFF_LEN)//如果当前写入数据后未到达缓冲区末尾
        {
            memcpy(ESP01S_REC_RING_BUFF + Write_Index, data, size);
            Write_Index += size;
        }
        else//如果当前写入数据后会到达缓冲区末尾，则需要分两段写
        {
            memcpy(ESP01S_REC_RING_BUFF + Write_Index, data, ESP01S_REC_RING_BUFF_LEN - Write_Index);//先写第一段直接到达末尾
            memcpy(ESP01S_REC_RING_BUFF, data + ESP01S_REC_RING_BUFF_LEN - Write_Index, size - ESP01S_REC_RING_BUFF_LEN + Write_Index);//再写第二段
            Write_Index = (Write_Index + size) % ESP01S_REC_RING_BUFF_LEN;
        }
    }
}

//从环形缓冲区读取数据
uint16_t ESP01S_READ_RING_BUFFER(void)
{
    //判断缓冲区是否有数据
    if(ESP01S_RING_BUFFER_GET_DATA_LENGTH()==0)//没有数据，读取失败
    {
        return 0;
    }
    else//还有数据，读取一个字节
    {
        ESP01S_READ_BUFF[ESP01S_Data_Length] = ESP01S_REC_RING_BUFF[Read_Index];
        ESP01S_Data_Length++;
        Read_Index = (Read_Index + 1) % ESP01S_REC_RING_BUFF_LEN;

        return 1;
    }
}

void ESP01S_CLEAR_READ_BUFF(void)
{
    ESP01S_Data_Length = 0;
    memset(ESP01S_READ_BUFF, 0, ESP01S_READ_BUFF_LEN);
}

void ESP01S_Printf(const char* format, ...)
{
    char buffer[512];  // 根据你的需要调整大小
    va_list args;
    
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // 通过串口2发送给ESP-01S
    HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

//初始化串口接收
void ESP01S_UART_Init(void)
{
    //清空串口初始化前的所有问题
    __HAL_UART_CLEAR_OREFLAG(&huart2);
    __HAL_UART_CLEAR_IDLEFLAG(&huart2); 
    //关闭DMA过半中断
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
    //串口中断接收函数
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, ESP01S_REC_BUFF, ESP01S_REC_BUFF_LEN);//使用串口空闲中断加DMA接收数据
}

//清空缓存
void ESP01S_ClearBuff(void)
{
    memset(ESP01S_REC_BUFF, 0, ESP01S_REC_BUFF_LEN);
}

//等待应答
uint8_t ESP01S_WaitAck(char *string, uint16_t timeout_ms)
{
    uint32_t start_time = HAL_GetTick();//记录开始时间
    while(HAL_GetTick() - start_time < timeout_ms)
    {
        if(ESP01S_READ_RING_BUFFER())//如果从缓冲区中读到了数据
        {
            if(strstr((char *)ESP01S_READ_BUFF, (char *)string)!=NULL)
            {
                return 1;
            }
        }
    }
    return 0;
}

void ESP01S_INFORMATION(void)
{
    printf("Wirte_Index=%d\r\n",Write_Index);
    printf("Read_Index=%d\r\n",Read_Index);
    for(uint16_t i=0;i<ESP01S_Data_Length;i++)
    {
        printf("%c",ESP01S_READ_BUFF[i]);
    }
    printf("\r\n");
}

//初始化ESP01S,并且连接到WIFI
uint8_t ESP01S_Init(void)
{
    //先开启串口中断
    ESP01S_UART_Init();
    //初始化环形缓冲区
    ESP01S_RING_BUFFER_Init();

    //先复位ESP01S,并等待2秒将初始化内容删除
    ESP01S_CLEAR_READ_BUFF();
    ESP01S_Printf("AT+RST\r\n");
    if(ESP01S_WaitAck("OK", 5000)==1)
    {
        printf("ESP01S RESET OK\r\n");
    }
    else
    {
        printf("ESP01S RESET ERROR\r\n");
        return 1;
    }

    mdelay(2000);
    //ESP01S复位后会发送大量信息，可能会造成错误，这里需要重新开启串口中断
    ESP01S_UART_Init();
    ESP01S_RING_BUFFER_Init();
    
    //在发送AT进行测试
    ESP01S_CLEAR_READ_BUFF();//清空缓存
    ESP01S_Printf("AT\r\n");
    if(ESP01S_WaitAck("OK", 2000)==1)
    {
        printf("ESP01S TEST OK\r\n");
    }
    else
    {
        printf("ESP01S TEST ERROR\r\n");
        return 2;
    }
    
    //关闭回显
    ESP01S_CLEAR_READ_BUFF();
    ESP01S_Printf("ATE0\r\n");
    if(ESP01S_WaitAck("OK", 2000)==1)
    {
        printf("ESP01S CLOSE ECHO OK\r\n");
    }
    else
    {
        printf("ESP01S CLOSE ECHO ERROR\r\n");
        return 3;
    }
    
    //设置ESP01S为WIFI模式
    ESP01S_CLEAR_READ_BUFF();
    ESP01S_Printf("AT+CWMODE=1\r\n");
    if(ESP01S_WaitAck("OK", 2000)==1)
    {
        printf("ESP01S WIFI MODE OK\r\n");
    }
    else
    {
        printf("ESP01S WIFI MODE ERROR\r\n");
        return 4;
    }
    
    //连接WIFI
    ESP01S_CLEAR_READ_BUFF();
    ESP01S_Printf("AT+CWJAP=\"%s\",\"%s\"\r\n",WIFI_SSID,WIFI_PWD);
    if(ESP01S_WaitAck("OK", 5000)==1)
    {
        printf("ESP01S CONNECT WIFI OK\r\n");
    }
    else
    {
        printf("ESP01S CONNECT WIFI ERROR\r\n");
        return 5;
    }

    printf("ESP01S INIT OK\r\n");
    return 0;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(huart->Instance==USART2)
    {
        if(ESP01S_REC_BUFF_FLAG == 0)//此时为OTA程序开始接收
        {
            //将接收到的数据写入缓冲区
            ESP01S_WRITE_RING_BUFFER(ESP01S_REC_BUFF, Size);
        }
        else//MQTT接收
        {
            //将数据写入MQTT的缓存区
            memcpy(ESP01S_MQTT_REC_BUFF, ESP01S_REC_BUFF, Size);
            //判断是否与WIFI或服务器断开连接
            if(Size<25)
            {
                if(strstr((char *)ESP01S_MQTT_REC_BUFF, "WIFI DISCONNECT") || strstr((char *)ESP01S_MQTT_REC_BUFF, "MQTTDISCONNECTED"))
                {
                    MQTT_ConnectState = 1;
                }
            }
        }
        
        ESP01S_ClearBuff();//清空接收数组
		ESP01S_UART_Init();//重新开启中断
    }
}
