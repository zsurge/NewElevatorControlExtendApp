/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp_usart3.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月22日
  最近修改   :
  功能描述   : 串口3 指令接收
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

#ifndef __BSP_USART5_H
#define __BSP_USART5_H

#include "stdio.h"	
#include "string.h"
#include "stm32f4xx_conf.h"

#define USART5_CLK                         RCC_APB1Periph_UART5
#define USART5_GPIO_RX_PORT                GPIOD
#define USART5_GPIO_RX_CLK                 RCC_AHB1Periph_GPIOD
#define USART5_GPIO_TX_PORT                GPIOC
#define USART5_GPIO_TX_CLK                 RCC_AHB1Periph_GPIOC

			
#define USART5_RX_PIN                      GPIO_Pin_2
#define USART5_RX_AF                       GPIO_AF_UART5
#define USART5_RX_SOURCE                   GPIO_PinSource2


#define USART5_TX_PIN                      GPIO_Pin_12
#define USART5_TX_AF                       GPIO_AF_UART5
#define USART5_TX_SOURCE                   GPIO_PinSource12


#define RS485_U5_RX_EN()	    GPIOD->BSRRH = GPIO_Pin_7
#define RS485_U5_TX_EN()	    GPIOD->BSRRL = GPIO_Pin_7

#define USART5MAXBUFFSIZE 1024




void bsp_Usart5_Init (uint32_t BaudRate);
void bsp_Usart5_SendString (const uint8_t *Buff);
void bsp_Usart5_SendData (const uint8_t *Buff, uint16_t SendSize);
void bsp_Usart5_RecvReset (void);
uint8_t bsp_Usart5_RecvOne (uint8_t *Str);
uint8_t bsp_Usart5_Read(uint8_t *buf, uint16_t len);

    
#endif


