/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Reader_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月27日
  最近修改   :
  功能描述   : 处理维根读卡器任务
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月27日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "Reader_Task.h"
#include "CmdHandle.h"
#include "bsp_dipSwitch.h"
#include "tool.h"


#define LOG_TAG    "reader"
#include "elog.h"


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define READER_TASK_PRIO	    ( tskIDLE_PRIORITY + 1)
#define READER_STK_SIZE 		(configMINIMAL_STACK_SIZE*8)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *ReaderTaskName = "vReaderTask";  

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskReader = NULL;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskReader(void *pvParameters);

void CreateReaderTask(void)
{
    //跟android通讯串口数据解析
    xTaskCreate((TaskFunction_t )vTaskReader,     
                (const char*    )ReaderTaskName,   
                (uint16_t       )READER_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )READER_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskReader);
}



static void vTaskReader(void *pvParameters)
{ 
    uint32_t CardID = 0;
    uint8_t dat[4] = {0};
    uint8_t asc[9] = {0};
    uint8_t tmp[26] ={ 0x43,0x41,0x52,0x44,0x20,0x32,0x33,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x38,0x39,0x30,0x30,0x30,0x30,0x30,0x30,0x0d,0x0a };

    memset(&gReaderMsg,0x00,sizeof(READER_BUFF_STRU));
    READER_BUFF_STRU *ptReader = &gReaderMsg;

    while(1)
    {
    	/* 清零 */
        ptReader->dataLen = 0;
        ptReader->authMode = AUTH_MODE_CARD;
        memset(ptReader->data,0x00,sizeof(ptReader->data));   
        
        CardID = bsp_WeiGenScanf();

        if(CardID != 0)
        {
            memset(dat,0x00,sizeof(dat));            
            
//			dat[0] = CardID>>24;
			dat[0] = CardID>>16;
			dat[1] = CardID>>8;
			dat[2] = CardID&0XFF;    

            dbh("card id",(char *)dat,3);
            
            bcd2asc(asc, dat, 6, 0);
            log_d("asc = %s\r\n",asc);
            
            memcpy(tmp+17,asc,6);
            log_d("tmp = %s\r\n",tmp);
            
            ptReader->dataLen = 25;
            memcpy(ptReader->data,tmp,ptReader->dataLen);

			/* 使用消息队列实现指针变量的传递 */
			if(xQueueSend(xDataProcessQueue,              /* 消息队列句柄 */
						 (void *) &ptReader,   /* 发送结构体指针变量ptReader的地址 */
						 (TickType_t)50) != pdPASS )
			{
                DBG("the queue is full!\r\n");                             
            } 
            else
            {
                dbh("WGREADER",(char *)dat,4);
            }          

          
        }
        
    	/* 发送事件标志，表示任务正常运行 */        
    	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_4);       
        
        vTaskDelay(100);        
    }

}   



