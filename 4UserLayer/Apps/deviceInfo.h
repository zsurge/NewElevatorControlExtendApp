/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : deviceInfo.h
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年6月28日
  最近修改   :
  功能描述   : 设备各种参数设置及默认值
  函数列表   :
  修改历史   :
  1.日    期   : 2020年6月28日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
#ifndef __DEVICEINFO_H__
#define __DEVICEINFO_H__
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "stm32f4xx.h" 

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
 #define UPGRADE_URL_MAX_LEN    300
 #define MQTT_TOPIC_MAX_LEN     128

 #define DEFAULT_INIVAL 0xAA55AA55
 #define DEFAULT_BASE_INIVAL 0xAA55AA55 
 #define DEFAULT_DEV_NAME "ELEVATOR"

 #define DEVICE_DISABLE 0x00
 #define DEVICE_ENABLE  0x5555AAAA

 #define WRITE_PRARM    0x01
 #define READ_PRARM     0x02


//#define DEFAULT_TEMPLATE_PARAM                                              
//{                                                                             
//    {.id = 1,
//    .modeType = "5",
//    .templateCode = "100000",
//    .templateName = "defaultName",
//    .templateStatus = 1,
//    .callingWay = "1,2,3",
//    .offlineProcessing = 2
//    }
//}


/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
////////////////////模板信息////////////////////////////////////////
 //创建人信息
 typedef struct
 {
     uint8_t createorID[12];
     uint8_t createTime[20];
     uint8_t createCompanyId[12];
     uint8_t updateUserId[12];
     uint8_t updateUserName[20];    
 }CREATOR_INFO_STRU;
 
 typedef struct
 {
     uint8_t channelType;//通道类型 1、智慧通道 2、梯控
     uint8_t voiceSize;
     uint8_t templateType;//模板模式类型 1、高峰模式 2、节假日模式
     uint8_t startTime[20];
     uint8_t endTime[20];
 }TEMPLATE_BASE_DATA_STRU;
 
 typedef struct 
 {
     uint8_t isFace;
     uint8_t isQrCode;
     uint8_t isIcCard;          
 }CALLINGWAY_STRU;
 
 typedef struct
 {
     uint8_t isPeakMode;
     uint8_t isHolidayMode;
     uint8_t isNormalMode;          
 }WORKMODE_STRU;
 
 typedef struct 
 {
     uint8_t callingWay[8];
     uint8_t beginTime[20];
     uint8_t endTime[20];
     uint8_t outsideTimeMode[20];
     uint8_t outsideTimeData[20];          
 }TEMPLATE_SET_DATA_STRU;

typedef union
{
	unsigned int iFlag;        //
	unsigned char cFlag[4];    //卡号按字符
}DEVICE_SWITCH;

 
 typedef struct
 {
     DEVICE_SWITCH initFlag;   //初始化标志\x55\xAA\x55\xBB认定为已做初始化
     uint8_t id;
     uint8_t modeType[8];
     uint8_t templateCode[20];
     uint8_t templateName[50];
     uint8_t templateStatus;
     uint8_t callingWay[8];
     uint8_t offlineProcessing;
     uint8_t peakModeCnt;
     WORKMODE_STRU workMode;
     CALLINGWAY_STRU templateCallingWay;
     CALLINGWAY_STRU peakCallingWay;
     TEMPLATE_BASE_DATA_STRU peakMode[3];
     TEMPLATE_BASE_DATA_STRU holidayMode[3];
     TEMPLATE_SET_DATA_STRU peakInfo[1];
     TEMPLATE_SET_DATA_STRU hoildayInfo[1];
     CREATOR_INFO_STRU creatorInfo;     
 }TEMPLATE_PARAM_STRU;
//////////////////////////////////////////////////////////////////
 
//升级状态
typedef enum UPGRADE_STATUS
{
    UPGRADING = 0,
    UPGRADE_COMPLETE,
    UPGRADE_FAILED
}UPGRADE_STATUS_ENUM;

typedef struct DEVICE_ID
{
    DEVICE_SWITCH downLoadFlag;       //
    char qrSn[8];                     //二维码下发的本机ID
    char qrSnLen;
    char deviceSn[32];                //MQTT 订阅时的SN      
    char deviceSnLen;
}DEVICE_ID_STRU;

typedef struct UPGRADE_URL
{
    char upgradeState;//升级中，升级完成，升级失败
    char url[UPGRADE_URL_MAX_LEN];//用于存储升级文件存放地址
    char retUrl[UPGRADE_URL_MAX_LEN];//通知服务器升级状态的数据
}UPGRADE_URL_STRU;

typedef struct
{
    char publish[MQTT_TOPIC_MAX_LEN];   //发布的主题
    char subscribe[MQTT_TOPIC_MAX_LEN]; //订阅的主题
}MQTT_TOPIC_STRU;




typedef struct DEV_BASE_PARAM
{
    //设备状态
    DEVICE_SWITCH deviceState; //\x55\xAA\x55\xBB 设备可用  
    
    //SN
    DEVICE_ID_STRU deviceCode;

    //升级参数
    UPGRADE_URL_STRU upgradeInfo;   

    //MQTT参数
    MQTT_TOPIC_STRU mqttTopic;    
}DEV_BASE_PARAM_STRU;

///////////////////////FLASH相关///////////////////////////////////////////

//存储表头的一些索引值
typedef struct RECORDINDEX
{
    volatile uint32_t cardNoIndex;      //当前已存储了多少个卡号
    volatile uint32_t userIdIndex;      //当前已存储了多少个用户号
    volatile uint32_t delCardNoIndex;   //当前已存储了多少个已删除的卡号
    volatile uint32_t delUserIdIndex;   //当前已存储了多少个已删除的用户号
}RECORDINDEX_STRU;



/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
extern TEMPLATE_PARAM_STRU gtemplateParam;
extern DEV_BASE_PARAM_STRU gDevBaseParam;
extern RECORDINDEX_STRU gRecordIndex;


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

#endif

