// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (c) 2008-2023 100askTeam : Dongshan WEI <weidongshan@qq.com> 
 * Discourse:  https://forums.100ask.net
 */

 
/*  Copyright (C) 2008-2023 ���ڰ������Ƽ����޹�˾
 *  All rights reserved
 *
 *
 * ��������: ��������д���ĵ�������ѧԱѧϰʹ�ã�����ת��������(�뱣��������Ϣ)����ֹ������ҵ��;��
 * ��������: ��������д�ĳ��򣬿���������ҵ��;�������������е��κκ����
 * 
 * 
 * ��������ѭGPL V3Э�飬ʹ������ѭЭ������
 * ���������õĿ����壺	DShanMCU-F103
 * ������Ƕ��ʽѧϰƽ̨��https://www.100ask.net
 * ��������������������	https://forums.100ask.net
 * �������ٷ�Bվ��				https://space.bilibili.com/275908810
 * �������ٷ��Ա���			https://100ask.taobao.com
 * ��ϵ����(E-mail)��	  weidongshan@qq.com
 *
 * ��Ȩ���У�����ؾ���
 *  
 * �޸���ʷ     �汾��           ����        �޸�����
 *-----------------------------------------------------
 * 2023.08.04      v01         ���ʿƼ�      �����ļ�
 *-----------------------------------------------------
 */


#include "driver_timer.h"
#include "stm32f1xx_hal.h"

/**********************************************************************
 * �������ƣ� udelay
 * ���������� us�������ʱ����(����rt-thread�Ĵ���)
 * ��������� us - ��ʱ����us
 * ��������� ��
 * �� �� ֵ�� ��
 * �޸�����        �汾��     �޸���	      �޸�����
 * -----------------------------------------------
 * 2023/08/03	     V1.0	  Τ��ɽ	      ����
 ***********************************************************************/
void udelay(int us)
{
#if 0    
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;

    ticks = us * reload / (1000);  /* ����reload��Ӧ1ms */
    told = SysTick->VAL;
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
#else
    extern TIM_HandleTypeDef        htim4;
    TIM_HandleTypeDef *hHalTim = &htim4;

    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = __HAL_TIM_GET_AUTORELOAD(hHalTim);

    ticks = us * reload / (1000);  /* ����reload��Ӧ1ms */
    told = __HAL_TIM_GET_COUNTER(hHalTim);
    while (1)
    {
        tnow = __HAL_TIM_GET_COUNTER(hHalTim);
        if (tnow != told)
        {
            if (tnow > told)
            {
                tcnt += tnow - told;
            }
            else
            {
                tcnt += reload - told + tnow;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
#endif
}


/**********************************************************************
 * �������ƣ� mdelay
 * ���������� ms�������ʱ����
 * ��������� ms - ��ʱ����ms
 * ��������� ��
 * �� �� ֵ�� ��
 * �޸�����        �汾��     �޸���	      �޸�����
 * -----------------------------------------------
 * 2023/08/03	     V1.0	  Τ��ɽ	      ����
 ***********************************************************************/
void mdelay(int ms)
{
    for (int i = 0; i < ms; i++)
        udelay(1000);
}

/**********************************************************************
 * �������ƣ� system_get_ns
 * ���������� ���ϵͳʱ��(��λns)
 * ��������� ��
 * ��������� ��
 * �� �� ֵ�� ϵͳʱ��(��λns)
 * �޸�����        �汾��     �޸���	      �޸�����
 * -----------------------------------------------
 * 2023/08/03	     V1.0	  Τ��ɽ	      ����
 ***********************************************************************/
uint64_t system_get_ns(void)
{
    //extern uint32_t HAL_GetTick(void);
    extern TIM_HandleTypeDef        htim4;
    TIM_HandleTypeDef *hHalTim = &htim4;
    
    uint64_t ns = HAL_GetTick();
    uint64_t cnt;
    uint64_t reload;

    cnt = __HAL_TIM_GET_COUNTER(hHalTim);
    reload = __HAL_TIM_GET_AUTORELOAD(hHalTim);

    ns *= 1000000;
    ns += cnt * 1000000 / reload;
    return ns;
}

// unsigned long getRunTimeCounterValue(void)
// {
//     return system_get_ns()/1000;
// }


