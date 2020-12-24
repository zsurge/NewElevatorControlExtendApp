/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp_USART6.c
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
#include "stm32f4xx_conf.h"
#include "bsp_usart1.h"


volatile uint8_t USART6RecvBuf[USART1MAXBUFFSIZE] = {0};
volatile uint8_t USART6SendBuf[USART1MAXBUFFSIZE] = {0};

volatile uint16_t RecvTop6 = 0;
volatile uint16_t RecvEnd6 = 0;


u16 UART6_ReceiveSize = 0;



static void Usart6_DMA2_TX_Config(void);
static void Usart6_DMA2_RX_Config(void);



/*****************************************************************************
 函 数 名  : bsp_Usart1Init
 功能描述  : 串口初始化
 输入参数  : uint32_t BaudRate  波特率
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
void bsp_Usart1Init (uint32_t BaudRate);
    修改内容   : 新生成函数

*****************************************************************************/
void bsp_Usart6_Init (uint32_t BaudRate)
{
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;


    USART_DeInit(USART6);
    
    //使能GPIOA时钟
	RCC_AHB1PeriphClockCmd(USART6_GPIO_CLK,ENABLE); 

	//使能USART1时钟
	RCC_APB2PeriphClockCmd(USART6_CLK,ENABLE);
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(USART6_GPIO_PORT,USART6_TX_SOURCE,USART6_TX_AF); //GPIOA9复用为USART1
	GPIO_PinAFConfig(USART6_GPIO_PORT,USART6_RX_SOURCE,USART6_RX_AF); //GPIOA10复用为USART1
	
	//USART6端口配置
	GPIO_InitStructure.GPIO_Pin = USART6_TX_PIN | USART6_RX_PIN; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(USART6_GPIO_PORT,&GPIO_InitStructure); //初始化PA9，PA10

    //DE6
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	/* 打开GPIO时钟 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* 无上拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);   		

   //USART6 初始化设置
	USART_InitStructure.USART_BaudRate = BaudRate;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART6, &USART_InitStructure); //初始化串口1    


	//Usart6 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

    //modify 2020.05.29	
//	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);//开启相关中断
//	USART_Cmd(USART6, ENABLE);  //使能串口1  
//	USART_ClearFlag(USART6, USART_FLAG_TC);     /* 清发送完成标志，Transmission Complete flag */

    USART_ITConfig(USART6, USART_IT_RXNE, DISABLE);
    USART_ITConfig(USART6, USART_IT_TC,   DISABLE);
    USART_ITConfig(USART6, USART_IT_TXE,  DISABLE);  

    
    USART_ITConfig(USART6, USART_IT_IDLE, ENABLE);//开启空闲中断 
    
    USART_DMACmd(USART6, USART_DMAReq_Tx, ENABLE);  
    USART_DMACmd(USART6, USART_DMAReq_Rx, ENABLE);
    
    Usart6_DMA2_TX_Config();   
    Usart6_DMA2_RX_Config();


    USART_Cmd(USART6, ENABLE);//使能串口6
    RS485_U6_RX_EN();
}


static void Usart6_DMA2_TX_Config(void)  
{  
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2,ENABLE);//DMA2时钟使能 
    
    //reset 
    DMA_DeInit(DMA2_Stream6); //如果使用串口6， DMA2 通道5，数据流6

    while(DMA_GetCmdStatus(DMA2_Stream6) != DISABLE);//等待DMA可配置 
    
    
    //DMA configuration  
    DMA_InitStructure.DMA_Channel = DMA_Channel_5; //通道选择
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART6->DR; //DMA外设地址
    DMA_InitStructure.DMA_BufferSize = USART1MAXBUFFSIZE;//1;   //数据传输量，初始化时先定为1
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART6SendBuf; //DMA 存储器0地址     
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral; //存储器到外设模式    
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式   
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //存储器增量模式 
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //外设数据长度:8位 
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //存储器数据长度:8位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal ;  // 使用普通模式    
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //中等优先级    
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;  //存储器突发单次传输
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single; //外设突发单次传输
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold =DMA_FIFOThreshold_Full;
    /* 3. 配置DMA */
    DMA_Init(DMA2_Stream6, &DMA_InitStructure);

    /* 6. 配置DMA中断优先级 */
    NVIC_InitStructure.NVIC_IRQChannel                   = DMA2_Stream6_IRQn;           
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;          
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0; 
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);  

    DMA_Cmd(DMA2_Stream6,ENABLE);
     
