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

#ifndef __BSP_USART6_H
#define __BSP_USART6_H
#include "stdio.h"	
#include "string.h"
#include "stm32f4xx_conf.h"
#include "sys.h"
#include "ringbuff.h"


//引脚定义
/*******************************************************/
#define USART1_CLK                         RCC_APB2Periph_USART6
#define USART1_GPIO_PORT                   GPIOC
#define USART1_GPIO_CLK                    RCC_AHB1Periph_GPIOC

#define USART1_RX_PIN                      GPIO_Pin_7
#define USART1_RX_AF                       GPIO_AF_USART6
#define USART1_RX_SOURCE                   GPIO_PinSource7

#define USART1_TX_PIN                      GPIO_Pin_6
#define USART1_TX_AF                       GPIO_AF_USART6
#define USART1_TX_SOURCE                   GPIO_PinSource6


#define COM6_RXBUFFER_SIZE   256 //串口接收缓存器长度
#define UART6_RX_LEN         COM6_RXBUFFER_SIZE //USART6 DMA接收缓存器长度
#define UART6_TX_LEN         COM6_RXBUFFER_SIZE //USART6 DMA发送缓存器长度

#define RS485_U6_RX_EN()	    {PAout(8)=0;} //SP485接收模式,低电平有效
#define RS485_U6_TX_EN()	    {PAout(8)=1;} //SP485发送模式,高电平有效

extern uint8_t gUsart6RXBuff[COM6_RXBUFFER_SIZE];//接收缓存器
extern RingBuff_t ringbuff_handle;


void BSP_Usart6_Init(uint32_t bound);
void BSP_DMAUsart6Send(uint8_t *str,uint8_t cndtr);
uint16_t BSP_DMAUsart6Read(uint8_t *buff, uint32_t len);
    
#endif


