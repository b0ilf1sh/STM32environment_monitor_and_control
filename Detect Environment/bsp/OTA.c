#include "OTA.h"

OTA_STATE_t OTA_State=OTA_INIT;
//s为单片机软件版本，需要修改，f为模块版本，无需修改
int16_t mcu_version_integer=0;//单片机版本的整数部分
int16_t mcu_version_decimals=0;//单片机版本小数部分
int16_t target_version_integer=0;//获得目标版本的整数部分
int16_t target_version_decimals=0;//目标版本小数部分
uint32_t tid=0;//获得用于更新的tid
uint32_t target_version_size=0;//目标版本文件大小
uint32_t APP_RealGet_Size=0;//实际接收到的大小
uint8_t update_status=0;//升级状态
uint64_t OTA_LastTime=0;
uint8_t OTA_NEW_APP_BUFF[OTA_NEW_APP_BUFF_LEN]={0};

//与服务器建立TCP连接,并开启透传
uint8_t OTA_TCP_CONNECT(void)
{
    //连接TCP
    ESP01S_CLEAR_READ_BUFF();
    ESP01S_Printf("AT+CIPSTART=\"TCP\",%s,%d\r\n",tcp_host,tcp_port);
    if(ESP01S_WaitAck("CONNECT", 5000)==1)
    {
        printf("OTA CONNECT TCP OK\r\n");
    }
    else
    {
        printf("OTA CONNECT TCP ERROR\r\n");
        return 1;
    }

    //开启透传
    ESP01S_CLEAR_READ_BUFF();
    ESP01S_Printf("AT+CIPMODE=1\r\n");
    if(ESP01S_WaitAck("OK", 5000)==1)
    {
        printf("OTA START TRANSPARENT OK\r\n");
    }
    else
    {
        printf("OTA START TRANSPARENT ERROR\r\n");
        return 2;
    }

    //开启发送
    ESP01S_CLEAR_READ_BUFF();
    ESP01S_Printf("AT+CIPSEND\r\n");
    if(ESP01S_WaitAck(">", 5000)==1)
    {
        printf("OTA START SEND OK\r\n");
    }
    else
    {
        printf("OTA START SEND ERROR\r\n");
        return 3;
    }

    return 0;
}

//关闭透传
void OTA_CLOSE_TOUC(void)
{
    ESP01S_CLEAR_READ_BUFF();
    ESP01S_Printf("+++");
    HAL_Delay(1000);
}

//断开TCP连接
uint8_t OTA_TCP_DISCONNECT(void)
{
    //先关闭透传
    OTA_CLOSE_TOUC();

    ESP01S_CLEAR_READ_BUFF();
    ESP01S_Printf("AT+CIPCLOSE\r\n");
    if(ESP01S_WaitAck("CLOSED", 5000)==1)
    {
        printf("OTA DISCONNECT TCP OK\r\n");
        return 0;
    }
    else
    {
        printf("OTA DISCONNECT TCP ERROR\r\n");
        return 1;
    }
}

//OTA初始化，先连接WIFI,再与服务器建立tcp连接，再开启透传
uint8_t OTA_Init(void)
{
    //连接WIFI
    if(ESP01S_Init())
    {
        printf("ESP01S INIT ERROR\r\n");
        return 1;
    }

    //与服务器建立TCP连接
    if(OTA_TCP_CONNECT())
    {
        printf("OTA TCP CONNECT ERROR\r\n");
        return 2;
    }

    printf("OTA INIT OK\r\n");
    return 0;  
}

