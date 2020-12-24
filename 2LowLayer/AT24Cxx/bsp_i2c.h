/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp_i2c.h
  版 本 号   : 初稿
  作    者   : armfly
  生成日期   : 2020年1月16日
  最近修改   :
  功能描述   : bsp_i2c.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年1月16日
    作    者   : armfly
    修改内容   : 创建文件

******************************************************************************/
#ifndef __BSP_I2C_H__
#define __BSP_I2C_H__

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "stm32f4xx.h" 

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define I2C_WR	0		/* 写控制bit */
#define I2C_RD	1		/* 读控制bit */

/* 定义I2C总线连接的GPIO端口 */
#define GPIO_PORT_I2C	GPIOF			/* GPIO端口 */
#define RCC_I2C_PORT 	RCC_AHB1Periph_GPIOF		/* GPIO端口时钟 */
#define I2C_SCL_PIN		GPIO_Pin_1			/* 连接到SCL时钟线的GPIO */
#define I2C_SDA_PIN		GPIO_Pin_0			/* 连接到SDA数据线的GPIO */

        
/* 定义读写SCL和SDA的宏 */
#define I2C_SCL_1()  GPIO_SetBits(GPIO_PORT_I2C, I2C_SCL_PIN)				/* SCL = 1 */
#define I2C_SCL_0()  GPIO_ResetBits(GPIO_PORT_I2C, I2C_SCL_PIN)			    /* SCL = 0 */
        
#define I2C_SDA_1()  GPIO_SetBits(GPIO_PORT_I2C, I2C_SDA_PIN)				/* SDA = 1 */
#define I2C_SDA_0()  GPIO_ResetBits(GPIO_PORT_I2C, I2C_SDA_PIN)				/* SDA = 0 */
        
#define I2C_SDA_READ()  GPIO_ReadInputDataBit(GPIO_PORT_I2C, I2C_SDA_PIN)	/* 读SDA口线状态 */
#define I2C_SCL_READ()  GPIO_ReadInputDataBit(GPIO_PORT_I2C, I2C_SCL_PIN)	/* 读SCL口线状态 */

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/


void bsp_initI2C(void);
void i2c_Start(void);
void i2c_Stop(void);
void i2c_SendByte(uint8_t _ucByte);
uint8_t i2c_ReadByte(void);
uint8_t i2c_WaitAck(void);
void i2c_Ack(void);
void i2c_NAck(void);
uint8_t i2c_CheckDevice(uint8_t _Address);



#endif /* __BSP_I2C_H__ */
