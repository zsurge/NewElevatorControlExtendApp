/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : DataProcess_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年5月15日
  最近修改   :
  功能描述   : 对刷卡/QR/远程送过来的数据进行处理
  函数列表   :
  修改历史   :
  1.日    期   : 2020年5月15日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#define LOG_TAG    "DataProcess"
#include "elog.h"

#include "DataProcess_Task.h"
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

 
#define DATAPROC_TASK_PRIO		(tskIDLE_PRIORITY + 7) 
#define DATAPROC_STK_SIZE 		(configMINIMAL_STACK_SIZE*12)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *dataProcTaskName = "vDataProcTask"; 

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskDataProc = NULL;  


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
 
static void vTaskDataProcess(void *pvParameters);

void CreateDataProcessTask(void)
{
    xTaskCreate((TaskFunction_t )vTaskDataProcess,         
                (const char*    )dataProcTaskName,       
                (uint16_t       )DATAPROC_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )DATAPROC_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskDataProc);
}


static void vTaskDataProcess(void *pvParameters)
{
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(30); /* 设置最大等待时间为100ms */ 
    uint8_t isFind = 0; 
    uint8_t ret = 0; 
    uint8_t jsonBuf[512] = {0};
    uint8_t cardNo[8] = {0};
    int len = 0;
    uint8_t i = 0;
    
    ELEVATOR_BUFF_STRU devSendData;    
    USERDATA_STRU localUserData ;
    READER_BUFF_STRU *ptMsg  = &gReaderMsg;
    ELEVATOR_TRANBUFF_STRU *sendElevator = &gElevtorData;
    
    while (1)
    {
        memset(&devSendData,0x00,sizeof(ELEVATOR_BUFF_STRU));
        memset(&localUserData,0x00,sizeof(USERDATA_STRU));     
        memset(&gReaderMsg,0x00,sizeof(READER_BUFF_STRU));    
        memset(&gElevtorData,0x00,sizeof(ELEVATOR_TRANBUFF_STRU));    

        /* 清零 */
        ptMsg->authMode = 0; //默认为刷卡
        ptMsg->dataLen = 0;
        memset(ptMsg->data,0x00,sizeof(ptMsg->data));
    
        xReturn = xQueueReceive( xDataProcessQueue,    /* 消息队列的句柄 */
                                 (void *)&ptMsg,  /*这里获取的是结构体的地址 */
                                 xMaxBlockTime); /* 设置阻塞时间 */
        if(pdTRUE != xReturn)
        {
            //消息接收成功，发送接收到的消息
//            packetSendBuf(ptMsg); 
//            log_d("exec packetSendBuf end\r\n");

            continue;
        }
        
        
        switch (ptMsg->authMode)
        {
            case AUTH_MODE_CARD:            
                //读卡 CARD 230000000089E1E35D,23         
                memcpy(cardNo,ptMsg->data,CARD_NO_LEN);
                log_d("key = %s\r\n",cardNo);     
                
                isFind = readUserData(cardNo,CARD_MODE,&localUserData);   

                log_d("isFind = %d,rUserData.cardState = %d\r\n",isFind,localUserData.cardState);

                if(localUserData.cardState != CARD_VALID || isFind != 0)
                {
                    //未找到记录，无权限
                    log_e("not find record\r\n");
                   break;
                } 
                
                localUserData.platformType = 4;
                localUserData.authMode = ptMsg->authMode; 
                memcpy(localUserData.timeStamp,time_to_timestamp(),TIMESTAMP_LEN);
                log_d("localUserData->timeStamp = %s\r\n",localUserData.timeStamp);  
                
                //1.打包
                packetPayload(&localUserData,jsonBuf); 
                len = strlen((const char*)jsonBuf);
                
                //2.发给服务器
                len = mqttSendData(jsonBuf,len);
                log_d("send = %d\r\n",len);
                
                //3.计算电梯数据
                ret = packetToElevatorExtend(&localUserData,&devSendData);
                if(ret != NO_ERR)
                {
                    log_d("invalid floor\r\n");
                    break;  //无权限   
                }    

                //单层权限，并且在第2个设备或以后
                if(devSendData.data[0].devSn > 1)
                {
                    sendElevator->devSn = devSendData.data[0].devSn;
                    sendElevator->value = devSendData.data[0].value;
                    log_d("value = %x,devsn = %d\r\n",sendElevator->value,sendElevator->devSn);
                    //发送数据到队列 
                    sendQueueToDev(sendElevator);                       
                }
                else
                {
                    //4.发送电梯数据到队列
                    for(i=0;i<8;i++)
                    {   
                        if(devSendData.data[i].devSn == 1)
                        {
                            sendElevator->devSn = i+1;
                            sendElevator->value = devSendData.data[i].value;
                            log_d("value = %x,devsn = %d\r\n",sendElevator->value,sendElevator->devSn);
                           //发送数据到队列 
                           sendQueueToDev(sendElevator);                
                        }
                    }     
                }

                break;
            case AUTH_MODE_QR:
                isFind = parseQrCode(ptMsg->data,&localUserData);
                if(isFind != NO_ERR)
                {
                    log_d("not find record\r\n");
                    break ;  //无权限
                }
                
                localUserData.authMode = ptMsg->authMode; 
                
                //1.打包
                packetPayload(&localUserData,jsonBuf); 
                len = strlen((const char*)jsonBuf);

                //2.发给服务器
                len = mqttSendData(jsonBuf,len);
                log_d("send = %d\r\n",len);  

                //3.计算电梯数据
                ret = packetToElevatorExtend(&localUserData,&devSendData);
                if(ret != NO_ERR)
                {
                    log_d("invalid floor\r\n");
                    break;  //无权限   
                }

                //4.发送电梯数据到队列
                for(i=0;i<8;i++)
                {   
                    if(devSendData.data[i].devSn == 1)
                    {
                        sendElevator->devSn = i+1;
                        sendElevator->value = devSendData.data[i].value;
                       //发送数据到队列 
                       sendQueueToDev(sendElevator);  
                    }
                }                 
                break;
            case AUTH_MODE_REMOTE:
                //直接发送目标楼层
                log_d("send desc floor = %d,%d\r\n",ptMsg->data[0],ptMsg->dataLen);  
                
                localUserData.authMode = ptMsg->authMode; 
                ret = calcSingleFloor(ptMsg->data[0],&devSendData);
                if(ret != NO_ERR)
                {
                    log_d("invalid floor\r\n");
                    break;
                }
                sendElevator->devSn = devSendData.data[0].devSn;
                sendElevator->value = devSendData.data[0].value;
                
                log_d("send desc floor = %d,%d\r\n",sendElevator->value,sendElevator->devSn);  
                
                sendQueueToDev(sendElevator);
                break;
            case AUTH_MODE_UNBIND:
                //直接发送停用设备指令
                xQueueReset(xDataProcessQueue); 
                log_d("send AUTH_MODE_UNBIND floor\r\n");
                break;     
            case AUTH_MODE_BIND:
                //直接发送启动设置指令
                xQueueReset(xDataProcessQueue); 
                log_d("send AUTH_MODE_BIND floor\r\n");
                break;                
        }
        
        
        /* 发送事件标志，表示任务正常运行 */        
        xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_6);  
        vTaskDelay(100);

    }

}


