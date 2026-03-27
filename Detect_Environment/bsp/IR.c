#include "IR.h"

uint8_t IR_State;//是否有按键按下
uint8_t IR_Data[4];//按键键值
uint8_t IR_RepeatFlag;//重复标志位

void IR_Init(void)
{
	HAL_TIM_Base_Start(&htim1);
}

uint8_t IR_GetData(void)
{
	if(IR_State)
	{
		IR_State=0;
		return IR_Data[2];
	}
	else
	{
		return 0;
	}
}

uint8_t IR_GetRepeatFlag(void)
{
	return IR_RepeatFlag;
}

void IR_EXTI_Callback(void)
{
	static uint8_t s=0, IRNum;
	uint64_t a;
	uint8_t i;
	
	if(s==0)
	{
		__HAL_TIM_SET_COUNTER(&htim1, 0);
		s=1;
	}
	else if(s==1)
	{
		a=__HAL_TIM_GET_COUNTER(&htim1);
		__HAL_TIM_SET_COUNTER(&htim1, 0);
		
		if(a>=13500-500 && a<= 13500+500)
		{		
			s=2;
		}
		else if((a>=11500-500 && a<= 11500+500))
		{
			IR_State = 1;
			IR_RepeatFlag = 1;
			s=0;
		}
		else//出错时继续等待起始信号
		{
			IR_State = 0;
		}
	}
	else if(s==2)
	{
		a=__HAL_TIM_GET_COUNTER(&htim1);
		__HAL_TIM_SET_COUNTER(&htim1, 0);
		
		if(a>=2240-500 && a<=2240+500)
		{
			IR_Data[IRNum/8] |= 1 << (7-IRNum%8);
			IRNum++;
		}
		else if(a>=1120-500 && a<=1120+500)
		{
			IR_Data[IRNum/8] &= ~(1 << (7-IRNum%8));
			IRNum++;
		}
		else
		{
			IRNum = 0;
			IR_State = 0;
			
			for(i=0;i<4;i++)
			{
				IR_Data[i] = 0;
			}
			
			s=0;
		}
		
		if(IRNum >= 32)
		{
			//~运算会将数据转为int类型
			if((IR_Data[0] == (uint8_t)(~IR_Data[1])) && (IR_Data[2] == (uint8_t)(~IR_Data[3])))
			{
				IR_State = 1;
				IR_RepeatFlag = 0;
			}
			else
			{
				for(i=0;i<4;i++)
				{
					IR_Data[i] = 0;
				}
				IR_State = 0;
			}
			
			IRNum = 0;
			s=0;
		}
	}
}
