/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : BarCode_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月27日
  最近修改   :
  功能描述   : 条码处理任务
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月27日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "bsp_usart5.h"

#include "CmdHandle.h"
#include "tool.h"
#include "deviceInfo.h"
#include "stdlib.h"
#include "bsp_ds1302.h"

#include "malloc.h"
//#include "bsp_uart_fifo.h"
#define LOG_TAG    "BarCode"
#include "elog.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define BARCODE_TASK_PRIO		(tskIDLE_PRIORITY + 8)
#define BARCODE_STK_SIZE 		(configMINIMAL_STACK_SIZE*10)


#define UNFINISHED		        	    0x00
#define FINISHED          	 			0x55

#define STARTREAD		        	    0x00
#define ENDREAD         	 			0xAA


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *BarCodeTaskName = "vBarCodeTask";  

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskBarCode = NULL;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskBarCode(void *pvParameters);


static int compareDate(uint8_t *date1,uint8_t *date2);
static int compareTime(uint8_t *currentTime);
static void packetUserData(char *src,int icFlag,int qrFlag,READER_BUFF_STRU *desc);
static uint8_t parseReader(void);


typedef struct FROMREADER
{
    uint8_t rxBuff[512];               //接收字节数    
    uint8_t rxStatus;                   //接收状态
    uint16_t rxCnt;                     //接收字节数
}FROMREADER_STRU;

static FROMREADER_STRU gReaderData;


void CreateBarCodeTask(void)
{
    //读取条码数据并处理
    xTaskCreate((TaskFunction_t )vTaskBarCode,     
                (const char*    )BarCodeTaskName,   
                (uint16_t       )BARCODE_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )BARCODE_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskBarCode);
}


