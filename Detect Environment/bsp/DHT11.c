#include "DHT11.h"

uint8_t DHT11_TEMP_DATA[DHT11_DATA_NUM] = {0};//存储温度
uint8_t DHT11_HUM_DATA[DHT11_DATA_NUM] = {0};//存储湿度
uint8_t DHT11_NUM=0;//记录存到第几个数据

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
	uint16_t timeout=10000;
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
	DHT11_GetData(&hum, &tem);
	for(uint8_t i=0;i<DHT11_DATA_NUM; i++)
	{
		DHT11_HUM_DATA[i] = hum;
		DHT11_TEMP_DATA[i] = tem;
	}
}

//获取成功获得的数据的平均值
void DHT11_Run(uint8_t *humidity, uint8_t *temperature)
{
	uint16_t hum_sum=0,temp_sum=0;
	
	if(DHT11_GetData(&DHT11_HUM_DATA[DHT11_NUM], &DHT11_TEMP_DATA[DHT11_NUM])==0)
	{
		DHT11_NUM = (DHT11_NUM + 1) % DHT11_DATA_NUM;
	}
	
	for(uint8_t i=0; i<DHT11_DATA_NUM; i++)
	{
		hum_sum += DHT11_HUM_DATA[i];
		temp_sum += DHT11_TEMP_DATA[i];
	}
	
	*humidity = hum_sum / DHT11_DATA_NUM;
	*temperature = temp_sum / DHT11_DATA_NUM;
}
