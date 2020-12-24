/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : test.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月28日
  最近修改   :
  功能描述   : 功能模块测试
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月28日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/
#ifndef __TEST_H
#define __TEST_H

//标准库头文件
#include "stdlib.h"

#include "cJSON.h"


//中间层头文件
#include "sys.h"
#include "delay.h"
#include "easyflash.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sfud.h"
#include "ini.h"




void searchHeadTest(uint8_t* header);



#endif