uint8_t Report_Version(uint32_t integer, uint32_t decimals)
{
    ESP01S_CLEAR_READ_BUFF();
    ESP01S_Printf("POST /fuse-ota/%s/%s/version HTTP/1.1\r\n"
                  "Content-Type: application/json\r\n"
                  "Authorization:%s\r\n"
                  "host:iot-api.heclouds.com\r\n"
                  "Content-Length:%d\r\n"
                  "\r\n"
                  "{\"s_version\":\"V%d.%d\", \"f_version\": \"V2.0\"}\r\n\r\n", tcp_username, tcp_client_id, tcp_password, 39+get_length_lookup(integer)+get_length_lookup(decimals), integer,decimals);

    //先找到request_id，request_id为数据末尾的字符串，这样确保读缓冲中的数据是完整的
    if(ESP01S_WaitAck("request_id", 5000))
    {
        //寻找发送成功的字符串
        if(strstr((char *)ESP01S_READ_BUFF, "\"msg\":\"succ\"")!=NULL)//上报版本成功
        {
            printf("REPORT VERSION OK\r\n");
            return 1;
        }
        else//失败
        {
            printf("REPORT VERSION ERROR\r\n");
            return 0;
        }
    }
    else
    {
        printf("no request_id\r\n");
        return 0;
    }
}

//上报设备当前版本
uint8_t OTA_ReportVersion(void)
{
    //从W25Q64获取单片机软件版本
    uint8_t version[4]={0};
    W25Q64_ReadPage(OTA_APP_LEN_ADDR+4, version, 4);
    mcu_version_integer = version[0] | (version[1]<<8) | (version[2]<<16) | (version[3]<<24);//整数部分
    W25Q64_ReadPage(OTA_APP_LEN_ADDR+8, version, 4);
    mcu_version_decimals = version[0] | (version[1]<<8) | (version[2]<<16) | (version[3]<<24);//小数部分

    //上报版本
    return Report_Version(mcu_version_integer, mcu_version_decimals);
}

uint8_t OTA_GetNeedData(void)
{
    //寻找末尾的request_id这个字符串
    if(ESP01S_WaitAck("request_id", 5000))//找到了
    {
        //先判断有无升级信息
        if(strstr((char *)ESP01S_READ_BUFF, "\"msg\":\"task succ\"")!=NULL 
        || strstr((char *)ESP01S_READ_BUFF, "\"msg\":\"not exist\"")!=NULL)//检测到升级成功或没有升级任务
        {
            printf("NO UPDATE\r\n");
            return 2;
        }
        else if(strstr((char *)ESP01S_READ_BUFF, "\"msg\":\"succ\"")!=NULL)//有升级任务
        {
            //检测是否有升级标志位
            char *data = strstr((char *)ESP01S_READ_BUFF, "status");
            if(data!=NULL)
            {
                update_status = atoi((char *)data+8);
                if(update_status != 1)//不是待升级状态
                {
                    printf("no update state\r\n");
                    return 2;
                }
                else//待升级状态
                {
                    //获取需要的数据
                    data = strstr((char *)ESP01S_READ_BUFF, "target");//版本信息
                    if(data != NULL)
                    {
                        target_version_integer = atoi((char *)data+10);
                        target_version_decimals = atoi((char *)data+12);
                        if(target_version_integer<0 || target_version_decimals<0)
                        {
                            printf("GET VERSION ERROR\r\n");
                            return 0;
                        }
                    }

                    data = strstr((char *)ESP01S_READ_BUFF, "tid");//tid信息
                    if(data != NULL)
                    {
                        tid = atoi((char *)data+5);
                        if(tid<=0)
                        {
                            printf("GET TID ERROR\r\n");
                            return 0;
                        }
                    }

                    data = strstr((char *)ESP01S_READ_BUFF, "size");//新程序大小信息
                    if(data != NULL)
                    {
                        target_version_size = atoi((char *)data+6);
                        if(target_version_size<=0)
                        {
                            printf("GET TID ERROR\r\n");
                            return 0;
                        }
                    }

                    printf("UPDATE\r\n");
                    printf("version:V%d.%d,update_status:%d,tid=%d,size=%d\r\n",target_version_integer,target_version_decimals,update_status,tid,target_version_size);
                    return 1;
                }
            }
            else
            {
                printf("GET STATUS ERROR\r\n");
                return 0;
            }
        }
        else//都没找到，错误
        {
            printf("Data Error\r\n");
            return 0;
        }
    }
    else//没找到
    {
        printf("no request_id\r\n");
        return 0;
    }
}