static void vTaskBarCode(void *pvParameters)
{ 
    uint8_t sendBuff[512] = {0};
    uint16_t len = 0; 
    uint8_t localTime[20] = {0};

    uint8_t relieveControl[38] = {0};

    uint8_t semavalue = 0;    
    BaseType_t xReturn = pdTRUE;/* 定义一个创建信息返回值，默认为pdPASS */

    int cmpTimeFlag = -1;

//    memset(&gReaderMsg,0x00,sizeof(READER_BUFF_STRU));
    READER_BUFF_STRU *ptQR = &gReaderMsg; 

    /* 清零 */
    ptQR->authMode = 0; 
    ptQR->dataLen = 0;
    ptQR->state = ENABLE;
    memset(ptQR->data,0x00,sizeof(ptQR->data));    
     
    while(1)
    { 
        memset(sendBuff,0x00,sizeof(sendBuff));
        
        if(parseReader() == FINISHED)
        {        

        len = gReaderData.rxCnt;
        memcpy(sendBuff,gReaderData.rxBuff,len);        
        memset(&gReaderData,0x00,sizeof(FROMREADER_STRU));
        
        if(len > 10  && sendBuff[len-1] == 0x0A && sendBuff[len-2] == 0x0D && gDevBaseParam.deviceState.iFlag == DEVICE_ENABLE)
        {       
            log_d("card or qr = %s\r\n",sendBuff);

            // 获取任务通知 , 没获取到则不等待
            xReturn = xSemaphoreTake(CountSem_Handle,0); /*  等待时间：0 */
            semavalue=uxSemaphoreGetCount(CountSem_Handle);	//获取计数型信号量值
            log_d("1 semavalue = %d,xReturn = %d\r\n",semavalue,xReturn);

            if(semavalue == 1) 
            {
//                log_d("2 semavalue = %d,xReturn = %d\r\n",semavalue,xReturn);
                
                //添加模版是否启用的判定
                if(gtemplateParam.templateStatus == 0 || gtemplateParam.offlineProcessing == 1)
                {
                    //2020-03-18 这里应该发送电梯不接受控制的指令，而不是不用continue,
                    //在授权方式那里，添加控制类型，并定义控制指令，发给电梯

                        //add code 发指不受控制指令                    
                        ptQR->authMode = AUTH_MODE_RELIEVECONTROL;
                        ptQR->dataLen = sizeof(relieveControl);
                        memcpy(ptQR->data,relieveControl,ptQR->dataLen);   
                        log_d("the template is disable\r\n");

                        
                        xReturn = xSemaphoreGive(CountSem_Handle);// 给出计数信号量
                        
                        semavalue=uxSemaphoreGetCount(CountSem_Handle); //获取计数型信号量值
                        

//                        comClearRxFifo(COM5);
//                        bsp_Usart5_RecvReset();
                        memset(sendBuff,0x00,sizeof(sendBuff));
                        len = 0;
                }
                else
                {
//                    log_d("the template is enable\r\n");
                    //判定高峰节假日模式是否开启
                    if(gtemplateParam.workMode.isPeakMode || gtemplateParam.workMode.isHolidayMode)
                    {
                        //读取当前时间
                        strcpy((char*)localTime,(const char*)bsp_ds1302_readtime());
                        
                        //判定节假日模板有效期
                        if(compareDate(localTime,gtemplateParam.peakInfo[0].endTime) < 0) //在有效期内
                        {   
                            log_d("in the peak mode valid date \r\n");
                            
                            //判定有效期内不受控时间段
                            cmpTimeFlag = compareTime(localTime);

                            log_d("cmpTimeFlag =%d\r\n",cmpTimeFlag);

                            if(cmpTimeFlag == 1)
                            {
                                log_d("peak mode - >in the out of control \r\n");
                                //在不受控时间段，发送脱离控制指令
                                ptQR->authMode = AUTH_MODE_RELIEVECONTROL;
                                ptQR->dataLen = sizeof(relieveControl);
                                memcpy(ptQR->data,relieveControl,sizeof(relieveControl)); 
                            } 
                            else
                            {
                                log_d("peak mode  >in the control time \r\n");
                                packetUserData((char *)sendBuff,gtemplateParam.peakCallingWay.isIcCard,gtemplateParam.peakCallingWay.isQrCode,ptQR);
                                //没有指定不受控日期 或 当前日期在有效期内
                                //添加节假日的呼梯方式的判定
                                    
                            }
                        }
                        else
                        {
                            //不在有效期内
                            //判定模板的呼梯方式
                            log_d("outside peak mode the valid date \r\n");

                            packetUserData((char *)sendBuff,gtemplateParam.templateCallingWay.isIcCard,gtemplateParam.templateCallingWay.isQrCode,ptQR);

                        }

                    } 
                     else
                     {
                        //判定模板的呼梯方式
//                        log_d("Now it's normal operation mode \r\n");
                        packetUserData((char *)sendBuff,gtemplateParam.templateCallingWay.isIcCard,gtemplateParam.templateCallingWay.isQrCode,ptQR);

                     }  

                    log_d("ptQR = %s,len = %d,state = %d\r\n",ptQR->data,ptQR->dataLen,ptQR->state);

                    if(ptQR->state)
                    {
                    	/* 使用消息队列实现指针变量的传递 */
                    	if(xQueueSend(xDataProcessQueue,              /* 消息队列句柄 */
                    				 (void *) &ptQR,   /* 发送指针变量recv_buf的地址 */
                    				 (TickType_t)50) != pdPASS )
                    	{
                            log_d("the queue is full!\r\n");                
                            xQueueReset(xDataProcessQueue);
                        } 
                        else
                        {
                            log_d("xQueueSend ok\r\n");     
                            log_d("ptQR = %s,len = %d,state = %d\r\n",ptQR->data,ptQR->dataLen,ptQR->state);
                        }
                        
                    }

                    xReturn = xSemaphoreGive(CountSem_Handle);// 给出计数信号量                        
                    semavalue=uxSemaphoreGetCount(CountSem_Handle); //获取计数型信号量值 
//                    comClearRxFifo(COM5);
//                    bsp_Usart5_RecvReset();
                    memset(sendBuff,0x00,sizeof(sendBuff));
                    len = 0;
                }

//                comClearRxFifo(COM5);
//                bsp_Usart5_RecvReset();
                memset(sendBuff,0x00,sizeof(sendBuff));
                len = 0;
            }
            else
            {
//                comClearRxFifo(COM5);
//                bsp_Usart5_RecvReset();
                memset(sendBuff,0x00,sizeof(sendBuff));
                len = 0;
            }
        }
        else
        {
//            comClearRxFifo(COM5);
//            bsp_Usart5_RecvReset();
            memset(sendBuff,0x00,sizeof(sendBuff));
            len = 0;
        }       
       
}

    	/* 发送事件标志，表示任务正常运行 */        
    	xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_4);  
        vTaskDelay(200);        
    }

}


