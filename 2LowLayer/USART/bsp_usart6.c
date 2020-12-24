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
#include "bsp_usart6.h"
/* FreeRTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"

uint8_t gUsart6RXBuff[COM6_RXBUFFER_SIZE];//接收缓存器

//串口1接收DMA缓存
RingBuff_t ringbuff_handle;


uint8_t Uart6_Tx[UART6_TX_LEN] = {0};               //串口1发送DMA缓存     
uint8_t Uart6_Rx[UART6_RX_LEN] = {0};               //串口1接收DMA缓存 

static void BSP_DMAUsar6Rx_Init(void);
static void BSP_DMAUsar6Tx_Init(void);

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

	RCC_AHB1PeriphClockCmd(USART1_GPIO_CLK,ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(USART1_CLK,ENABLE);//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(USART1_GPIO_PORT,USART1_TX_SOURCE,USART1_TX_AF); //GPIOA9复用为USART1
	GPIO_PinAFConfig(USART1_GPIO_PORT,USART1_RX_SOURCE,USART1_RX_AF); //GPIOA10复用为USART1
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = USART1_TX_PIN | USART1_RX_PIN; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(USART1_GPIO_PORT,&GPIO_InitStructure); //初始化PA9，PA10

    //DE1
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	/* 打开GPIO时钟 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		/* 设为输出口 */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		/* 设为推挽 */
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	/* 无上拉电阻 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	/* IO口最大速度 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);   		

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = BaudRate;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART6, &USART_InitStructure); //初始化串口1    


    //开启USART6
    USART_Cmd(USART6, ENABLE);
    

	USART_ClearFlag(USART6, USART_FLAG_TC); //清除发送完成标志	
	while(USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET);	//等待空闲帧发送完成后再清零发送完成标志
	USART_ClearFlag(USART6, USART_FLAG_TC);	//清除发送完成标志
    
    //开启USART6总线空闲中断
	USART_ITConfig(USART6,USART_IT_TC,DISABLE);  
	USART_ITConfig(USART6,USART_IT_RXNE,DISABLE); 
	USART_ITConfig(USART6,USART_IT_TXE,DISABLE); 
	USART_ITConfig(USART6,USART_IT_IDLE,ENABLE); 



	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =4;		//子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器


	
    //串口接收DMA配置
    BSP_DMAUsar6Rx_Init();
    //串口发送DMA配置
    BSP_DMAUsar6Tx_Init();

    USART_DMACmd(USART6,USART_DMAReq_Tx,ENABLE);  //采用DMA方式发送   
    USART_DMACmd(USART6,USART_DMAReq_Rx,ENABLE); //采用DMA方式接收

    
    Create_RingBuff(&ringbuff_handle,  gUsart6RXBuff,sizeof(gUsart6RXBuff));
}

void USART6_IRQHandler(void)
{    
    uint32_t rxBuffLen = 0;
    uint32_t ulReturn;
    /* 进入临界段，临界段可以嵌套 */
    ulReturn = taskENTER_CRITICAL_FROM_ISR();
    
    if(USART_GetITStatus(USART6, USART_IT_IDLE) != RESET)  
    {
      //1.清除USART6接收完成中断
        USART6->SR;  
        USART6->DR;   //清USART_IT_IDLE标志  
      //2.存储收到的数据内容、长度、标志位
        DMA_Cmd(DMA2_Stream1,DISABLE); //使能数据流1 通道5
        
        DMA_ClearFlag(DMA2_Stream1, DMA_FLAG_TCIF1 | DMA_FLAG_FEIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1);//清零标志位
    
        rxBuffLen = UART6_RX_LEN - DMA_GetCurrDataCounter(DMA2_Stream1);        //计算接收数据包长度      
        
        Write_RingBuff(&ringbuff_handle,Uart6_Rx,rxBuffLen);  
        
        DMA_SetCurrDataCounter(DMA2_Stream1,UART6_RX_LEN);                      //设置传输数据长度
        DMA_Cmd(DMA2_Stream1,ENABLE);                                           //打开DMA 
    } 

//    if(USART_GetITStatus(USART6, USART_IT_TC) != RESET)
//    {
//        USART_ClearFlag(USART6,USART_IT_TC);
//        
//        //关闭发送完成中断
//        USART_ITConfig(USART6,USART_IT_TC,DISABLE);
//        
//        RS485_U6_RX_EN();
//    }

    if(USART_GetITStatus(USART6, USART_IT_TC) != RESET)
    {
        USART_ClearITPendingBit(USART6, USART_IT_TC);   //清除USART1发送完成中断标志
        DMA_ClearFlag(DMA2_Stream6, DMA_FLAG_TCIF6 | DMA_FLAG_FEIF6 | 
                      DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6);//清零标志位 
                      
        RS485_U6_RX_EN();       //切换为RS485接收模式 
    }    

  /* 退出临界段 */
  taskEXIT_CRITICAL_FROM_ISR( ulReturn );	    

}