//    DMA_ITConfig(DMA2_Stream6,DMA_IT_TC,ENABLE);
}

static void Usart6_DMA2_RX_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure; 
    NVIC_InitTypeDef NVIC_InitStructure;

    /* 1.使能DMA2时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    /* 2.配置使用DMA接收数据 */
    DMA_DeInit(DMA2_Stream1); 

    while(DMA_GetCmdStatus(DMA2_Stream1) != DISABLE);//等待DMA可配置  

    DMA_InitStructure.DMA_Channel             = DMA_Channel_5;               /* 配置DMA通道 */
    DMA_InitStructure.DMA_PeripheralBaseAddr  = (uint32_t)(&(USART6->DR));   /* 源 */
    DMA_InitStructure.DMA_Memory0BaseAddr     = (uint32_t)USART6RecvBuf;             /* 目的 */
    DMA_InitStructure.DMA_DIR                 = DMA_DIR_PeripheralToMemory;    /* 方向 */
    DMA_InitStructure.DMA_BufferSize          = USART1MAXBUFFSIZE;                    /* 长度 */                  
    DMA_InitStructure.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;    /* 外设地址是否自增 */
    DMA_InitStructure.DMA_MemoryInc           = DMA_MemoryInc_Enable;         /* 内存地址是否自增 */
    DMA_InitStructure.DMA_PeripheralDataSize  = DMA_MemoryDataSize_Byte;      /* 目的数据带宽 */
    DMA_InitStructure.DMA_MemoryDataSize      = DMA_MemoryDataSize_Byte;      /* 源数据宽度 */
    DMA_InitStructure.DMA_Mode                = DMA_Mode_Normal;              /* 单次传输模式/循环传输模式 */
    DMA_InitStructure.DMA_Priority            = DMA_Priority_VeryHigh;        /* DMA优先级 */
    DMA_InitStructure.DMA_FIFOMode            = DMA_FIFOMode_Disable;          /* FIFO模式/直接模式 */
    DMA_InitStructure.DMA_FIFOThreshold       = DMA_FIFOThreshold_HalfFull; /* FIFO大小 */
    DMA_InitStructure.DMA_MemoryBurst         = DMA_MemoryBurst_Single;       /* 单次传输 */
    DMA_InitStructure.DMA_PeripheralBurst     = DMA_PeripheralBurst_Single;

    /* 3. 配置DMA */
    DMA_Init(DMA2_Stream1, &DMA_InitStructure);

    //DMA NVIC  
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);  

    // 清除DMA所有标志
    DMA_ClearFlag(DMA2_Stream1,DMA_FLAG_TCIF1);

    DMA_ITConfig(DMA2_Stream1,DMA_IT_TC, ENABLE);
    
    DMA_Cmd(DMA2_Stream1, ENABLE);  //开启DMA传输 

}



//开启一次DMA传输
//DMA_Streamx:DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7 
//ndtr:数据传输量  
void DmaSendDataProc(DMA_Stream_TypeDef *DMA_Streamx,u16 ndtr)
{
    RS485_U6_TX_EN();
    
	DMA_Cmd(DMA_Streamx, DISABLE);                      //关闭DMA传输 
	
	while (DMA_GetCmdStatus(DMA_Streamx) != DISABLE){}	//确保DMA可以被设置  
		
	DMA_SetCurrDataCounter(DMA_Streamx,ndtr);          //数据传输量  
 
	DMA_Cmd(DMA_Streamx, ENABLE);                      //开启DMA传输 
}

