/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : Hc595_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月25日
  最近修改   :
  功能描述   : 数码管操作
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月25日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "hc595_task.h"
#include "bsp_digitaltube.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define DISPLAY_TASK_PRIO	( tskIDLE_PRIORITY + 2)
#define DISPLAY_STK_SIZE    (configMINIMAL_STACK_SIZE)  

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskDisplay = NULL;      //数码管


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
const char *HC595TaskName = "vHC595Task"; 

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
void vTaskDisplay(void *pvParameters);

void CreateHc595Task(void)
{
    //数码管
    xTaskCreate((TaskFunction_t )vTaskDisplay,
                (const char*    )HC595TaskName,       
                (uint16_t       )DISPLAY_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )DISPLAY_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskDisplay);     

}

//6230351869440954

static void vTaskDisplay(void *pvParameters)
{
    while(1)
    {
        bsp_HC595Show('A',0,9);
        vTaskDelay(1000);
        bsp_HC595Show(0,0,2);
        vTaskDelay(1000);

        
        /* 发送事件标志，表示任务正常运行 */
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_3);     

    }  

}