//检测升级任务
uint8_t OTA_CheckUpdate(void)
{
    //发送检测升级任务指令
    ESP01S_CLEAR_READ_BUFF();
    ESP01S_Printf("GET /fuse-ota/%s/%s/check?type=2&version=V%d.%d HTTP/1.1\r\n"
                  "Content-Type: application/json\r\n"
                  "Authorization:%s\r\n"
                  "host:iot-api.heclouds.com\r\n\r\n", tcp_username, tcp_client_id, mcu_version_integer,mcu_version_decimals, tcp_password);

    //检测是否需要升级,返回1为需要升级，0为不要
    return OTA_GetNeedData();
}

//擦除W25Q64中存储APP的区域（BLOCK1和BLOCK2，共128KB）
void OTA_EraseAppBlock(void)
{
    uint32_t App_Sector_Address=0;
    for(uint8_t i=0;i<OTA_APP_W25Q64_SECTOR_NUM;i++)
    {
        App_Sector_Address = OTA_APP_W25Q64_Start_Address + i * OTA_APP_W25Q64_Sector_Size;
        W25Q64_EraseSector(App_Sector_Address);
    }

    printf("erase ok\r\n");
}

//下载新程序
uint8_t OTA_DOWNLOAD_NEW_APP(void)
{
    //擦除W25Q64中的APP区域
    OTA_EraseAppBlock();

    uint16_t i=0,j=0,k=0;
    char *data_ptr=0;

    //判断新APP大小是否为每次接收大小的整数倍
    if(target_version_size % OTA_NEW_APP_SIZE == 0)//是整数倍
    {
        //如果为整数倍，接收次数为(target_version_size/OTA_NEW_APP_SIZE)
        for(i=0; i<target_version_size/OTA_NEW_APP_SIZE; i++)
        {
            printf("i=%d\r\n",i);//调试信息
            //每次接收都有5次机会
            for(j=0; j<5; j++)
            {
                //先清空读缓存数组
                ESP01S_CLEAR_READ_BUFF();
                //发送接收数据命令,每次接收1KB
                ESP01S_Printf("GET /fuse-ota/%s/%s/%d/download HTTP/1.1\r\n"
                            "Authorization:%s\r\n"
                            "host:iot-api.heclouds.com\r\n"
                            "Range:bytes=%d-%d\r\n\r\n",tcp_username, tcp_client_id, tid, tcp_password, OTA_NEW_APP_SIZE*i, OTA_NEW_APP_SIZE*(i+1)-1);

                if(ESP01S_WaitAck("Ota-Errno: 0", 5000))//检测到了没有问题
                {
                    //获取此次数据长度
                    data_ptr = strstr((char *)ESP01S_READ_BUFF, "Content-Length");
                    if(data_ptr != NULL)
                    {
                        uint16_t content_length = atoi(data_ptr+16);//获取本次获得的程序大小信息
                        APP_RealGet_Size+=content_length;//记录总共获得多少数据，用于后续校验
                        printf("content_len=%d\r\n",content_length);
                        printf("APP_RealGet_Size=%d\r\n",APP_RealGet_Size);

                        //先找到\r\n\r\n，这个字符串后面为新程序信息
                        if(ESP01S_WaitAck("\r\n\r\n", 1000))//找到了该信息，此时读索引已经位于新程序的开头
                        {
                            //先将新程序存入新程序缓存区
                            for(uint16_t i=0; i<content_length; i++)
                            {
                                OTA_NEW_APP_BUFF[i] = ESP01S_REC_RING_BUFF[Read_Index];
                                Read_Index = (Read_Index + 1) % ESP01S_REC_RING_BUFF_LEN;
                            }

                            //将数据存到W25Q64
                            for(k=0; k<content_length/OTA_W25Q64_PAGE_LEN; k++)
                            {
                                W25Q64_WritePage(OTA_APP_W25Q64_Start_Address + i * OTA_NEW_APP_SIZE + k * OTA_W25Q64_PAGE_LEN, OTA_NEW_APP_BUFF + k * OTA_W25Q64_PAGE_LEN, OTA_W25Q64_PAGE_LEN);
                            }
                            break;
                        }
                    }
                }

                if(j==4)
                {
                    return 0;
                }
            }
        }
    }
    else//不是整数倍
    {
        //不为整数倍，接收次数为(target_version_size/OTA_NEW_APP_SIZE)+1
        for(i=0; i<(target_version_size/OTA_NEW_APP_SIZE)+1; i++)
        {
            printf("i=%d\r\n",i);//调试信息
            //每次接收都有5次机会
            for(j=0; j<5; j++)
            {
                //先清空读缓存数组
                ESP01S_CLEAR_READ_BUFF();
                if(i!=target_version_size/OTA_NEW_APP_SIZE)//不是最后一次接收
                {
                    //发送接收数据命令,每次接收一页
                    ESP01S_Printf("GET /fuse-ota/%s/%s/%d/download HTTP/1.1\r\n"
                                "Authorization:%s\r\n"
                                "host:iot-api.heclouds.com\r\n"
                                "Range:bytes=%d-%d\r\n\r\n",tcp_username, tcp_client_id, tid, tcp_password, OTA_NEW_APP_SIZE*i, OTA_NEW_APP_SIZE*(i+1)-1);
                }
                else  //最后一次接收，数据大小不为1KB，特殊处理
                {
                    ESP01S_Printf("GET /fuse-ota/%s/%s/%d/download HTTP/1.1\r\n"
                                "Authorization:%s\r\n"
                                "host:iot-api.heclouds.com\r\n"
                                "Range:bytes=%d-%d\r\n\r\n",tcp_username, tcp_client_id, tid, tcp_password, OTA_NEW_APP_SIZE*i, target_version_size-1);
                }
                
                if(ESP01S_WaitAck("Ota-Errno: 0", 5000))//检测到了没有问题
                {
                    //获取此次数据长度
                    data_ptr = strstr((char *)ESP01S_READ_BUFF, "Content-Length");
                    if(data_ptr != NULL)
                    {
                        uint16_t content_length = atoi(data_ptr+16);//获取本次获得的程序大小信息
                        APP_RealGet_Size+=content_length;//记录总共获得多少数据，用于后续校验
                        printf("content_len=%d\r\n",content_length);
                        printf("APP_RealGet_Size=%d\r\n",APP_RealGet_Size);

                        //先找到\r\n\r\n，这个字符串后面为新程序信息
                        if(ESP01S_WaitAck("\r\n\r\n", 1000))//找到了该信息，此时读索引已经位于新程序的开头
                        {
                            //先将新程序存入新程序缓存区
                            for(uint16_t i=0; i<content_length; i++)
                            {
                                OTA_NEW_APP_BUFF[i] = ESP01S_REC_RING_BUFF[Read_Index];
                                Read_Index = (Read_Index + 1) % ESP01S_REC_RING_BUFF_LEN;
                            }

                            //将数据存到W25Q64
                            if(i!=target_version_size/OTA_NEW_APP_SIZE)//不是最后一次存储
                            {
                                for(k=0; k<content_length/OTA_W25Q64_PAGE_LEN; k++)
                                {
                                    W25Q64_WritePage(OTA_APP_W25Q64_Start_Address + i * OTA_NEW_APP_SIZE + k * OTA_W25Q64_PAGE_LEN, OTA_NEW_APP_BUFF + k * OTA_W25Q64_PAGE_LEN, OTA_W25Q64_PAGE_LEN);
                                }
                            }
                            else  //最后一次存储，此时数据不满1KB
                            {
                                //判断最后一次数据是否为256的整数倍
                                if(content_length % OTA_W25Q64_PAGE_LEN == 0)//是整数倍
                                {
                                    for(k=0; k<content_length/OTA_W25Q64_PAGE_LEN; k++)
                                    {
                                        W25Q64_WritePage(OTA_APP_W25Q64_Start_Address + i * OTA_NEW_APP_SIZE + k * OTA_W25Q64_PAGE_LEN, OTA_NEW_APP_BUFF + k * OTA_W25Q64_PAGE_LEN, OTA_W25Q64_PAGE_LEN);
                                    }
                                }
                                else//不是整数倍
                                {
                                    for(k=0; k<(content_length/OTA_W25Q64_PAGE_LEN) + 1; k++)
                                    {
                                        if(k != content_length/OTA_W25Q64_PAGE_LEN)//不是最后一次存入W25Q64
                                        {
                                            W25Q64_WritePage(OTA_APP_W25Q64_Start_Address + i * OTA_NEW_APP_SIZE + k * OTA_W25Q64_PAGE_LEN, OTA_NEW_APP_BUFF + k * OTA_W25Q64_PAGE_LEN, OTA_W25Q64_PAGE_LEN);
                                        }
                                        else//最后一次存入W25Q64
                                        {
                                            W25Q64_WritePage(OTA_APP_W25Q64_Start_Address + i * OTA_NEW_APP_SIZE + k * OTA_W25Q64_PAGE_LEN, OTA_NEW_APP_BUFF + k * OTA_W25Q64_PAGE_LEN, content_length % OTA_W25Q64_PAGE_LEN);
                                        }
                                    }
                                }
                            }
                            break;
                        }
                    }
                }

                if(j==4)
                {
                    printf("5 error\r\n");
                    return 0;
                }
            }
        }
    }
    return 1;
}

