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

    ELEVATOR_BUFF_STRU *recvBuf = &gRecvElevtorData;
    uint32_t i = 0;
    uint16_t bufLen = 0;    
    uint8_t sendBuf[104] = {0x7E,0x55,0x53,0x5A,0x8D,0x4B,0x57,0xA1,0x00,0x00,0x00,0x00,0x02,0x01,0x04,0xBC,0x00,0x00,0x00,0x00,0x00,0x13,0x0C,0x1B,0x42,0x01,0x01,0x00,0x00,0x00,0x01,0x00,0x00}; 
//    uint8_t sendBuf[104] =  { 0x7E,0x55,0x53,0x64,0x1E,0xFD,0x19,0xA1,0x00,0x00,0x00,0x00,0x02,0x00,0x04,0xC5,0x00,0x00,0x00,0x00,0x00,0x15,0x07,0x0C,0x42,0x01,0x01,0x00,0x00,0x00,0x01,0x00,0x00};
    uint8_t tmpBuf[104] = {0};
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(50); /* 设置最大等待时间为200ms */  
    uint8_t all[104] = { 0x7E,0x51,0x57,0x00,0x00,0x00,0x00,0x0D,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A };


    xLastWakeTime = xTaskGetTickCount();
    
    while (1)
    {

        memset(&gRecvElevtorData,0x00,sizeof(gRecvElevtorData)); 
        memset(tmpBuf,0x00,sizeof(tmpBuf));
        
        xReturn = xQueueReceive( xTransDataQueue,    /* 消息队列的句柄 */
                                 (void *)&recvBuf,  /*这里获取的是结构体的地址 */
                                 xMaxBlockTime); /* 设置阻塞时间 */
        if(pdTRUE == xReturn)
        {
            //消息接收成功，发送接收到的消息  
            log_d("recvBuf->type = %d\r\n",recvBuf->type);
            if(recvBuf->type == 0) //多层，开放
            {
                sendBuf[13] = recvBuf->type;
                memcpy(sendBuf+33,recvBuf->data,71);

                memcpy(tmpBuf,sendBuf,54);
                memcpy(tmpBuf+55,sendBuf+54,49);
                tmpBuf[103] = 0x0d;
                dbh("recv queue buff", tmpBuf,104); 
                RS485_SendBuf(COM6,tmpBuf,104);    
            }
            else if(recvBuf->type == 1) //单层直达
            {
                sendBuf[13] = recvBuf->type;
                memcpy(sendBuf+33,recvBuf->data,71);
                dbh("recv queue buff", sendBuf,104); 
                RS485_SendBuf(COM6,sendBuf,104);               
            }
            else //所有
            {
                RS485_SendBuf(COM6,all,104);   
            }

        }

        
        vTaskDelay(300);
        
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
       printf("%02x ",ch);
       switch (rxFromHost.rxStatus)
        {                
            case STEP1:
                if(0xFE == ch) /*接收包头*/
                {
                    rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;                            
                }
                break;       
            default:      /* 接收整个数据包 */
            
                rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;  
                rxFromHost.rxCRC ^= ch;
                
                if(rxFromHost.rxCnt == 12)
                {
                
                    if(rxFromHost.rxCRC == rxFromHost.rxBuff[rxFromHost.rxCnt-1])
                    { 
                        memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));
                        printf("\r\n");
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