/*
该算法总体思想是计算给定日期到 0年3月1日的天数，然后相减，获取天数的间隔。

m1 = (month_start + 9) % 12; 用于判断日期是否大于3月（2月是判断闰年的标识），还用于纪录到3月的间隔月数。

y1 = year_start - m1/10; 如果是1月和2月，则不包括当前年（因为是计算到0年3月1日的天数）。

d1 = 365*y1 + y1/4 - y1/100 + y1/400 + (m1*306 + 5)/10 + (day_start - 1);

? ? 其中 365*y1 是不算闰年多出那一天的天数，

? ? y1/4 - y1/100 + y1/400 ?是加所有闰年多出的那一天，

(m2*306 + 5)/10?用于计算到当前月到3月1日间的天数，306=365-31-28（1月和2月），5是全年中不是31天月份的个数

(day_start - 1)?用于计算当前日到1日的间隔天数。
————————————————
原文链接：https://blog.csdn.net/a_ran/article/details/43601699
*/



//返回date1和date2中间相关的天数，若date2小于date1，则返回负数
//只能计算2000~2099年
static int compareDate(uint8_t *date1,uint8_t *date2)
{
    int y2, m2, d2;
	int y1, m1, d1;
    int year_start,month_start,day_start;
    int year_end,month_end,day_end;
    char buff[4] = {0};

    if(strlen((const char*)date2) < 9)
    {
        return -1;//错误的日期
    }

    memcpy(buff,date1,4);
    year_start = atoi(buff);

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,date1+5,2);    
    month_start = atoi(buff);

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,date1+8,2);      
    day_start = atoi(buff);   

    //2020-03-01 18:42:25  
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,date2,4);
    year_end = atoi(buff);

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,date2+5,2);
    month_end = atoi(buff);
        
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,date2+8,2);        
    day_end = atoi(buff);

	m1 = (month_start + 9) % 12;
	y1 = year_start - m1/10;
	d1 = 365*y1 + y1/4 - y1/100 + y1/400 + (m1*306 + 5)/10 + (day_start - 1);
 
	m2 = (month_end + 9) % 12;
	y2 = year_end - m2/10;
	d2 = 365*y2 + y2/4 - y2/100 + y2/400 + (m2*306 + 5)/10 + (day_end - 1);
	
	return (d2 - d1);    

}

//判定当前时间段，是否在time123时间段内，在内返回：1，不是返回-1，若没有指定时间段则返回0
static int compareTime(uint8_t *currentTime)
{
    int ret = -1;
    int localTime = 0;
    char buff[4] = {0};

    int begin1,begin2,begin3,end1,end2,end3;

    if(strlen((const char*)gtemplateParam.holidayMode[0].startTime)==0 && strlen((const char*)gtemplateParam.holidayMode[1].startTime)==0 && strlen((const char*)gtemplateParam.holidayMode[2].startTime)==0)
    {
        //没有指定时间段
        ret = 0;
        return ret;
    }    
    //把时间转换为分钟
    //2020-03-01 18:42:25  
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,currentTime+11,2);    
    localTime = atoi(buff)*60;

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,currentTime+14,2);    
    localTime += atoi(buff);    
    
    //17:30
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gtemplateParam.holidayMode[0].startTime,2);
    begin1 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gtemplateParam.holidayMode[0].startTime+3,2);
    begin1 += atoi(buff); 

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gtemplateParam.holidayMode[1].startTime,2);
    begin2 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gtemplateParam.holidayMode[1].startTime+3,2);
    begin2 += atoi(buff);   

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gtemplateParam.holidayMode[2].startTime,2);
    begin3 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gtemplateParam.holidayMode[2].startTime+3,2);
    begin3 += atoi(buff);   

    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gtemplateParam.holidayMode[0].endTime,2);
    end1 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gtemplateParam.holidayMode[0].endTime+3,2);
    end1 += atoi(buff);   
    
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gtemplateParam.holidayMode[1].endTime,2);
    end2 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gtemplateParam.holidayMode[1].endTime+3,2);
    end2 += atoi(buff);   
    
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gtemplateParam.holidayMode[2].endTime,2);
    end3 = atoi(buff)*60;
    memset(buff,0x00,sizeof(buff));
    memcpy(buff,gtemplateParam.holidayMode[2].endTime+3,2);
    end3 += atoi(buff);       


    if(localTime>begin1 && localTime<end1)
    {
        ret = 1;
    }

    
    if(localTime>begin2 && localTime<end2)
    {
        ret = 1;
    }

    
    if(localTime>begin3 && localTime<end3)
    {
        ret = 1;
    }
    return ret;
}