//检查新APP的一些数据是否正确
uint8_t OTA_CHECK_NEW_APP(void)
{
    //先校验新程序预期大小和实际大小是否相同
    if(target_version_size == APP_RealGet_Size)
    {
        //校验栈顶地址和复位地址
        uint8_t data[4]={0};
        W25Q64_ReadPage(OTA_APP_W25Q64_Start_Address, data, 4);
        uint32_t APP_Stack_ptr = data[0] | (data[1]<<8) | (data[2]<<16) | (data[3]<<24);
        W25Q64_ReadPage(OTA_APP_W25Q64_Start_Address+4, data, 4);
        uint32_t APP_Reset_Handler = data[0] | (data[1]<<8) | (data[2]<<16) | (data[3]<<24);

        if((APP_Stack_ptr & 0xFFFF0000) != 0x20000000)
        {
            printf("APP_Stack_ptr ERROR\r\n");
            return 0;
        }

        if(APP_Reset_Handler < OTA_APP_Start_Address || APP_Reset_Handler > OTA_APP_End_Address)
        {
            printf("APP_Reset_Handler ERROR\r\n");
            return 0;
        }

        return 1;
    }
    else
    {
        printf("APP_LEN ERROR\r\n");
        return 0;
    }
}

//复位程序
uint8_t OTA_Reset(void)
{
    //将APP大小和版本存到W25Q64中
    uint8_t APP_Message[12] = {(uint8_t)(target_version_size&0xFF), (uint8_t)((target_version_size>>8)&0xFF), 
                           (uint8_t)((target_version_size>>16)&0xFF), (uint8_t)((target_version_size>>24)&0xFF),
                           (uint8_t)(target_version_integer&0xFF), (uint8_t)((target_version_integer>>8)&0xFF), 
                           (uint8_t)((target_version_integer>>16)&0xFF), (uint8_t)((target_version_integer>>24)&0xFF),
                           (uint8_t)(target_version_decimals&0xFF), (uint8_t)((target_version_decimals>>8)&0xFF), 
                           (uint8_t)((target_version_decimals>>16)&0xFF), (uint8_t)((target_version_decimals>>24)&0xFF)};
    W25Q64_EraseSector(OTA_APP_LEN_ADDR);
    W25Q64_WritePage(OTA_APP_LEN_ADDR, APP_Message, sizeof(APP_Message));

    //重新报告版本，说明更新完成
    for(uint8_t i=0; i<3; i++)
    {
        if(Report_Version(target_version_integer, target_version_decimals))
        {
            break;
        }

        if(i==2)
        {
            return 0;
        }
    }

    //断开TCP连接
    for(uint8_t i=0; i<3; i++)
    {
        if(OTA_TCP_DISCONNECT()==0)
        {
            break;
        }

        if(i==2)
        {
            return 0;
        }
    }
    
    //确认APP正确后将更新标志位存入W25Q64
    uint8_t UpdateData[3]={OTA_APP_UPDATE, (uint8_t)(OTA_APP_KEY&0xFF), (uint8_t)((OTA_APP_KEY>>8)&0xFF)};
    W25Q64_EraseSector(OTA_APP_UPDATE_FLAG_ADDR);
    W25Q64_WritePage(OTA_APP_UPDATE_FLAG_ADDR, UpdateData, sizeof(UpdateData));
    //复位
    printf("start reset\r\n");
    NVIC_SystemReset();
    return 1;
}

