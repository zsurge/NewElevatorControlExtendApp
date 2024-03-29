/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Key_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月25日
  最近修改   :
  功能描述   : 处理按键的任务
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月25日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "key_task.h"
#include "bsp_key.h"
#include "bsp_dipSwitch.h"
#include "bsp_ds1302.h"
#include "easyflash.h"
#include "tool.h"
#include "bsp_beep.h"
#include "localdata.h"
#include "bsp_MB85RC128.h"
#include "test.h"


#define LOG_TAG    "keyTask"
#include "elog.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define KEY_STK_SIZE        (configMINIMAL_STACK_SIZE*4)
#define KEY_TASK_PRIO	    ( tskIDLE_PRIORITY + 3)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *keyTaskName = "vKeyTask";     

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskKey = NULL;
#define MAX_TIME_OUT    3000


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskKey(void *pvParameters);
static void check_msg_queue(void);



void CreateKeyTask(void)
{
    //按键
    xTaskCreate((TaskFunction_t )vTaskKey,         
                (const char*    )keyTaskName,       
                (uint16_t       )KEY_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )KEY_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskKey); 
}


#if 0
static void vTaskKey(void *pvParameters)
{
    
	uint8_t ucKeyCode;
	uint8_t pcWriteBuffer[1024];


    uint32_t g_memsize;

    log_d("start vTaskKey\r\n");
    while(1)
    {
        ucKeyCode = bsp_Key_Scan(0);      
		
		if (ucKeyCode != KEY_NONE)
		{
            //dbg("ucKeyCode = %d\r\n",ucKeyCode);
              
			switch (ucKeyCode)
			{
				/* K1键按下 打印任务执行情况 */
				case KEY_SET_PRES:	             
					printf("=================================================\r\n");
					printf("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
					vTaskList((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);
                    
					printf("\r\n任务名       运行计数         使用率\r\n");
					vTaskGetRunTimeStats((char *)&pcWriteBuffer);
					printf("%s\r\n", pcWriteBuffer);          

                    g_memsize = xPortGetFreeHeapSize();
                    printf("系统当前内存大小为 %d 字节，开始申请内存\r\n",g_memsize);
//                    farm_read();
					break;				
				/* K2键按下，打印串口操作命令 */
				case KEY_RR_PRES:                 
                    check_msg_queue();                    

//                    bsp_ds1302_mdifytime("2020-01-17 09:24:15");
                    log_d("read gpio0 = %d\r\n",bsp_dipswitch_read()); 
//                    log_d("read gpio1 = %d\r\n",((bsp_dipswitch_read() & 0x01) +1)); //第1位用来表示机器ID                
//                    log_d("read gpio2 = %d\r\n",(((bsp_dipswitch_read()>>1) & 0x07)));//第2，3，4用来补偿负楼层
                    log_d("dev sn = %d\r\n",((bsp_dipswitch_read() & 0x03) +1)); //第1位用来表示机器ID 
                    log_d("calc floor num = %d\r\n",(((bsp_dipswitch_read()>>2) & 0x03)));//第2，3，4用来补偿负楼层

                    if(DIP4)
                    {
                        log_d("DIP4 = 1\r\n");
                    }
                    else
                    {
                        log_d("DIP4 = 0\r\n");
                    }

                    if(DIP5)
                    {
                        log_d("DIP5 = 1\r\n");
                    }
                    else
                    {
                        log_d("DIP5 = 0\r\n");
                    }                    
                    
//                      searchHeadTest("24450854");
//                    farm_test();

//                    testSplit();
//                    eraseUserDataAll();
//                      ee_test();
//			        
					break;
				case KEY_LL_PRES:   
                    log_i("KEY_DOWN_K3\r\n");
//                    searchHeadTest("16707692");
//                    ef_env_set_default();
//                    calcRunTime();       
                    bsp_ds1302_mdifytime("2021-01-09 14:26:00");
                    log_d("bsp_ds1302_readtime = %s\r\n",bsp_ds1302_readtime());
//                    ef_set_env_blob("device_sn","88888888",8); 
//                    ef_print_env();
//                    searchHeaderIndex("00012926",USER_MODE,&index);


					break;
				case KEY_OK_PRES:    
//                    test_env();
                   
                     log_d("KEY_DOWN_K4\r\n");
//                     eraseUserDataAll();
//                    ef_set_env_blob("remote_sn","7A13DCC67054F72CC07F",20);
//                ef_set_env_blob("remote_sn","823545AE9B2345B08FD8",20);
                    
					break;                
				
				/* 其他的键值不处理 */
				default:   
				log_e("KEY_default\r\n");
					break;
			}
		}

        /* 发送事件标志，表示任务正常运行 */
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);
		
		vTaskDelay(20);
	}   

}

#endif
static void vTaskKey(void *pvParameters)
{
    int32_t iTime1, iTime2;

    while(1)
    {
        switch (Key_Scan(GPIOE, GPIO_Pin_2))
        {
            case KEY_ON: 
                log_d("GPIO_Pin_2\r\n");
                iTime1 = xTaskGetTickCount();   /* 记下开始时间 */      
                if(DIP4)
                {
                    log_d("DIP4 = 1\r\n");
                }
                else
                {
                    log_d("DIP4 = 0\r\n");
                }

                if(DIP5)
                {
                    log_d("DIP5 = 1\r\n");
                }
                else
                {
                    log_d("DIP5 = 0\r\n");
                }                 

                break;            
            case KEY_HOLD:     
                log_d("GPIO_Pin_2 Long\r\n");                
                break;
            case KEY_OFF:  
                iTime2 = xTaskGetTickCount();	/* 记下结束时间 */

                if(iTime2 - iTime1 > MAX_TIME_OUT)
                {
                    log_d("GPIO_Pin_2 on\r\n");
                }           
                break;
            case KEY_ERROR:              
                break;
            default:
                break;
        }
        switch (Key_Scan(GPIOE, GPIO_Pin_3))
        {
            case KEY_ON: 
                log_d("GPIO_Pin_3\r\n");
                log_d("read gpio1 = %d\r\n",((bsp_dipswitch_read() & 0x01) +1)); //第1位用来表示机器ID                
                log_d("read gpio2 = %d\r\n",(((bsp_dipswitch_read()>>1) & 0x07)));//第2，3，4用来补偿负楼层                
                iTime1 = xTaskGetTickCount();   /* 记下开始时间 */   
                

                break;            
            case KEY_HOLD:      
                break;
            case KEY_OFF:  
                iTime2 = xTaskGetTickCount();	/* 记下结束时间 */

                if(iTime2 - iTime1 > MAX_TIME_OUT)
                {
                }           
                break;
            case KEY_ERROR:              
                break;
            default:
                break;
        }
        switch (Key_Scan(GPIOE, GPIO_Pin_4))
        {
            case KEY_ON: 
                log_d("GPIO_Pin_4\r\n");
                iTime1 = xTaskGetTickCount();   /* 记下开始时间 */              

                break;            
            case KEY_HOLD:      
                break;
            case KEY_OFF:  
                iTime2 = xTaskGetTickCount();	/* 记下结束时间 */

                if(iTime2 - iTime1 > MAX_TIME_OUT)
                {
                }           
                break;
            case KEY_ERROR:              
                break;
            default:
                break;
        }
        switch (Key_Scan(GPIOA, GPIO_Pin_0))
        {
            case KEY_ON: 
                log_d("GPIO_Pin_0\r\n");
                iTime1 = xTaskGetTickCount();   /* 记下开始时间 */              

                break;            
            case KEY_HOLD:      
                break;
            case KEY_OFF:  
                iTime2 = xTaskGetTickCount();	/* 记下结束时间 */

                if(iTime2 - iTime1 > MAX_TIME_OUT)
                {
                    log_d ( "eraseUserDataAll开始记时\r\n");
                    eraseUserDataAll();
                }           
                break;
            case KEY_ERROR:              
                break;
            default:
                break;
        }

        /* 发送事件标志，表示任务正常运行 */
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_2);
		
		vTaskDelay(20);        
    }
}


//查询Message_Queue队列中的总队列数量和剩余队列数量
void check_msg_queue(void)
{
    
	u8 msgq_remain_size;	//消息队列剩余大小
    u8 msgq_total_size;     //消息队列总大小
    
    taskENTER_CRITICAL();   //进入临界区
    msgq_remain_size=uxQueueSpacesAvailable(xDataProcessQueue);//得到队列剩余大小
    msgq_total_size=uxQueueMessagesWaiting(xDataProcessQueue)+uxQueueSpacesAvailable(xDataProcessQueue);//得到队列总大小，总大小=使用+剩余的。
	printf("Total Size = %d, Remain Size = %d\r\n",msgq_total_size,msgq_remain_size);	//显示DATA_Msg消息队列总的大小

    taskEXIT_CRITICAL();    //退出临界区
}






