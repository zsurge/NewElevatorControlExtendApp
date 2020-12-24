/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Comm_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月28日
  最近修改   :
  功能描述   : 跟电梯通讯的任务处理文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月28日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#define LOG_TAG    "CommTask"
#include "elog.h"

#include "Comm_Task.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "bsp_dipSwitch.h"
#include "FloorDataProc.h"
#include "bsp_usart6.h"
#include "malloc.h"
#include "tool.h"


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

#define UNFINISHED		        	    0x00
#define FINISHED          	 			0x55



#define STEP1   0
#define STEP2   10
#define STEP3   20
#define STEP4   30

typedef struct FROMHOST
{
    uint8_t rxStatus;                   //接收状态
    uint8_t rxCRC;                      //校验值
    uint8_t rxBuff[16];                 //接收字节数
    uint16_t rxCnt;                     //接收字节数    
}FROMHOST_STRU;

 
#define COMM_TASK_PRIO		(tskIDLE_PRIORITY + 6) 
#define COMM_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)

uint16_t packetBuf(ELEVATOR_TRANBUFF_STRU *src,uint8_t *desc);
uint16_t packetDefault(uint8_t devSn ,uint8_t *desc);

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *CommTaskName = "vCommTask"; 

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskComm = NULL;  

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskComm(void *pvParameters);
static uint8_t deal_Serial_Parse(void);

static FROMHOST_STRU rxFromHost;


void CreateCommTask(void)
{
    xTaskCreate((TaskFunction_t )vTaskComm,         
                (const char*    )CommTaskName,       
                (uint16_t       )COMM_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )COMM_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskComm);
}


static void vTaskComm(void *pvParameters)
{
    TickType_t xLastWakeTime;
    ELEVATOR_TRANBUFF_STRU *recvBuf = &gRecvElevtorData;
    uint8_t devSn = 0;

    uint32_t i = 0;
    uint8_t buf[32] = {0};
    uint16_t bufLen = 0;    
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(30); /* 设置最大等待时间为200ms */  

    xLastWakeTime = xTaskGetTickCount();
    
    while (1)
    {
        memset(&gRecvElevtorData,0x00,sizeof(gRecvElevtorData));       

  
        xReturn = xQueueReceive( xTransDataQueue,    /* 消息队列的句柄 */
                                 (void *)recvBuf,  /*这里获取的是结构体的地址 */
                                 xMaxBlockTime); /* 设置阻塞时间 */
        if(pdTRUE == xReturn)
        {
            //消息接收成功，发送接收到的消息   
            memset(buf,0x00,sizeof(buf));
            bufLen = packetBuf(recvBuf ,buf);
            RS485_SendBuf(COM6,buf,bufLen); 
            dbh("send com6 buff", buf, bufLen);  

        }
        else
        {
            //无消息则发送握手消息            
            memset(buf,0x00,sizeof(buf));
            if(devSn > 7)
            {
              devSn = 0;
            }
            devSn++;
            bufLen = packetDefault(devSn,buf);
            RS485_SendBuf(COM6,buf,bufLen); 
        }  
        
        vTaskDelay(50);
        
        if(deal_Serial_Parse() == FINISHED)
        { 
            log_d("recv extend return data\r\n");
        }  

		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);  
    }

}

static uint8_t deal_Serial_Parse(void)
{
    uint8_t ch = 0;
    uint8_t devID = 0;

    
    while(RS485_Recv(COM6,&ch,1))
    {
       switch (rxFromHost.rxStatus)
        {                
            case STEP1:
                if(0xA6 == ch) /*接收包头*/
                {
                    rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;
                    rxFromHost.rxCRC ^= ch;
                    if(rxFromHost.rxCnt==3)
                        rxFromHost.rxStatus = STEP2;
                }

                break;
           case STEP2:
                devID = (bsp_dipswitch_read() & 0x03)+1;

                if(devID == ch) //判定第二个字节是否是当前设备ID，读取拨码开关的值
                {
                    rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;
                    rxFromHost.rxCRC ^= ch;               
                    rxFromHost.rxStatus = STEP3;                
                }
                else
                {                
                   memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));                   
                }
                break;           
            default:      /* 接收整个数据包 */
            
                rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;  
                
                if(rxFromHost.rxCnt == 8)
                {
                
                    if(rxFromHost.rxCRC == rxFromHost.rxBuff[rxFromHost.rxCnt-1])
                    { 
                        memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));
                        return FINISHED;                         
                    }  
                    memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));
                }
                else
                {
                     rxFromHost.rxCRC ^= ch;
                }
             
                break;
         }
         
    }   

    return UNFINISHED;
}


uint16_t packetBuf(ELEVATOR_TRANBUFF_STRU *src,uint8_t *desc)
{
    uint8_t buf[32] = {0};
    uint16_t len = 0;    

    buf[len++] = 0xA6;
    buf[len++] = 0xA6;
    buf[len++] = 0xA6;
    buf[len++] = src->devSn;
    buf[len++] = 0x03;    
    buf[len++] = 0xA1;
    buf[len++] = src->value/256;//高8位
    buf[len++] = src->value%256;//低8位
    buf[len++] = xorCRC(buf,8);

    memcpy(desc,buf,len);

    return len;    
}

uint16_t packetDefault(uint8_t devSn ,uint8_t *desc)
{
    uint8_t buf[32] = {0};
    uint16_t len = 0;

    buf[len++] = 0xA6;
    buf[len++] = 0xA6;
    buf[len++] = 0xA6;
    buf[len++] = devSn;
    buf[len++] = 0x03;    
    buf[len++] = 0x06;
    buf[len++] = 0x00;//高8位
    buf[len++] = 0x00;//低8位
    buf[len++] = xorCRC(buf,8);

    memcpy(desc,buf,len);

    return len;    
}





