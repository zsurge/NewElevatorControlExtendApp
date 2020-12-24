/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : test.c
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

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "malloc.h"
#include "tool.h"
//驱动层头文件
#include "bsp.h"


#define LOG_TAG    "TEST"
#include "elog.h"

#define HEAD_MAX_NUM 30000

static char *createRandom(void);
static char *createRandom(void)
{ 
    char ret[8] = {0};  
    uint32_t num = 0;
//    srand(gRandomNum);
    num = rand()%30207068;      //产生一个0-32768的随机数

    sprintf(ret,"%08d",num);
    return ret;    
}


void searchHeadTest(uint8_t* header)
{


}





