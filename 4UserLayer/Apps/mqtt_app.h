/**********************************************************************************************************
** 文件名		:mqtt_app.h
** 作者			:maxlicheng<licheng.chn@outlook.com>
** 作者github	:https://github.com/maxlicheng
** 作者博客		:https://www.maxlicheng.com/	
** 生成日期		:2018-08-08
** 描述			:mqtt服务程序头文件
************************************************************************************************************/
#ifndef _MQTT_APP_H_
#define _MQTT_APP_H_
#include "sys.h"

//用户需要根据设备信息完善以下宏定义中的四元组内容
#define PRODUCT_KEY    	"a1Yjxb6GjGk"															//阿里云颁发的产品唯一标识，11位长度的英文数字随机组合
#define DEVICE_NAME    	"elevator"																//用户注册设备时生成的设备唯一编号，支持系统自动生成，也可支持用户添加自定义编号，产品维度内唯一
#define DEVICE_SECRET  	"vfW2KtmvfGy9AcBwNY9h4wksJifwt2Lf"				//设备密钥，与DeviceName成对出现，可用于一机一密的认证方案


#define CMD_ID   "commandCode"


#define   KEEPLIVE_TIME   50
#define   MQTT_VERSION    4

//以下宏定义固定，不需要修改


//#define CONTENT				"7A13DCC67054F72CC07F"//"clientId"DEVICE_NAME"deviceName"DEVICE_NAME"productKey"PRODUCT_KEY"timestamp789"	//计算登录密码用
#define CLIENT_ID			DEVICE_NAME"|12334444234234"											//客户端ID
#define USER_NAME			"admin"																													//客户端用户名
#define PASSWORD			"1234567812345678"																			//客户端登录password通过hmac_sha1算法得到，大小写不敏感


//#define   HOST_NAME       "192.168.110.78"     //服务器IP地址 线下 
//#define   HOST_PORT     1883    //由于是TCP连接，端口必须是1883
//#define DEVICE_PUBLISH		"/smartCloud/server/msg/device"			
//#define DEVICE_SUBSCRIBE	"/smartCloud/terminal/msg/"DEVICE_SN
//#define DEVICE_SN           "3E51E8848A4C00863617"

//#define DEVICE_SN           //"C7A052661C5D0CBE1E5F"
//#define DEVICE_SN         "0BF49D025715AFB3B0A1"


//设置设备属性

//以下三个TOPIC的宏定义不需要用户修改，可以直接使用
//IOT HUB为设备建立三个TOPIC：update用于设备发布消息，error用于设备发布错误，get用于订阅消息
//#define TOPIC_UPDATE         "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
//#define TOPIC_ERROR          "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
//#define TOPIC_GET            "/"PRODUCT_KEY"/"DEVICE_NAME"/get"

void mqtt_thread(void);
//u32 PublishData(float temp, float humid, unsigned char *payloadbuf);






#endif



