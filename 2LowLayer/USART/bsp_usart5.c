/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp_Usart3.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月22日
  最近修改   :
  功能描述   : 串口3 IAP升级
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

#include "bsp_usart5.h"
#include "bsp_time.h"

volatile uint8_t USART5RecvBuf[USART5MAXBUFFSIZE] = {0};
volatile uint16_t RecvTop5 = 0;
volatile uint16_t RecvEnd5 = 0;





/*****************************************************************************
 函 数 名  : bsp_Usart5Init
 功能描述  : 串口初始化
 输入参数  : uint32_t BaudRate  波特率
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
void bsp_Usart3Init (uint32_t BaudRate);
    修改内容   : 新生成函数

*****************************************************************************/
void bsp_Usart5_Init (uint32_t BaudRate)
{
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;


	RCC_APB1PeriphClockCmd(USART5_CLK,ENABLE); //使能GPIOA时钟
	RCC_AHB1PeriphClockCmd(USART5_GPIO_RX_CLK|USART5_GPIO_TX_CLK,ENABLE);//使能Usart5时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(USART5_GPIO_TX_PORT,USART5_TX_SOURCE,USART5_TX_AF); //GPIOB10复用为USART3
	GPIO_PinAFConfig(USART5_GPIO_RX_PORT,USART5_RX_SOURCE,USART5_RX_AF); //GPIOB11复用为USART3
	
	//Usart2端口配置
	GPIO_InitStructure.GPIO_Pin = USART5_RX_PIN; //GPIOB10与GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(USART5_GPIO_RX_PORT,&GPIO_InitStructure); //初始化PB10，PB11

	GPIO_InitStructure.GPIO_Pin = USART5_TX_PIN; //GPIOB10与GPIOB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(USART5_GPIO_TX_PORT,&GPIO_InitStructure); //初始化PB10，PB11


	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* 无上拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOD, &GPIO_InitStructure);   


   //USART3 初始化设置
	USART_InitStructure.USART_BaudRate = BaudRate;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(UART5, &USART_InitStructure); //初始化串口2    

	//USART3 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;//串口3中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//开启相关中断    
	
	USART_Cmd(UART5, ENABLE);  //使能串口1   


	RS485_U5_RX_EN();

}

void UART5_IRQHandler (void)
{
	uint8_t temp = 0;

	//Receive data register not empty flag
	if (USART_GetITStatus (UART5, USART_IT_RXNE) != RESET)
	{
		temp = USART_ReceiveData (UART5);		//get received data

		if ( (RecvEnd5 == (RecvTop5 - 1) ) || ( (RecvTop5 == 0) && (RecvEnd5 == (USART5MAXBUFFSIZE - 1) ) ) )
		{
			//缓冲溢出
			RecvTop5 = 0;
			RecvEnd5 = 0;
		}
		else
		{
			USART5RecvBuf[RecvEnd5] = temp;
			RecvEnd5++;

			if (RecvEnd5 >= USART5MAXBUFFSIZE) RecvEnd5 = 0;
		}

		/* Clear the UART1 Recvive interrupt */
		USART_ClearITPendingBit (UART5, USART_IT_RXNE);
	}
	else if (USART_GetITStatus (UART5, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE) != RESET)
	{
		USART_GetITStatus (UART5, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE);
		USART_ReceiveData (UART5);
	}
}


void bsp_Usart5_SendOne (const uint8_t dat)
{
    RS485_U5_TX_EN();
    
	USART_SendData (UART5, dat);

	while (USART_GetFlagStatus (UART5, USART_FLAG_TXE) == RESET) {}

	USART_ClearFlag (UART5, USART_FLAG_TXE);

    RS485_U5_RX_EN();
}



/*****************************************************************************
 函 数 名  : bsp_Usart3SendString
 功能描述  : 发送字符串
 输入参数  : const uint8_t *Buff  发送字符串
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
void bsp_Usart3SendString (const uint8_t *Buff);
    修改内容   : 新生成函数

*****************************************************************************/
void bsp_Usart5_SendString (const uint8_t *Buff)
{
	while (*Buff != 0)
	{
		bsp_Usart5_SendOne (*Buff);
		Buff++;
	}    
}


/*****************************************************************************
 函 数 名  : bsp_Usart3SendData
 功能描述  : 发送指定长度字符
 输入参数  : const uint8_t *Buff  发送数据的buff
             uint16_t SendSize  发送数据的长度  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
void bsp_Usart3SendData (const uint8_t *Buff, uint16_t SendSize);
    修改内容   : 新生成函数

*****************************************************************************/
void bsp_Usart5_SendData (const uint8_t *Buff, uint16_t SendSize)
{
	while (SendSize != 0)
	{
		bsp_Usart5_SendOne (*Buff);
		Buff++;
		SendSize--;
	}

}



/*****************************************************************************
 函 数 名  : bsp_Usart3RecvReset
 功能描述  : 清空串口接收缓冲
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
void bsp_Usart3RecvReset (void);
    修改内容   : 新生成函数

*****************************************************************************/
void bsp_Usart5_RecvReset (void)
{
	RecvTop5 = 0;
	RecvEnd5 = 0;

	memset((void *)USART5RecvBuf, 0, USART5MAXBUFFSIZE); 
}



/*****************************************************************************
 函 数 名  : bsp_Usart3RecvOne
 功能描述  : 读取1个字节
 输入参数  : uint8_t *Str  读出的字节存储在Str[0]
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
uint8_t bsp_Usart3RecvOne (uint8_t *Str);
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t bsp_Usart5_RecvOne (uint8_t *Str)
{
	if (RecvTop5 == RecvEnd5) return 0;//read nothing

	*Str = USART5RecvBuf[RecvTop5];
	RecvTop5++;

	if (RecvTop5 >= USART5MAXBUFFSIZE) RecvTop5 = 0;

	return 1;//read one

}




/*****************************************************************************
 函 数 名  : dev_Usart2Read
 功能描述  : 读串口数据
 输入参数  : unsigned char *buf  
             int len             
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
int dev_Usart2Read(unsigned char *buf, int len)
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t bsp_Usart5_Read(uint8_t *Buff, uint16_t len)
{

	uint16_t RecvLen = 0;
	uint8_t tmp[1] = {0};

	if (len == 0 || Buff == NULL) return 0;

	while (len--)
	{
		if (bsp_Usart5_RecvOne (tmp) == 1)
		{
			Buff[RecvLen++] = tmp[0];
		}

		if (RecvLen >= len) return RecvLen;
	}

	return RecvLen;
    
}