void OTA_Run(void)
{
    uint8_t i=0;
    switch(OTA_State)
    {
        case OTA_IDLE:
            if(HAL_GetTick()-OTA_LastTime >= OTA_CHECK_UPDATE_TIME)
            {
                MQTT_ConnectState = 1;//表示MQTT连接断开
                ESP01S_PROTOCOL_FLAG = 0;//改为TCP协议
                ESP01S_REC_BUFF_FLAG = 0;//使用缓冲区接收数据
                OTA_State = OTA_INIT;
            }
            break;
        case OTA_INIT:    //先进行初始化，连接到onenet
            for(i=0;i<3;i++)
            {
                if(OTA_Init()==0)
                {
                    OTA_State = OTA_REPORT_VERSION;
                    break;
                }

                if(i==2)
                {
                    OTA_State = OTA_FINISH;
                    printf("OTA INIT ERROR\r\n");
                }
            }
            break;
        case OTA_REPORT_VERSION:  //上报版本
            for(i=0;i<3;i++)
            {
                if(OTA_ReportVersion())
                {
                    OTA_State = OTA_CHECK_UPDATE;
                    break;
                }

                if(i==2)
                {
                    OTA_State = OTA_FINISH;
                    printf("OTA Report Version ERROR\r\n");
                }
            }
            break;
        case OTA_CHECK_UPDATE:    //检查是否有更新
            for(i=0;i<3;i++)
            {
				uint8_t CheckUpdate_State = OTA_CheckUpdate();
                if(CheckUpdate_State==1)
                {
                    printf("OTA CHECK UPDATE OK\r\n");
                    OTA_State = OTA_APP_DOWN;//跳转到下载状态
                    break;
                }
				else if(CheckUpdate_State==2)
				{
					printf("OTA CHECK NO UPDATE\r\n");
                    OTA_State = OTA_FINISH;//跳转到结束状态
                    break;
				}

                if(i==2)
                {
                    OTA_State = OTA_FINISH;
                    printf("OTA Check Update ERROR\r\n");
                }
            }
            break;
        case OTA_APP_DOWN:  //下载新程序
            if(OTA_DOWNLOAD_NEW_APP())//成功接收新程序
            {
                printf("APP DOWN OK\r\n");
                OTA_State = OTA_APP_CHECK;
            }
            else    //失败
            {
                printf("APP DOWN ERROR\r\n");
                OTA_State = OTA_FINISH;
            }
            break;
        case OTA_APP_CHECK:
            if(OTA_CHECK_NEW_APP())//校验成功
            {
                printf("APP CHECK OK\r\n");
                OTA_State = OTA_RESET;
            }
            else //失败
            {
                printf("APP CHECK ERROR\r\n");
                OTA_State = OTA_FINISH;
            }
            break;
        case OTA_RESET:
            if(OTA_Reset()==0)
            {
                printf("APP RESET ERROR\r\n");
                OTA_State = OTA_FINISH;
            }
            break;
        case OTA_FINISH:   //无论是否成功，都会来到这个状态做最后处理
            OTA_TCP_DISCONNECT();//返回值不重要，为0时代表断开成功，为1时代表之前已经断开了
            APP_RealGet_Size = 0;//清除APP实际接收到的大小
            ESP01S_PROTOCOL_FLAG = 1;//将连接协议改为MQTT
            OTA_LastTime = HAL_GetTick();//记录下此时时间
            OTA_State = OTA_IDLE;
            break;
        default:
            break;
    }
}

//计算变量长度
uint8_t get_length_lookup(uint16_t num) 
{
    if (num < 10) return 1;
    if (num < 100) return 2;
    if (num < 1000) return 3;
    if (num < 10000) return 4;
    return 5;  // 10000到65535都是5位数
}