//USART1接收DMA配置
static void BSP_DMAUsar6Rx_Init(void)
{
    DMA_InitTypeDef   DMA_InitStructure;
    u16 mid_u16RetryCnt = 0;
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);                        //启动DMA时钟  
    
    DMA_DeInit(DMA2_Stream1);
    while ((DMA_GetCmdStatus(DMA2_Stream1) != DISABLE) && (mid_u16RetryCnt++ < 500));
    
    DMA_InitStructure.DMA_Channel = DMA_Channel_5;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART6->DR);         //外设地址      
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Uart6_Rx;                 //内存地址      
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;                     //dma传输方向单向      
    DMA_InitStructure.DMA_BufferSize = UART6_RX_LEN;                            //设置DMA在传输时缓冲区的长度     
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            //设置DMA的外设递增模式，一个外设        
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     //设置DMA的内存递增模式      
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;     //外设数据字长         
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;             //内存数据字长      
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                               //设置DMA的传输模式      
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                     //设置DMA的优先级别     
    
 	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
 	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
 	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
 	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    
    DMA_Init(DMA2_Stream1,&DMA_InitStructure);	  
    DMA_Cmd(DMA2_Stream1,ENABLE);                                               //使能数据流1 通道5
}
 
//USART6发送DMA配置
static void BSP_DMAUsar6Tx_Init(void)
{
    DMA_InitTypeDef   DMA_InitStructure;  
    NVIC_InitTypeDef NVIC_InitStructure;
    u16 mid_u16RetryCnt = 0;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);                        //启动DMA时钟  
    
    DMA_DeInit(DMA2_Stream6);
    while ((DMA_GetCmdStatus(DMA2_Stream6) != DISABLE) && (mid_u16RetryCnt++ < 500));
    
    DMA_InitStructure.DMA_Channel = DMA_Channel_5;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART6->DR);         //DMA外设基地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Uart6_Tx;                 //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;                     //数据传输方向，从内存读取发送到外设
	DMA_InitStructure.DMA_BufferSize = UART6_TX_LEN;                            //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;            //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                     //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;     //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;             //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                               //工作在正常模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                       //DMA通道 x拥有中优先级 
    
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;    
	


    //DMA发送中断设置
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    DMA_ITConfig(DMA2_Stream6,DMA_IT_TC,ENABLE);    
    
	DMA_Init(DMA2_Stream6, &DMA_InitStructure);                                 //根据DMA_InitStruct中指定的参数初始化DMA的通道USART6_Tx_DMA_Channel所标识的寄存器   
    DMA_Cmd(DMA2_Stream6,DISABLE);
}
//USART2 DMA发送指定长度的数据
//str：要发送的数据首地址
//cndtr:数据传输量 
void BSP_DMAUsart6Send(uint8_t *str,uint8_t cndtr)
{
//    RS485_U6_TX_EN();
//    memcpy(Uart6_Tx, str, cndtr);
//    USART_DMACmd(USART6,USART_DMAReq_Tx,ENABLE);                                //使能串口6的DMA发送
//	DMA_Cmd(DMA2_Stream6, DISABLE );                                            //关闭USART6 TX DMA2 所指示的通道      
// 	DMA_SetCurrDataCounter(DMA2_Stream6,cndtr);                                 //DMA通道的DMA缓存的大小
// 	DMA_Cmd(DMA2_Stream6, ENABLE);

    u16 l_u16RetryCnt = 0;    
    RS485_U6_TX_EN();
    memcpy(Uart6_Tx, str, cndtr);
    DMA_Cmd(DMA2_Stream6, DISABLE);                      //关闭DMA传输           
    while ((DMA_GetCmdStatus(DMA2_Stream6) != DISABLE) && (l_u16RetryCnt++ < 500));	//等待DMA可配置	
    DMA_SetCurrDataCounter(DMA2_Stream6, cndtr);  //数据传输量 	 
    DMA_Cmd(DMA2_Stream6, ENABLE);                      	//开启DMA传输    	
}

//DMA2中断函数。
void DMA2_Stream6_IRQHandler(void)
{
//    if(DMA_GetITStatus(DMA2_Stream6,DMA_IT_TCIF6) != RESET) 
//	{
//		//清除标志位
//		DMA_ClearFlag(DMA2_Stream6,DMA_FLAG_TCIF6);
//		//关闭DMA
//		DMA_Cmd(DMA2_Stream6,DISABLE);
//		//打开发送完成中断,发送最后两个字节
//		USART_ITConfig(USART6,USART_IT_TC,ENABLE);
//		
//		RS485_U6_RX_EN();		//切换为RS485接收模式				
//	}

    if(DMA_GetFlagStatus(DMA2_Stream6, DMA_FLAG_TCIF6) != RESET)	//DMA发送完成  
    {   
		DMA_ClearFlag(DMA2_Stream6, DMA_FLAG_TCIF6 | DMA_FLAG_FEIF6 | 
					  DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6); 	//清除标志位			
		
		while(!USART_GetFlagStatus(USART6, USART_FLAG_TC));	//等待USART1发送完成标志TC置1
		USART_ClearFlag(USART6, USART_FLAG_TC); 	//清除发送完成标志

		DMA_Cmd(DMA2_Stream6,DISABLE);
        USART_ITConfig(USART6,USART_IT_TC,ENABLE);		 
		RS485_U6_RX_EN();		//切换为RS485接收模式		
    } 

}


uint16_t BSP_DMAUsart6Read(uint8_t *buffter, uint32_t len)
{
    return Read_RingBuff(&ringbuff_handle, buffter, len);
}




