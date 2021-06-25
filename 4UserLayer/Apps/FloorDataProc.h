/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : FloorDataProc.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年12月23日
  最近修改   :
  功能描述   : 电梯控制器的指令处理文件
  函数列表   :
  修改历史   :
  1.日    期   : 2019年12月23日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/
#ifndef __FLOORDATAPROC_H_
#define __FLOORDATAPROC_H_


/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "errorcode.h"
#include "LocalData.h"
#include "bsp_uart_fifo.h"
#include "tool.h"
#include "bsp_dipSwitch.h"
#include "easyflash.h"
#include "cmdhandle.h"
#include "MQTTPacket.h"
#include "transport.h"
#include "jsonUtils.h"
#include "bsp_ds1302.h"
#include "ini.h"



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_DEV_NO 8
 
typedef struct
{
   uint8_t devSn;  
   uint32_t value;
}ELEVATOR_TRANBUFF_STRU;

typedef struct
{
    ELEVATOR_TRANBUFF_STRU data[MAX_DEV_NO];
}ELEVATOR_BUFF_STRU;



/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

extern ELEVATOR_TRANBUFF_STRU gElevtorData,gRecvElevtorData;



SYSERRORCODE_E packetToElevatorExtend(USERDATA_STRU *localUserData,ELEVATOR_BUFF_STRU *devSendData);//add 1204


SYSERRORCODE_E calcSingleFloor(uint8_t layer,ELEVATOR_BUFF_STRU *eBuf);

SYSERRORCODE_E calcMultilFloor(uint8_t *floorBuf,uint8_t num,ELEVATOR_BUFF_STRU *eBuf);
void sendQueueToDev(ELEVATOR_TRANBUFF_STRU *devSendData);


#endif


