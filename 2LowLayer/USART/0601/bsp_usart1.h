/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp_usart1.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月22日
  最近修改   :
  功能描述   : 串口1初始化
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

#ifndef __BSP_USART1_H
#define __BSP_USART1_H
#include "stdio.h"	
#include "string.h"
#include "stm32f4xx_conf.h"
#include "stdarg.h"


//引脚定义
/*******************************************************/
#define USART6_CLK                         RCC_APB2Periph_USART6
#define USART6_GPIO_PORT                   GPIOC
#define USART6_GPIO_CLK                    RCC_AHB1Periph_GPIOC

#define USART6_RX_PIN                      GPIO_Pin_7
#define USART6_RX_AF                       GPIO_AF_USART6
#define USART6_RX_SOURCE                   GPIO_PinSource7

#define USART6_TX_PIN                      GPIO_Pin_6
#define USART6_TX_AF                       GPIO_AF_USART6
#define USART6_TX_SOURCE                   GPIO_PinSource6


#define USART1MAXBUFFSIZE 512 

extern volatile uint8_t USART6RecvBuf[USART1MAXBUFFSIZE];
extern volatile uint8_t USART6SendBuf[USART1MAXBUFFSIZE];

extern volatile uint16_t RecvTop6;
extern volatile uint16_t RecvEnd6;

#define RS485_U6_RX_EN()	    GPIOA->BSRRH = GPIO_Pin_8
#define RS485_U6_TX_EN()	    GPIOA->BSRRL = GPIO_Pin_8


void bsp_Usart6_Init (uint32_t BaudRate);
void bsp_Usart6_SendString (const uint8_t *Buff);
void bsp_Usart6_SendData (const uint8_t *Buff, uint16_t SendSize);
void bsp_Usart6_RecvReset (void);
uint8_t bsp_Usart6_RecvOne (uint8_t *Str);
uint8_t bsp_Usart6_Read(uint8_t *buf, uint16_t len);

void bsp_Usart6_DMA_Send(uint8_t *buf, uint16_t len);

void SendBytesInfoProc(u8* pSendInfo, u16 nSendCount);

    
#endif


