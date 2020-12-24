/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : ini.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月28日
  最近修改   :
  功能描述   : 对参数进行操作
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月28日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

#ifndef __INI_H
#define __INI_H

#include "easyflash.h"
#include "version.h"
#include "string.h"
#include "deviceInfo.h"
#include "tool.h"
#include "LocalData.h"
#include "eth_cfg.h"
#include "cJSON.h"
#include "errorcode.h"
#include "calcDevNO.h"
#include "malloc.h"
#include "bsp_MB85RC128.h"
#include "bsp_ds1302.h"




//设备在线状态
#define ON_LINE                 1
#define OFF_LINE                (-1)

//呼梯状态：1、成功 0/2失败，3 QR设备已禁用  
#define CALL_OK                 1
#define CALL_NG                 2
#define CALL_ERR                0
#define QR_DISABLE              3

//进出方向 1、进 2、出
#define DIRECTION_IN            1
#define DIRECTION_OUT           2

#define MAX_FLOOR               200 //最高200层，超过200层，按负楼层处理

typedef uint8_t(*CallBackParam)(void * struParam,uint8_t mode,uint32_t len,uint32_t addr);

void readTemplateData(void);


void RestoreDefaultSetting(void);

void SystemUpdate(void);

//void readDevState(void);

void SaveDevState(uint32_t     state);

//void ReadLocalDevSn(void);


uint8_t packetPayload(USERDATA_STRU *localUserData,uint8_t *descJson);
//解析QRCODE数据
uint8_t parseQrCode(uint8_t *jsonBuff,USERDATA_STRU *qrCodeInfo);
//保存模板信息
SYSERRORCODE_E saveTemplateParam(uint8_t *jsonBuff);


//add 2020.07.06
void initTemplateParam(void);
void ClearTemplateParam(void);
uint8_t optTemplateParam(void *stParam,uint8_t mode,uint32_t len,uint32_t addr);

void initDevBaseParam(void);
void ClearDevBaseParam(void);
uint8_t optDevBaseParam(void *stParam,uint8_t mode,uint32_t len,uint32_t addr);

void initRecordIndex(void);
void ClearRecordIndex(void);
uint8_t optRecordIndex(RECORDINDEX_STRU *recoIndex,uint8_t mode);



void clearTemplateFRAM(void);



#endif
