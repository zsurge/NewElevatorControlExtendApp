/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : key.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��5��25��
  ����޸�   :
  ��������   : ��������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��5��25��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __bsp_KEY_H
#define __bsp_KEY_H	 

#include "sys.h" 
#include "delay.h"

#if 0



/*����ķ�ʽ��ͨ��ֱ�Ӳ����⺯����ʽ��ȡIO*/
#define KEY0 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4) //PE4
#define KEY1 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)	//PE3 
#define KEY2 		GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2) //PE2
#define WK_UP 	    GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)	//PA0



#define KEY_NONE        0   //�ް���
#define KEY_SET_PRES 	1	//KEY0����
#define KEY_RR_PRES	    2	//KEY1����
#define KEY_LL_PRES	    3	//KEY2����
#define KEY_OK_PRES     4	//KEY3����



#define KEY0_PRES 	1
#define KEY1_PRES	2
#define KEY2_PRES	3
#define WKUP_PRES   4

void bsp_key_Init(void);	//IO��ʼ��
uint8_t bsp_Key_Scan(u8);  		//����ɨ�躯��	
#endif

#define KEY_NONE        0   //�ް���
#define KEY_SET_PRES 	1	//KEY0����
#define KEY_RR_PRES	    2	//KEY1����
#define KEY_LL_PRES	    3	//KEY2����
#define KEY_OK_PRES     4	//KEY3����




//�궨��
#define    	KEY_OFF	   		1
#define    	KEY_ON	   	 	0
#define    	KEY_HOLD		2
#define		KEY_IDLE		3
#define		KEY_ERROR		10

#define		HOLD_COUNTS			50
#define 	SHAKES_COUNTS		5


//����״̬�ṹ�壬�洢�ĸ�����
typedef struct
{
 	uint8_t KeyLogic;
	uint8_t KeyPhysic;
 	uint8_t KeyONCounts;
 	uint8_t KeyOFFCounts;
}KEY_TypeDef;







void bsp_key_Init(void);	//IO��ʼ��
//uint8_t bsp_key_Scan(uint8_t);  		//����ɨ�躯��		
uint8_t Key_Scan(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);



#endif

