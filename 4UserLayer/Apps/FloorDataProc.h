/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : FloorDataProc.h
  �� �� ��   : ����
  ��    ��   : �Ŷ�
  ��������   : 2019��12��23��
  ����޸�   :
  ��������   : ���ݿ�������ָ����ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2019��12��23��
    ��    ��   : �Ŷ�
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __FLOORDATAPROC_H_
#define __FLOORDATAPROC_H_


/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
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
 * �궨��                                       *
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
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

extern ELEVATOR_TRANBUFF_STRU gElevtorData,gRecvElevtorData;



SYSERRORCODE_E packetToElevatorExtend(USERDATA_STRU *localUserData,ELEVATOR_BUFF_STRU *devSendData);//add 1204


SYSERRORCODE_E calcSingleFloor(uint8_t layer,ELEVATOR_BUFF_STRU *eBuf);

SYSERRORCODE_E calcMultilFloor(uint8_t *floorBuf,uint8_t num,ELEVATOR_BUFF_STRU *eBuf);
void sendQueueToDev(ELEVATOR_TRANBUFF_STRU *devSendData);


#endif