///////////////////////////////////////////////////////////////////////////
//串口1
//发送单字节
 void SendByteInfoProc(u8 nSendInfo)
{
	u8 *pBuf = NULL;
	//指向发送缓冲区
	pBuf = USART6SendBuf;
	*pBuf++ = nSendInfo;
 
	DmaSendDataProc(DMA2_Stream6,1); //开始一次DMA传输！	  
}
//发送多字节
void SendBytesInfoProc(u8* pSendInfo, u16 nSendCount)
{
	u16 i = 0;
	u8 *pBuf = NULL;
	//指向发送缓冲区
	pBuf = USART6SendBuf;
 
	for (i=0; i<nSendCount; i++)
	{
		*pBuf++ = pSendInfo[i];
	}
	//DMA发送方式
	DmaSendDataProc(DMA2_Stream6,nSendCount); //开始一次DMA传输！	  
}


//发送完成中断
void DMA2_Stream6_IRQHandler(void)
{
	//清除标志
	if(DMA_GetFlagStatus(DMA2_Stream6,DMA_FLAG_TCIF6)!=RESET)//等待DMA2_Steam7传输完成
	{ 
		DMA_ClearFlag(DMA2_Stream6,DMA_FLAG_TCIF6);//清除DMA2_Steam7传输完成标志
		RS485_U6_RX_EN();
	}
}


//接收完成中断
void DMA2_Stream1_IRQHandler(void)
{
	//清除标志
	if(DMA_GetFlagStatus(DMA2_Stream1,DMA_FLAG_TCIF1)!=RESET)//等待DMA2_Steam7传输完成
	{ 
		DMA_Cmd(DMA2_Stream1, DISABLE); //关闭DMA,防止处理其间有数据

 
		UART6_ReceiveSize =USART1MAXBUFFSIZE - DMA_GetCurrDataCounter(DMA2_Stream1);
		if(UART6_ReceiveSize !=0)
		{
//			OSSemPost((DMAReceiveSize_Sem);
		}
		
		DMA_ClearFlag(DMA2_Stream1,DMA_FLAG_TCIF1 | DMA_FLAG_FEIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1);//清除DMA2_Steam7传输完成标志
		DMA_SetCurrDataCounter(DMA2_Stream1, USART1MAXBUFFSIZE);
		DMA_Cmd(DMA2_Stream1, ENABLE);     //打开DMA,
	}
}


void USART6_IRQHandler (void)
{

    u16 data;
	if(USART_GetITStatus(USART6,USART_IT_IDLE) != RESET)
	{
		DMA_Cmd(DMA2_Stream1, DISABLE); //关闭DMA,防止处理其间有数据
 
		data = USART6->SR;
		data = USART6->DR;

		printf("UART6_ReceiveSize = %d\r\n",UART6_ReceiveSize);
		UART6_ReceiveSize =USART1MAXBUFFSIZE - DMA_GetCurrDataCounter(DMA2_Stream1);
		if(UART6_ReceiveSize !=0)
		{
//			OSSemPost(DMAReceiveSize_Sem);
		}
		DMA_ClearFlag(DMA2_Stream1,DMA_FLAG_TCIF1 | DMA_FLAG_FEIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1);//清除DMA2_Steam7传输完成标志
		DMA_SetCurrDataCounter(DMA2_Stream1, USART1MAXBUFFSIZE);
		DMA_Cmd(DMA2_Stream1, ENABLE);     //打开DMA,
 
	}




//	uint8_t temp = 0;

//    if (USART_GetFlagStatus(USART6, USART_FLAG_PE) != RESET)
//    {
//        USART_ClearFlag(USART6, USART_FLAG_PE);    
//        USART_ReceiveData(USART6);
//    }

//    if (USART_GetFlagStatus(USART6, USART_FLAG_ORE) != RESET)
//    {
//        USART_ClearFlag(USART6, USART_FLAG_ORE);    
//        USART_ReceiveData(USART6);
//    }
//     
//     if (USART_GetFlagStatus(USART6, USART_FLAG_FE) != RESET)
//    {
//        USART_ClearFlag(USART6, USART_FLAG_FE);    
//        USART_ReceiveData(USART6);
//    }	


//    //Receive data register not empty flag
//	if (USART_GetITStatus (USART6, USART_IT_RXNE) != RESET)
//	{

//        USART_ClearFlag(USART6, USART_FLAG_RXNE);
//		/* Clear the UART1 Recvive interrupt */
//		USART_ClearITPendingBit (USART6, USART_IT_RXNE);
//		
//		temp = USART_ReceiveData (USART6);		//get received data

//		if ( (RecvEnd6 == (RecvTop6 - 1) ) || ( (RecvTop6 == 0) && (RecvEnd6 == (USART1MAXBUFFSIZE - 1) ) ) )
//		{
//			//缓冲溢出
//			RecvTop6 = 0;
//			RecvEnd6 = 0;
//		}
//		else
//		{
//			USART6RecvBuf[RecvEnd6] = temp;
//			RecvEnd6++;

//			if (RecvEnd6 >= USART1MAXBUFFSIZE) RecvEnd6 = 0;
//		}		
//	}



//	if (USART_GetITStatus (USART1, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE) != RESET)
//	{
//		USART_GetITStatus (USART1, USART_IT_NE | USART_IT_PE | USART_IT_FE | USART_IT_ORE);
//		USART_ReceiveData (USART1);
//	}
}


