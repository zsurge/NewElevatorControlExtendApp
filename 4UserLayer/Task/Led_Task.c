/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Led_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月25日
  最近修改   :
  功能描述   : LED灯控制
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月25日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "led_task.h"
#include "bsp_digitaltube.h"
#include "bsp_led.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define LED_TASK_PRIO	    (tskIDLE_PRIORITY)
#define LED_STK_SIZE 		(configMINIMAL_STACK_SIZE)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *ledTaskName = "vLedTask";      //LED任务名称


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskLed = NULL;      //LED灯


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskLed(void *pvParameters);

void CreateLedTask(void)
{
    //创建LED任务
    xTaskCreate((TaskFunction_t )vTaskLed,         
                (const char*    )ledTaskName,       
                (uint16_t       )LED_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )LED_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskLed);
}


//LED任务函数 
static void vTaskLed(void *pvParameters)
{ 
    while(1)
    {  
        bsp_HC595Show('A',0,9);
        vTaskDelay(800);
        bsp_HC595Show(0,0,2);
        vTaskDelay(800);

        LEDERROR = !LEDERROR;
		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_0);  
      
    }
} 


