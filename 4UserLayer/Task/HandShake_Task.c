/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : HandShake_Task.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年2月26日
  最近修改   :
  功能描述   : 握手任务处理
  函数列表   :
  修改历史   :
  1.日    期   : 2020年2月26日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "handshake_task.h"
#include "easyflash.h"
#include "stdlib.h"
#include "tool.h"
#include "bsp_ds1302.h"
#include "ini.h"
#include "bsp_digitaltube.h"
#include "bsp_led.h"


#define LOG_TAG    "handShake"
#include "elog.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HANDSHAKE_TASK_PRIO	( tskIDLE_PRIORITY)
#define HANDSHAKE_STK_SIZE  (configMINIMAL_STACK_SIZE*2)

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
const char *handShakeTaskName = "vHandShakeTask";      //看门狗任务名


/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskHandShake = NULL;      //LED灯


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void vTaskHandShake(void *pvParameters);
static void DisplayDevInfo (void);


static void vTaskHandShake(void *pvParameters)
{
//    uint32_t i_boot_times = NULL;
//    char *c_old_boot_times, c_new_boot_times[12] = {0};
//    uint8_t bcdbuf[6] = {0};  


//    log_d("start vTaskHandShake\r\n");
//    
//    /* get the boot count number from Env */
//    c_old_boot_times = ef_get_env("boot_times");

//    i_boot_times = atol(c_old_boot_times);
//    
//    /* boot count +1 */
//    i_boot_times ++;

//    /* interger to string */
//    sprintf(c_new_boot_times,"%012ld", i_boot_times);
//    
//    /* set and store the boot count number to Env */
//    ef_set_env("boot_times", c_new_boot_times);    

//    asc2bcd(bcdbuf,(uint8_t *)c_new_boot_times , 12, 0);

    //读取本地时间
    log_d("bsp_ds1302_readtime= %s\r\n",bsp_ds1302_readtime());

    //读取模板数据
//    readTemplateData();
//    readDevState();
//    clearTemplateFRAM();

//    eraseUserDataAll();

    initRecordIndex();
    
    initDevBaseParam();

    initTemplateParam();
    
    DisplayDevInfo();
    
//    vTaskDelay(500);
//    
//    vTaskDelete( NULL ); //删除自己

    while(1)
    {  
        bsp_HC595Show('A',0,1);
        vTaskDelay(800);
        bsp_HC595Show(0,0,2);
        vTaskDelay(800);

        LEDERROR = !LEDERROR;
		/* 发送事件标志，表示任务正常运行 */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_0);  
      
    }    
}

void CreateHandShakeTask(void)
{  
    //跟android握手
    xTaskCreate((TaskFunction_t )vTaskHandShake,
    (const char*    )handShakeTaskName,       
    (uint16_t       )HANDSHAKE_STK_SIZE, 
    (void*          )NULL,              
    (UBaseType_t    )HANDSHAKE_TASK_PRIO,    
    (TaskHandle_t*  )&xHandleTaskHandShake);  

}


static void DisplayDevInfo(void)
{
    printf("\r\n==========Version==========\r\n");
	printf("Softversion :%s\r\n",gDevinfo.SoftwareVersion);
    printf("HardwareVersion :%s\r\n", gDevinfo.HardwareVersion);
	printf("Model :%s\r\n", gDevinfo.Model);
	printf("ProductBatch :%s\r\n", gDevinfo.ProductBatch);	    
	printf("BulidDate :%s\r\n", gDevinfo.BulidDate);
	printf("DevSn :%s\r\n", gDevinfo.GetSn());
    printf("Devip :%s\r\n", gDevinfo.GetIP());
	printf("DevID :%s\r\n", gDevBaseParam.deviceCode.qrSn);
}




