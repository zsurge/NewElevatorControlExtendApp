/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : key.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月25日
  最近修改   :
  功能描述   : 键盘驱动
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月25日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

#include "bsp_key.h"

//按键结构体数组，初始状态都是关闭
static KEY_TypeDef Key[4] =
	{{KEY_OFF, KEY_OFF, 0, 0},
	 {KEY_OFF, KEY_OFF, 0, 0},
     {KEY_OFF, KEY_OFF, 0, 0},
     {KEY_OFF, KEY_OFF, 0, 0}};


void bsp_key_Init(void)
{
	
      GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOA,GPIOE时钟
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4; //KEY0 KEY1 KEY2对应引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//普通输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIOE2,3,4
      
       
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//WK_UP对应引脚PA0
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;//下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA0

} 



/*
 * 函数名：Key_Scan
 * 描述  ：检测是否有按键按下
 * 输入  ：GPIOx：gpio的port
 *		   GPIO_Pin：gpio的pin
 * 输出  ：KEY_OFF、KEY_ON、KEY_HOLD、KEY_IDLE、KEY_ERROR
 */
 
uint8_t Key_Scan(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	KEY_TypeDef *KeyTemp;

	//检查按下的是哪一个按钮
	switch ((uint32_t)GPIOx)
	{
    	case ((uint32_t)GPIOE):
    		switch (GPIO_Pin)
    		{
        		case GPIO_Pin_2:
        			KeyTemp = &Key[0];
        			break;	
        		case GPIO_Pin_3:
        			KeyTemp = &Key[1];
        			break;	
        		case GPIO_Pin_4:
        			KeyTemp = &Key[2];
        			break;
        		//port和pin不匹配
        		default:
        			//printf("error: GPIO port pin not match\r\n");
        			return KEY_IDLE;
    		}
    		break;

    	case ((uint32_t)GPIOA):
    		switch (GPIO_Pin)
    		{
        		case GPIO_Pin_0:
        			KeyTemp = &Key[3];
        			break;				

        		//port和pin不匹配
        		default:
        			//printf("error: GPIO port pin not match\r\n");
        			return KEY_IDLE;
    		}
    		break;

    	default:
    		//printf("error: key do not exist\r\n");
    		return KEY_IDLE;
	}

	/* 检测按下、松开、长按 */
	KeyTemp->KeyPhysic = GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);

	switch (KeyTemp->KeyLogic)
	{
	
	case KEY_ON:
		switch (KeyTemp->KeyPhysic)
		{
		
		//（1，1）中将关闭计数清零，并对开启计数累加直到切换至逻辑长按状态
		case KEY_ON:
			KeyTemp->KeyOFFCounts = 0;
			KeyTemp->KeyONCounts++;
			if (KeyTemp->KeyONCounts >= HOLD_COUNTS)
			{
				KeyTemp->KeyONCounts = 0;
				KeyTemp->KeyLogic = KEY_HOLD;
				return KEY_HOLD;
			}
			return KEY_IDLE;
			
		//（1，0）中对关闭计数累加直到切换至逻辑关闭状态
		case KEY_OFF:
			KeyTemp->KeyOFFCounts++;
			if (KeyTemp->KeyOFFCounts >= SHAKES_COUNTS)
			{
				KeyTemp->KeyLogic = KEY_OFF;
				KeyTemp->KeyOFFCounts = 0;
				return KEY_OFF;
			}
			return KEY_IDLE;

		default:
			break;
		}

	case KEY_OFF:
		switch (KeyTemp->KeyPhysic)
		{
		
		//（0，1）中对开启计数累加直到切换至逻辑开启状态
		case KEY_ON:
			(KeyTemp->KeyONCounts)++;
			if (KeyTemp->KeyONCounts >= SHAKES_COUNTS)
			{
				KeyTemp->KeyLogic = KEY_ON;
				KeyTemp->KeyONCounts = 0;

				return KEY_ON;
			}
			return KEY_IDLE;
			
		//（0，0）中将开启计数清零
		case KEY_OFF:
			(KeyTemp->KeyONCounts) = 0;
			return KEY_IDLE;
		default:
			break;
		}

	case KEY_HOLD:
		switch (KeyTemp->KeyPhysic)
		{
		
		//（2，1）对关闭计数清零
		case KEY_ON:
			KeyTemp->KeyOFFCounts = 0;
			return KEY_HOLD;
		//（2，0）对关闭计数累加直到切换至逻辑关闭状态
		case KEY_OFF:
			(KeyTemp->KeyOFFCounts)++;
			if (KeyTemp->KeyOFFCounts >= SHAKES_COUNTS)
			{
				KeyTemp->KeyLogic = KEY_OFF;
				KeyTemp->KeyOFFCounts = 0;
				return KEY_OFF;
			}
			return KEY_IDLE;

		default:
			break;
		}

	default:
		break;
	}
	
	//一般不会到这里
	return KEY_ERROR;
}