void bsp_Usart6_SendOne (const uint8_t dat)
{	
    while (USART_GetFlagStatus (USART6, USART_FLAG_TXE) == RESET) {}
    USART_SendData (USART6, dat);
    while (USART_GetFlagStatus (USART6, USART_FLAG_TC) == RESET) {}
}



/*****************************************************************************
 函 数 名  : bsp_Usart1SendString
 功能描述  : 发送字符串
 输入参数  : const uint8_t *Buff  发送字符串
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
void bsp_Usart1SendString (const uint8_t *Buff);
    修改内容   : 新生成函数

*****************************************************************************/
void bsp_Usart6_SendString (const uint8_t *Buff)
{
	while (*Buff != 0)
	{
		bsp_Usart6_SendOne (*Buff);
		Buff++;
	}    
}


/*****************************************************************************
 函 数 名  : bsp_Usart1SendData
 功能描述  : 发送指定长度字符
 输入参数  : const uint8_t *Buff  发送数据的buff
             uint16_t SendSize  发送数据的长度  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
void bsp_Usart1SendData (const uint8_t *Buff, uint16_t SendSize);
    修改内容   : 新生成函数

*****************************************************************************/
void bsp_Usart6_SendData (const uint8_t *Buff, uint16_t SendSize)
{

    RS485_U6_TX_EN();
	while (SendSize != 0)
	{
		bsp_Usart6_SendOne (*Buff);
		Buff++;
		SendSize--;
	}

	RS485_U6_RX_EN();

}



/*****************************************************************************
 函 数 名  : bsp_Usart1RecvReset
 功能描述  : 清空串口接收缓冲
 输入参数  : void  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
void bsp_Usart1RecvReset (void);
    修改内容   : 新生成函数

*****************************************************************************/
void bsp_Usart6_RecvReset (void)
{
	RecvTop6 = 0;
	RecvEnd6 = 0;

	memset((void *)USART6RecvBuf, 0, USART1MAXBUFFSIZE); 
}



/*****************************************************************************
 函 数 名  : bsp_Usart1RecvOne
 功能描述  : 读取1个字节
 输入参数  : uint8_t *Str  读出的字节存储在Str[0]
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
uint8_t bsp_Usart1RecvOne (uint8_t *Str);
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t bsp_Usart6_RecvOne (uint8_t *Str)
{
	if (RecvTop6 == RecvEnd6) return 0;//read nothing

	*Str = USART6RecvBuf[RecvTop6];
	RecvTop6++;

	if (RecvTop6 >= USART1MAXBUFFSIZE) RecvTop6 = 0;

	return 1;//read one

}



/*****************************************************************************
 函 数 名  : dev_Usart1Read
 功能描述  : 读串口数据
 输入参数  : unsigned char *buf  
             int len             
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2019年5月22日
    作    者   : 张舵
int dev_Usart1Read(unsigned char *buf, int len)
    修改内容   : 新生成函数

*****************************************************************************/
uint8_t bsp_Usart6_Read(uint8_t *Buff, uint16_t len)
{

	uint16_t RecvLen = 0;
	uint8_t tmp[1] = {0};

	if (len == 0 || Buff == NULL) return 0;

	while (len--)
	{
		if (bsp_Usart6_RecvOne (tmp) == 1)
		{
			Buff[RecvLen++] = tmp[0];
		}

		if (RecvLen >= len) return RecvLen;
	}

	return RecvLen;
    
}






