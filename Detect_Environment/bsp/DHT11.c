#include "DHT11.h"

uint8_t DHT11_TEMP_DATA[DHT11_DATA_NUM] = {0};//存储温度
uint8_t DHT11_HUM_DATA[DHT11_DATA_NUM] = {0};//存储湿度

void DHT11_W_Pin(uint8_t PinState)
{
	if(PinState)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	}
}

uint8_t DHT11_R_Pin(void)
{
	uint8_t PinState;
	
	if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET)
	{
		PinState=1;
	}
	else 
	{
		PinState=0;
	}
	
	return PinState;
}

void DHT11_OFF(void)
{
	DHT11_W_Pin(1);
}

void DHT11_Start(void)
{
	DHT11_W_Pin(1);
	udelay(10);
	//发送复位信号
	DHT11_W_Pin(0);
	mdelay(20);
	//Delay_ms(20);
	DHT11_W_Pin(1);
}

uint8_t DHT11_Wait(uint8_t PinState)
{
	uint16_t timeout=5000;
	while(DHT11_R_Pin()!=PinState)
	{
		timeout--;
		if(timeout==0)
		{
			return 0;
		}
	}
	return 1;
}

uint8_t DHT11_ReadData(void)
{
	uint8_t Data=0,i;
	for(i=0;i<8;i++)
	{
		DHT11_Wait(1);
		udelay(40);
		//Delay_us(40);
		if(DHT11_R_Pin())
		{
			Data |= (1<<(8-(i+1)));
			DHT11_Wait(0);
		}
	}
	return Data;
}

uint8_t DHT11_GetData(uint8_t *humidity, uint8_t *temperature)
{
	uint8_t Data[5],i;
	
	//发送复位信号
	DHT11_Start();
	udelay(30);
	//Delay_us(30);
	
	if(DHT11_R_Pin()==0)
	{
		//等待响应信号
//		while(DHT11_R_Pin()==0);
//		while(DHT11_R_Pin()==1);
		DHT11_Wait(1);
		DHT11_Wait(0);
		
		//接收数据
		for(i=0;i<5;i++)
		{
			Data[4-i] = DHT11_ReadData();
		}
		
		//DHT11_W_Pin(1);
		
		if(Data[4] + Data[3] + Data[2] + Data[1] == Data[0])
		{
			*humidity = Data[4];
			*temperature = Data[2];
		
			return 0;
		}
		else
		{
			return 2;
		}
	}
	else
	{
		return 1;
	}
}

void DHT11_Init(void)
{
	uint8_t hum=0,tem=0;

	mdelay(1000);//延时1s等待DHT11稳定

	taskENTER_CRITICAL();
	for(uint8_t i=0;i<2; i++)
	{
		while(DHT11_GetData(&hum, &tem)!=0);//连续读取两次才能得到实时数据
	}
	taskEXIT_CRITICAL();

	// printf("hum=%d%%, tem=%dC\r\n", hum, tem);

	for(uint8_t i=0;i<DHT11_DATA_NUM; i++)
	{
		DHT11_HUM_DATA[i] = hum;
		DHT11_TEMP_DATA[i] = tem;
	}
}

//获取成功获得的数据的平均值
void DHT11_Run(uint8_t *humidity, uint8_t *temperature)
{
	static uint8_t DHT11_NUM=0;//记录存到第几个数据
	
	if(DHT11_GetData(&DHT11_HUM_DATA[DHT11_NUM], &DHT11_TEMP_DATA[DHT11_NUM])==0)
	{
		//采集成功
		DHT11_NUM = (DHT11_NUM + 1) % DHT11_DATA_NUM;
	}

	//临时数组，用于排序，取中位数
	uint8_t temp_sorted[DHT11_DATA_NUM]={0};
	uint8_t hum_sorted[DHT11_DATA_NUM]={0};

	//将数据复制到临时数组中
	memcpy(temp_sorted, DHT11_TEMP_DATA, DHT11_DATA_NUM);
	memcpy(hum_sorted, DHT11_HUM_DATA, DHT11_DATA_NUM);

	//冒泡排序
	for(uint8_t i=0; i<DHT11_DATA_NUM-1; i++)
	{
		for(uint8_t j=0; j<DHT11_DATA_NUM-1-i; j++)
		{
			if(temp_sorted[j] > temp_sorted[j+1])
			{
				uint8_t temp = temp_sorted[j];
				temp_sorted[j] = temp_sorted[j+1];
				temp_sorted[j+1] = temp;
			}
			
			if(hum_sorted[j] > hum_sorted[j+1])
			{
				uint8_t temp = hum_sorted[j];
				hum_sorted[j] = hum_sorted[j+1];
				hum_sorted[j+1] = temp;
			}
		}
	}
	
	*humidity = hum_sorted[DHT11_DATA_NUM/2];
	*temperature = temp_sorted[DHT11_DATA_NUM/2];
}
