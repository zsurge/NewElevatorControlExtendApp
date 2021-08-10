/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : key.c
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

#include "bsp_key.h"

//�����ṹ�����飬��ʼ״̬���ǹر�
static KEY_TypeDef Key[4] =
	{{KEY_OFF, KEY_OFF, 0, 0},
	 {KEY_OFF, KEY_OFF, 0, 0},
     {KEY_OFF, KEY_OFF, 0, 0},
     {KEY_OFF, KEY_OFF, 0, 0}};


void bsp_key_Init(void)
{
	
      GPIO_InitTypeDef  GPIO_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOE, ENABLE);//ʹ��GPIOA,GPIOEʱ��
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4; //KEY0 KEY1 KEY2��Ӧ����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
    GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4
      
       
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//WK_UP��Ӧ����PA0
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;//����
    GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA0

} 



/*
 * ��������Key_Scan
 * ����  ������Ƿ��а�������
 * ����  ��GPIOx��gpio��port
 *		   GPIO_Pin��gpio��pin
 * ���  ��KEY_OFF��KEY_ON��KEY_HOLD��KEY_IDLE��KEY_ERROR
 */
 
uint8_t Key_Scan(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	KEY_TypeDef *KeyTemp;

	//��鰴�µ�����һ����ť
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
        		//port��pin��ƥ��
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

        		//port��pin��ƥ��
        		default:
        			//printf("error: GPIO port pin not match\r\n");
        			return KEY_IDLE;
    		}
    		break;

    	default:
    		//printf("error: key do not exist\r\n");
    		return KEY_IDLE;
	}

	/* ��ⰴ�¡��ɿ������� */
	KeyTemp->KeyPhysic = GPIO_ReadInputDataBit(GPIOx, GPIO_Pin);

	switch (KeyTemp->KeyLogic)
	{
	
	case KEY_ON:
		switch (KeyTemp->KeyPhysic)
		{
		
		//��1��1���н��رռ������㣬���Կ��������ۼ�ֱ���л����߼�����״̬
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
			
		//��1��0���жԹرռ����ۼ�ֱ���л����߼��ر�״̬
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
		
		//��0��1���жԿ��������ۼ�ֱ���л����߼�����״̬
		case KEY_ON:
			(KeyTemp->KeyONCounts)++;
			if (KeyTemp->KeyONCounts >= SHAKES_COUNTS)
			{
				KeyTemp->KeyLogic = KEY_ON;
				KeyTemp->KeyONCounts = 0;

				return KEY_ON;
			}
			return KEY_IDLE;
			
		//��0��0���н�������������
		case KEY_OFF:
			(KeyTemp->KeyONCounts) = 0;
			return KEY_IDLE;
		default:
			break;
		}

	case KEY_HOLD:
		switch (KeyTemp->KeyPhysic)
		{
		
		//��2��1���Թرռ�������
		case KEY_ON:
			KeyTemp->KeyOFFCounts = 0;
			return KEY_HOLD;
		//��2��0���Թرռ����ۼ�ֱ���л����߼��ر�״̬
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
	
	//һ�㲻�ᵽ����
	return KEY_ERROR;
}



