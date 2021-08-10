/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : key.h
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
#ifndef __bsp_KEY_H
#define __bsp_KEY_H	 

#include "sys.h" 
#include "delay.h"

#if 0



/*下面的方式是通过直接操作库函数方式读取IO*/
#define KEY0 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4) //PE4
#define KEY1 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)	//PE3 
#define KEY2 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2) //PE2
#define WK_UP 	    GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)	//PA0



#define KEY_NONE        0   //无按键
#define KEY_SET_PRES 	1	//KEY0按下
#define KEY_RR_PRES	    2	//KEY1按下
#define KEY_LL_PRES	    3	//KEY2按下
#define KEY_OK_PRES     4	//KEY3按下



#define KEY0_PRES 	1
#define KEY1_PRES	2
#define KEY2_PRES	3
#define WKUP_PRES   4

void bsp_key_Init(void);	//IO初始化
uint8_t bsp_Key_Scan(u8);  		//按键扫描函数	
#endif

#define KEY_NONE        0   //无按键
#define KEY_SET_PRES 	1	//KEY0按下
#define KEY_RR_PRES	    2	//KEY1按下
#define KEY_LL_PRES	    3	//KEY2按下
#define KEY_OK_PRES     4	//KEY3按下




//宏定义
#define    	KEY_OFF	   		1
#define    	KEY_ON	   	 	0
#define    	KEY_HOLD		2
#define		KEY_IDLE		3
#define		KEY_ERROR		10

#define		HOLD_COUNTS			50
#define 	SHAKES_COUNTS		5


//按键状态结构体，存储四个变量
typedef struct
{
 	uint8_t KeyLogic;
	uint8_t KeyPhysic;
 	uint8_t KeyONCounts;
 	uint8_t KeyOFFCounts;
}KEY_TypeDef;







void bsp_key_Init(void);	//IO初始化
//uint8_t bsp_key_Scan(uint8_t);  		//按键扫描函数		
uint8_t Key_Scan(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);



#endif