static void packetUserData(char *src,int icFlag,int qrFlag,READER_BUFF_STRU *desc)
{
    READER_BUFF_STRU readerBuff = {0}; 
    uint8_t key[16] ={ 0x82,0x5d,0x82,0xd8,0xd5,0x2f,0xdf,0x85,0x28,0xa2,0xb5,0xd8,0x88,0x88,0x88,0x88 }; 
    uint8_t bcdBuff[512] = {0};
    memset(&readerBuff,0x00,sizeof(READER_BUFF_STRU));  

//    uint8_t offset = 0;

    //默认是支持上送的
    readerBuff.state = ENABLE;

    //去掉0D0A
    if(strlen(src)-2 >QUEUE_BUF_LEN)
    {
        readerBuff.dataLen = QUEUE_BUF_LEN;
    }
    else
    {
        readerBuff.dataLen = strlen(src)-2;
        //offset = readerBuff.dataLen;
    }
    
    //判定是刷卡还是QR
    if(strstr_t((const char*)src,(const char*)"CARD") == NULL)
    {
        //QR
//        readerBuff.authMode = AUTH_MODE_QR;   
//        asc2bcd(bcdBuff, (uint8_t *)src, readerBuff.dataLen, 0);
//        Des3_2(key, bcdBuff, readerBuff.dataLen/2, (uint8_t *)readerBuff.data, 1);
//        log_d("QR = %s\r\n",readerBuff.data);

        readerBuff.authMode = AUTH_MODE_QR;   
        memcpy(readerBuff.data,src,readerBuff.dataLen);
        log_d("QR = %s\r\n",readerBuff.data);
    }
    else
    {
        readerBuff.dataLen = 8;//卡号长度为8
        readerBuff.authMode = AUTH_MODE_CARD;  
//        offset -= 16;
        //CARD 120065AA89000000000 所以offset = 7
        memcpy(readerBuff.data,src + 9,readerBuff.dataLen);

        log_i("card NO. = %s\r\n",readerBuff.data);
    }
    
//    if(icFlag == 0)
//    {
//        //赋值   
//        if(readerBuff.authMode == AUTH_MODE_CARD)
//        {
//            readerBuff.state = DISABLE;
//            readerBuff.dataLen = 0;
//            memset(readerBuff.data,0x00,sizeof(readerBuff.data)); 
//            log_d("no support IC card\r\n");
//        }
//    }
//    
//    if(qrFlag == 0)
//    {
//        //赋值   
//        if(readerBuff.authMode == AUTH_MODE_QR)
//        {
//            readerBuff.state = DISABLE;
//            readerBuff.dataLen = 0;
//            memset(readerBuff.data,0x00,sizeof(readerBuff.data)); 
//            log_d("no support QR code\r\n");
//        }                              
//    } 
    
    *desc = readerBuff;    
}


static uint8_t parseReader(void)
{
    uint8_t ch = 0;   
    
    while(1)
    {    
        //读取485数据，若是没读到，退出，再重读
        if(bsp_Usart5_RecvOne(&ch) !=1)
        {            
            return UNFINISHED;
        }
        
        //读取缓冲区数据到BUFF
        gReaderData.rxBuff[gReaderData.rxCnt++] = ch;
        
//        log_d("ch = %c,gReaderData.rxBuff = %c \r\n",ch,gReaderData.rxBuff[gReaderData.rxCnt-1]);
         
        //最后一个字节为回车，或者总长度为510后，结束读取
        if(gReaderData.rxBuff[gReaderData.rxCnt-1] == 0x0A || gReaderData.rxCnt >=510)
        {   
            
           if(gReaderData.rxBuff[gReaderData.rxCnt-1] == 0x0A)
           {
//                dbh("gReaderData.rxBuff", (char*)gReaderData.rxBuff, gReaderData.rxCnt);  
//                log_d("gReaderData.rxBuff = %s,len = %d\r\n",gReaderData.rxBuff,gReaderData.rxCnt-1);
                return FINISHED;
           }

            //若没读到最后一个字节，但是总长到位后，清空，重读缓冲区
            memset(&gReaderData,0xFF,sizeof(FROMREADER_STRU));
        }
        
    }   

   
}



            
