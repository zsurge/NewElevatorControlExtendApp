/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年7月9日
  最近修改   :
  功能描述   : 这是硬件底层驱动程序的主文件。每个可以 #include "bsp.h" 来包含所有的外设驱动模块。
            bsp = Borad surport packet 板级支持包
  函数列表   :
  修改历史   :
  1.日    期   : 2019年7月9日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "bsp.h"
/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define HARDWARE_VERSION               "V1.0.1"
#define SOFTWARE_VERSION               "V1.0.4"

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

static void my_mem_init(void)
{
	mymem_init(SRAMIN);								//初始化内部内存池
	mymem_init(SRAMEX);								//初始化外部内存池
	mymem_init(SRAMCCM);	  					    //初始化CCM内存池
}


 void bsp_Init(void)
{
    NVIC_SetVectorTable(NVIC_VectTab_FLASH,0x30000);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	delay_init(168);            //初始化延时函数

	bsp_TIM6_Init();            //定时器6初始化
	
    bsp_InitUart();
    bsp_Usart5_Init(9600);

    FRAM_Init();              //eeprom初始化
    bsp_ds1302_init(); //DS1302_GPIO_Init();             //时钟芯片初始化
    
//    bsp_rtc_init();             //初始化RTC，只能放在UART后面

	bsp_key_Init();             //按键初始化

	bsp_LED_Init();		        //初始化LED端口	  

    bsp_HC595Init();            //初始化数码管
    
    STM_FLASH_Init();           //芯片内部FLASH初始化
    easyflash_init();           //外部FLASH初始化，使用easyflash    
    
//    bsp_spi_flash_init();

//    bsp_beep_init();            //蜂鸣器初始化        

//    bsp_WiegandInit();          //韦根读卡器初始化

    my_mem_init();                  //对内存进行初始化

  /* CmBacktrace initialize */
//   cm_backtrace_init("ElevatorControlAPP", HARDWARE_VERSION, SOFTWARE_VERSION);

}

