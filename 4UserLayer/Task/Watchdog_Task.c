/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Watchdog_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月27日
  最近修改   :
  功能描述   : 看门狗任务
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月27日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "Watchdog_Task.h"
#include "bsp_iwdg.h"
#include "tool.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define WATCHDOG_TASK_PRIO		(tskIDLE_PRIORITY + 9)
#define WATCHDOG_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *WatchDogTaskName = "vWatchDogTask";  

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskWatchDog = NULL;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskWatchDog(void *pvParameters);

void CreateWatchDogTask(void)
{
    //看门狗
    xTaskCreate((TaskFunction_t )vTaskWatchDog,     
                (const char*    )WatchDogTaskName,   
                (uint16_t       )WATCHDOG_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )WATCHDOG_TASK_PRIO,
                (TaskHandle_t*  )&xHandleTaskWatchDog);
}



static void vTaskWatchDog(void *pvParameters)
{
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS; /* 最大延迟100ms */   
    
	/* 
	  开始执行启动任务主函数前使能独立看门狗。
	  设置LSI是32分频，下面函数参数范围0-0xFFF，分别代表最小值1ms和最大值4095ms
	  下面设置的是4s，如果4s内没有喂狗，系统复位。

      LSI分频可以为 [4,8,16,32,64,128,256],
      对应的最大时间为别为：409.6 819.2 1638.4 3276.8 6553.6 13107.226214.4ms

      计算公式为 Tout=(4 * 2^PR * （RL+1）)/40 ms
      最小时间单位：1/40 ms
      考虑分频系数 Prescaler（4-256）：1*Prescaler/40 ms
      考虑RL值(0-FFF)：1*Prescaler*（RL+1）/40 ms

      目前我们使用的是128分频，最大13.1秒
	*/
	bsp_InitIwdg(4000);
	
	/* 打印系统开机状态，方便查看系统是否复位 */
	printf("=====================================================\r\n");
	printf("=系统开机执行\r\n");
	printf("=====================================================\r\n");

	
	
    while(1)
    {   
        
		/* 等待所有任务发来事件标志 */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, /* 事件标志组句柄 */
							         TASK_BIT_ALL,       /* 等待TASK_BIT_ALL被设置 */
							         pdTRUE,             /* 退出前TASK_BIT_ALL被清除，这里是TASK_BIT_ALL都被设置才表示“退出”*/
							         pdTRUE,             /* 设置为pdTRUE表示等待TASK_BIT_ALL都被设置*/
							         xTicksToWait); 	 /* 等待延迟时间 */
		
		if((uxBits & TASK_BIT_ALL) == TASK_BIT_ALL)
		{  
		    IWDG_Feed(); //喂狗	
		}
	    else
		{
			/* 基本是每xTicksToWait进来一次 */
			/* 通过变量uxBits简单的可以在此处检测那个任务长期没有发来运行标志 */
		}
    }
}




