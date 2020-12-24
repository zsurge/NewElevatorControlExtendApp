/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : ini.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年5月28日
  最近修改   :
  功能描述   : 参数设置
  函数列表   :
  修改历史   :
  1.日    期   : 2019年5月28日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#define LOG_TAG    "ini"
#include "elog.h"
#include "des.h"
#include "ini.h"
#include "stdlib.h"


TEMPLATE_PARAM_STRU gtemplateParam;
DEV_BASE_PARAM_STRU gDevBaseParam;
RECORDINDEX_STRU gRecordIndex;

static uint8_t opParam ( void* Param,uint8_t mode,uint32_t len,uint32_t addr );



/*****************************************************************************
 函 数 名  : RestoreDefaultSetting
 功能描述  : 恢复默认设置,实际是写入系统启动所需要的参数
           若使用ef_env_set_default 则需要在ef_port.c中对默认值进行修改
 输入参数  :

 输出参数  : 无
 返 回 值  : void

 修改历史      :
  1.日    期   : 2019年5月28日
    作    者   : 张舵

    修改内容   : 新生成函数

*****************************************************************************/
void RestoreDefaultSetting ( void )
{
	if ( ef_env_set_default() == EF_NO_ERR )
	{
		DBG ( "RestoreDefaultSetting success\r\n" );
	}

}

void SystemUpdate ( void )
{
	//写升级标志位
	if ( ef_set_env ( "WSPIFLASH","5050" ) == EF_NO_ERR )
	{
		ef_set_env ( "WMCUFLASH","6060" );
		//jump iap
		DBG ( "jump iap\r\n" );
		NVIC_SystemReset();
	}
}

#if 0
void readTemplateData ( void )
{
	char tempBuff[8] = {0};
	int valueLen = 0;
	int index = 0;
	char tmpIndex[2] = {0};
	char tmpKey[32] = {0};
	char* callingWay[4] = {0}; //存放分割后的子字符串
	int callingWayNum = 0;

	memset ( &gtemplateParam,0x00,sizeof ( TEMPLATE_PARAM_STRU ) );
	TEMPLATE_PARAM_STRU* templateParam = &gtemplateParam;


	//读取模板状态
	memset ( tempBuff,0x00,sizeof ( tempBuff ) );
	valueLen = ef_get_env_blob ( "templateStatus", tempBuff, sizeof ( tempBuff ), NULL );
	templateParam->templateStatus = atoi ( tempBuff );

	//读取模板是否支持离线状态
	memset ( tempBuff,0x00,sizeof ( tempBuff ) );
	valueLen = ef_get_env_blob ( "offlineStatus", tempBuff, sizeof ( tempBuff ), NULL );
	templateParam->offlineProcessing = atoi ( tempBuff );

	//读取工作模式，是否开节假日模式
	valueLen = ef_get_env_blob ( "modeType", templateParam->modeType, sizeof ( templateParam->modeType ), NULL );
	if ( valueLen == 0 )
	{
		log_d ( "get templateParam error!\r\n" );
	}

	//读取模板的识别模式
	valueLen = 0;
	valueLen = ef_get_env_blob ( "T_callingWay", templateParam->callingWay, sizeof ( templateParam->callingWay ), NULL );

	//解析模板识别方式
	split ( ( char* ) templateParam->callingWay,",",callingWay,&callingWayNum ); //调用函数进行分割
	while ( callingWayNum )
	{
		switch ( atoi ( callingWay[callingWayNum-1] ) )
		{
			case 1:
				templateParam->templateCallingWay.isFace = 1;
				break;
			case 2:
				templateParam->templateCallingWay.isQrCode = 1;
				break;
			case 3:
				templateParam->templateCallingWay.isIcCard = 1;
				break;
		}
		callingWayNum-- ;
	}


	//读取节假日的识别模式
	valueLen = 0;
	valueLen = ef_get_env_blob ( "peakCallingWay", templateParam->peakInfo[0].callingWay, sizeof ( templateParam->peakInfo[0].callingWay ), NULL );

	//解析识别方式
	memset ( callingWay,0x00,sizeof ( callingWay ) );
	callingWayNum = 0;
	split ( ( char* ) templateParam->peakInfo[0].callingWay,",",callingWay,&callingWayNum ); //调用函数进行分割
	while ( callingWayNum )
	{
		switch ( atoi ( callingWay[callingWayNum-1] ) )
		{
			case 1:
				templateParam->peakCallingWay.isFace = 1;
				break;
			case 2:
				templateParam->peakCallingWay.isQrCode = 1;
				break;
			case 3:
				templateParam->peakCallingWay.isIcCard = 1;
				break;
		}
		callingWayNum-- ;
	}


	//解析是否开启节假日模式
	memset ( callingWay,0x00,sizeof ( callingWay ) );
	callingWayNum = 0;
	split ( ( char* ) templateParam->modeType,",",callingWay,&callingWayNum ); //调用函数进行分割
	while ( callingWayNum )
	{
		switch ( atoi ( callingWay[callingWayNum-1] ) )
		{
			case 1:
				templateParam->workMode.isPeakMode = 1;
				break;
			case 2:
				templateParam->workMode.isHolidayMode = 1;
				break;
			case 5:
				templateParam->workMode.isNormalMode = 1;
				break;
		}
		callingWayNum-- ;
	}


	//读取节假日排除日期的类型,1周六，2周日，3节假日（目前节假日无法屏蔽）
	valueLen = 0;
	valueLen = ef_get_env_blob ( "peakHolidaysType", templateParam->peakInfo[0].outsideTimeMode, sizeof ( templateParam->peakInfo[0].outsideTimeMode ), NULL );

	//读取模板有效开始日期
	valueLen = 0;
	valueLen = ef_get_env_blob ( "peakStartDate", templateParam->peakInfo[0].beginTime, sizeof ( templateParam->peakInfo[0].beginTime ), NULL );

	//读取模板有效结束日期
	valueLen = 0;
	valueLen = ef_get_env_blob ( "peakEndDate", templateParam->peakInfo[0].endTime, sizeof ( templateParam->peakInfo[0].endTime ), NULL );

	//先判定有几个时间段
	memset ( tempBuff,0x00,sizeof ( tempBuff ) );
	valueLen = ef_get_env_blob ( "holidayTimeMapCnt", tempBuff, sizeof ( tempBuff ), NULL );
	templateParam->peakModeCnt = atoi ( tempBuff );


	memset ( tmpKey,0x00,sizeof ( tmpKey ) );
	memset ( tmpIndex,0x00,sizeof ( tmpIndex ) );
	for ( index= 0; index<templateParam->peakModeCnt; index++ )
	{
		//读取节假日模板的不受控时间段--开始时间
		sprintf ( tmpIndex,"%d",index );
		strcpy ( tmpKey,"hoildayModeStartTime" );
		strcat ( tmpKey,tmpIndex );


		valueLen = ef_get_env_blob ( tmpKey, templateParam->holidayMode[index].startTime, sizeof ( templateParam->holidayMode[index].startTime ), NULL );
		log_d ( "gtemplateParam->%s = %s\r\n",tmpKey,gtemplateParam.holidayMode[index].startTime );


		//读取节假日模板的不受控时间段--结束时间
		memset ( tmpKey,0x00,sizeof ( tmpKey ) );
		strcpy ( tmpKey,"hoildayModeEndTime" );
		strcat ( tmpKey,tmpIndex );
		valueLen = 0;
		valueLen = ef_get_env_blob ( tmpKey, templateParam->holidayMode[index].endTime, sizeof ( templateParam->holidayMode[index].endTime ), NULL );
		log_d ( "gtemplateParam->%s= %s\r\n",tmpKey,gtemplateParam.holidayMode[index].endTime );

	}


	log_d ( "gtemplateParam->peakInfo[0].beginTime = %s\r\n",gtemplateParam.peakInfo[0].beginTime );
	log_d ( "gtemplateParam->peakInfo[0].endTime = %s\r\n",gtemplateParam.peakInfo[0].endTime );


	log_d ( "gtemplateParam->templateCallingWay = %d\r\n",gtemplateParam.templateCallingWay.isFace );
	log_d ( "gtemplateParam->templateCallingWay = %d\r\n",gtemplateParam.templateCallingWay.isQrCode );
	log_d ( "gtemplateParam->templateCallingWay = %d\r\n",gtemplateParam.templateCallingWay.isIcCard );

	log_d ( "gtemplateParam->peakCallingWay = %d\r\n",gtemplateParam.peakCallingWay.isFace );
	log_d ( "gtemplateParam->peakCallingWay = %d\r\n",gtemplateParam.peakCallingWay.isQrCode );
	log_d ( "gtemplateParam->peakCallingWay = %d\r\n",gtemplateParam.peakCallingWay.isIcCard );


	log_d ( "gtemplateParam->templateStatus = %d\r\n",gtemplateParam.templateStatus );
	log_d ( "gtemplateParam->offlineProcessing = %d\r\n",gtemplateParam.offlineProcessing );
	log_d ( "gtemplateParam->modeType = %s\r\n",gtemplateParam.modeType );
	log_d ( "gtemplateParam->callingWay = %s\r\n",gtemplateParam.callingWay );
	log_d ( "gtemplateParam->peakInfo[0].callingWay = %s\r\n",gtemplateParam.peakInfo[0].callingWay );
	log_d ( "gtemplateParam->peakInfo[0].beginTime = %s\r\n",gtemplateParam.peakInfo[0].beginTime );
	log_d ( "gtemplateParam->peakInfo[0].endTime = %s\r\n",gtemplateParam.peakInfo[0].endTime );
	log_d ( "gtemplateParam->peakInfo[0].outsideTimeMode = %s\r\n",gtemplateParam.peakInfo[0].outsideTimeMode );

}
#endif




void SaveDevState ( uint32_t state )
{
	uint8_t ret = 0;
	//记录SN
	ClearDevBaseParam();
	optDevBaseParam ( &gDevBaseParam,READ_PRARM,sizeof ( DEV_BASE_PARAM_STRU ),DEVICE_BASE_PARAM_ADDR );
	gDevBaseParam.deviceState.iFlag = state;
	ret = optDevBaseParam ( &gDevBaseParam,WRITE_PRARM,sizeof ( DEV_BASE_PARAM_STRU ),DEVICE_BASE_PARAM_ADDR );

	if ( ret != 1 )
	{
		log_e ( "write device state error\r\n" );
	}
}

//void readDevState(void)
//{
//    uint8_t devState[5] = {0};
//
//    memset(devState,0x00,sizeof(devState));
//    ef_get_env_blob("DeviceState",devState,sizeof(devState) , NULL);
//    log_d("DeviceState = %s\r\n",devState);
//    gDevBaseParam.deviceState.iFlag = atoi((const char*)devState);
//
//    log_d("gDeviceStateFlag = %x\r\n",gDevBaseParam.deviceState.iFlag);

//}


uint8_t packetPayload ( USERDATA_STRU* localUserData,uint8_t* descJson )
{

	SYSERRORCODE_E result = NO_ERR;
	cJSON* root,*dataObj;
	char* tmpBuf;
	char tmpTime[32] = {0};

	root = cJSON_CreateObject();
	dataObj = cJSON_CreateObject();

	if ( !root && !dataObj )
	{
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
		cJSON_Delete ( root );
		my_free ( tmpBuf );
		return CJSON_CREATE_ERR;
	}

	cJSON_AddStringToObject ( root, "deviceCode", gDevBaseParam.deviceCode.deviceSn );
	log_d ( "deviceCode = %s\r\n",gDevBaseParam.deviceCode.deviceSn );

	cJSON_AddItemToObject ( root, "data", dataObj );

//    if(localUserData->platformType == 4 )
//    {
	cJSON_AddStringToObject ( root, "commandCode","3007" );
	cJSON_AddStringToObject ( dataObj, "cardNo", ( const char* ) localUserData->cardNo );
	cJSON_AddNumberToObject ( dataObj, "callType",localUserData->authMode );
	cJSON_AddNumberToObject ( dataObj, "status", ON_LINE );
	strcpy ( tmpTime, ( const char* ) bsp_ds1302_readtime() );
	cJSON_AddStringToObject ( dataObj, "callElevatorTime",tmpTime );
	cJSON_AddStringToObject ( dataObj, "timeStamp", ( const char* ) localUserData->startTime );
	cJSON_AddStringToObject ( dataObj, "userId", ( const char* ) localUserData->userId );

	if ( localUserData->authMode == 7 )
	{
		cJSON_AddNumberToObject ( dataObj, "type",localUserData->platformType );
		cJSON_AddNumberToObject ( dataObj, "callState",CALL_OK );
	}
	else
	{
		cJSON_AddNumberToObject ( dataObj, "type",CALL_OK );
	}
//    }
//    else
//    {
//        cJSON_AddStringToObject(root, "commandCode","4002");
//        cJSON_AddNumberToObject(dataObj, "enterType",  4);//这里设置为4是QRCODE在开发者平台上的数据，梯控上的数据为7
//        cJSON_AddStringToObject(dataObj, "qrId",  (const char*)localUserData->userId);
//        cJSON_AddNumberToObject(dataObj, "status", ON_LINE);
//        cJSON_AddStringToObject(dataObj, "enterTime",(const char*)bsp_ds1302_readtime());
//        cJSON_AddNumberToObject(dataObj, "faceCompare",CALL_OK);//：1、成功 2失败，
//        cJSON_AddNumberToObject(dataObj, "direction", DIRECTION_IN);//1进，2出
//        cJSON_AddStringToObject(dataObj, "cardNo", (const char*)localUserData->cardNo);
//    }

	tmpBuf = cJSON_PrintUnformatted ( root );

	if ( !tmpBuf )
	{
		log_d ( "cJSON_PrintUnformatted error \r\n" );
		cJSON_Delete ( root );
		my_free ( tmpBuf );
		return CJSON_FORMAT_ERR;
	}

	strcpy ( ( char* ) descJson,tmpBuf );

	cJSON_Delete ( root );
	my_free ( tmpBuf );
	return result;

}

//2020.09.07 数据格式有变化
//{"qS":"1599189101","datetime":"1599189101495","qE":"1599189161","expire":60,"f1":"#$%&()*+76543210/.-,","type":1,"ownerId":21310,"d2":".K.L.M.N.O.G"}

uint8_t parseQrCode ( uint8_t* jsonBuff,USERDATA_STRU* qrCodeInfo )
{
	cJSON* root=NULL,*tmpArray=NULL;
	uint8_t buf[300] = {0};
	uint8_t isFind = 0;
	uint32_t endTime = 0;
	uint8_t key[16] = { 0x82,0x5d,0x82,0xd8,0xd5,0x2f,0xdf,0x85,0x28,0xa2,0xb5,0xd8,0x88,0x88,0x88,0x88 };
	uint8_t bcd[17] = {0};
	uint8_t floorLen = 0;
	uint8_t i = 0;

	if ( !jsonBuff )
	{
		cJSON_Delete ( root );
		log_d ( "error json data\r\n" );
		return STR_EMPTY_ERR;
	}

	root = cJSON_Parse ( ( char* ) jsonBuff ); //解析数据包
	if ( !root )
	{
		cJSON_Delete ( root );
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
		return CJSON_PARSE_ERR;
	}

	tmpArray = cJSON_GetObjectItem ( root, "qS" );
	if ( tmpArray )
	{
		strcpy ( ( char* ) qrCodeInfo->startTime,tmpArray->valuestring );
		log_d ( "qrCodeInfo->startTime= %s\r\n",qrCodeInfo->startTime );

		endTime = atoi ( qrCodeInfo->startTime ) + 60;
		sprintf ( ( char* ) qrCodeInfo->endTime,"%d",endTime );
	}

//    tmpArray = cJSON_GetObjectItem(root, "qE");
//    strcpy((char *)qrCodeInfo->endTime,tmpArray->valuestring);
//    log_d("qrCodeInfo->endTime= %s\r\n",qrCodeInfo->endTime);

//    tmpArray = cJSON_GetObjectItem(root, "ownerId");
//    sprintf((char*)qrCodeInfo->userId,"%d",tmpArray->valueint);
//    log_d("qrCodeInfo->qrID= %s\r\n",qrCodeInfo->userId);

//    tmpArray = cJSON_GetObjectItem(root, "t");
//    qrCodeInfo->platformType = tmpArray->valueint;
//    log_d("qrCodeInfo->type= %d\r\n",qrCodeInfo->platformType);

	tmpArray = NULL;
	tmpArray = cJSON_GetObjectItem ( root, "i" );

	if ( tmpArray )
	{
		log_d ( "user id = %s,len = %d\r\n",tmpArray->valuestring,strlen ( ( const char* ) tmpArray->valuestring ) );

		asc2bcd ( bcd, ( uint8_t* ) tmpArray->valuestring, strlen ( ( const char* ) tmpArray->valuestring ), 1 );

		Des3_2 ( key, bcd, strlen ( ( const char* ) tmpArray->valuestring ) /2, qrCodeInfo->userId, 1 );

		log_d ( "qrCodeInfo->qrID= %s,value = %d\r\n",qrCodeInfo->userId,atoi ( qrCodeInfo->userId ) );
	}


	tmpArray = NULL;
	tmpArray = cJSON_GetObjectItem ( root, "f1" );
	if ( tmpArray )
	{
	    floorLen = strlen ( ( const char* ) tmpArray->valuestring );
		memcpy ( qrCodeInfo->accessFloor,parseAccessFloor ( tmpArray->valuestring ),strlen ( ( const char* ) tmpArray->valuestring ) );
	}

	memset ( buf,0x00,sizeof ( buf ) );
	tmpArray = NULL;
	tmpArray = cJSON_GetObjectItem ( root, "f2" );
	if ( tmpArray )
	{
		memcpy ( buf,parseAccessFloor ( tmpArray->valuestring ),strlen ( ( const char* ) tmpArray->valuestring ) );
        if(FLOOR_ARRAY_LENGTH - floorLen >= strlen ( ( const char* ) tmpArray->valuestring ))
        {
            for(i=0;i<strlen ( ( const char* ) tmpArray->valuestring );i++)
            {
                qrCodeInfo->accessFloor[floorLen+i] = buf[i]+MAX_FLOOR; //最高200层，超过200层，按负楼层处理
            }
        }

        dbh ( "qrCodeInfo->accessFloor  buf", ( char* ) qrCodeInfo->accessFloor,floorLen + strlen ( ( const char* ) tmpArray->valuestring ) );
        
	}

	qrCodeInfo->defaultFloor = qrCodeInfo->accessFloor[0];
	log_d ( "qrCodeInfo->defaultFloor = %d\r\n",qrCodeInfo->defaultFloor );

	memset ( buf,0x00,sizeof ( buf ) );
	tmpArray = NULL;
	tmpArray = cJSON_GetObjectItem ( root, "d1" );
	if ( tmpArray )
	{
		strcpy ( ( char* ) buf,tmpArray->valuestring );
		if ( strlen ( ( const char* ) buf ) > 0 )
		{
			log_d ( "d1 = %s\r\n",buf );
			if ( findDev ( buf,1 ) == 1 )
			{
				isFind =1;
				goto QR_END;
			}
		}
	}

	memset ( buf,0x00,sizeof ( buf ) );
	tmpArray = NULL;
	tmpArray = cJSON_GetObjectItem ( root, "d2" );
	if ( tmpArray )
	{
		strcpy ( ( char* ) buf,tmpArray->valuestring );
		if ( strlen ( ( const char* ) buf ) > 0 )
		{
			log_d ( "d2 = %s\r\n",buf );
			if ( findDev ( buf,2 ) == 1 )
			{
				isFind =1;
				goto QR_END;
			}
		}
	}

	memset ( buf,0x00,sizeof ( buf ) );
	tmpArray = NULL;
	tmpArray = cJSON_GetObjectItem ( root, "d3" );
	if ( tmpArray )
	{
		strcpy ( ( char* ) buf,tmpArray->valuestring );
		if ( strlen ( ( const char* ) buf ) > 0 )
		{
			log_d ( "d3= %s\r\n",buf );
			if ( findDev ( buf,3 ) == 1 )
			{
				isFind =1;
				goto QR_END;
			}
		}
	}


	memset ( buf,0x00,sizeof ( buf ) );
	tmpArray = NULL;
	tmpArray = cJSON_GetObjectItem ( root, "d4" );
	if ( tmpArray )
	{
		strcpy ( ( char* ) buf,tmpArray->valuestring );
		if ( strlen ( ( const char* ) buf ) > 0 )
		{
			log_d ( "d4 = %s\r\n",buf );
			if ( findDev ( buf,4 ) == 1 )
			{
				isFind =1;
				goto QR_END;
			}
		}
	}

	memset ( buf,0x00,sizeof ( buf ) );
	tmpArray = NULL;
	tmpArray = cJSON_GetObjectItem ( root, "d5" );
	if ( tmpArray )
	{
		strcpy ( ( char* ) buf,tmpArray->valuestring );
		if ( strlen ( ( const char* ) buf ) > 0 )
		{
			log_d ( "d5 = %s\r\n",buf );
			if ( findDev ( buf,5 ) == 1 )
			{
				isFind =1;
			}
		}
	}

QR_END:
	cJSON_Delete ( root );

	return isFind;

}



//保存模板信息
SYSERRORCODE_E saveTemplateParam ( uint8_t* jsonBuff )
{
	SYSERRORCODE_E result = NO_ERR;
	cJSON* root,*data,*templateData,*templateMap,*holidayTimeMap,*peakTimeMap;
	cJSON* tmpArray,*arrayElement;

	int holidayTimeMapCnt=0,peakTimeMapCnt=0,index = 0;
	uint8_t ret = 0;
	char tmpbuf[8] = {0};
	char tmpIndex[2] = {0};
	char tmpKey[32] = {0};
	char* callingWay[4] = {0}; //存放分割后的子字符串
	int callingWayNum = 0;

	TEMPLATE_PARAM_STRU* templateParam = &gtemplateParam;

	uint32_t curtick  =  xTaskGetTickCount();

	ClearTemplateParam();
	ret = optTemplateParam ( templateParam,READ_PRARM,sizeof ( TEMPLATE_PARAM_STRU ),DEVICE_TEMPLATE_PARAM_ADDR );
	if ( ret != NO_ERR )
	{
		return FLASH_R_ERR;
	}

	root = cJSON_Parse ( ( char* ) jsonBuff ); //解析数据包
	if ( !root )
	{
		cJSON_Delete ( root );
		log_d ( "Error before: [%s]\r\n",cJSON_GetErrorPtr() );
		return CJSON_PARSE_ERR;
	}

	data = cJSON_GetObjectItem ( root, "data" );
	if ( data == NULL )
	{
		log_d ( "data NULL\r\n" );
		result = CJSON_GETITEM_ERR;
		cJSON_Delete ( root );
		return result;
	}

	templateData = cJSON_GetObjectItem ( data, "template" );
	if ( templateMap == NULL )
	{
		log_d ( "templateData NULL\r\n" );
		result = CJSON_GETITEM_ERR;
		cJSON_Delete ( root );
		return result;
	}

	templateMap = cJSON_GetObjectItem ( templateData, "templateMap" );
	if ( templateMap == NULL )
	{
		log_d ( "templateMap NULL\r\n" );
		result = CJSON_GETITEM_ERR;
		cJSON_Delete ( root );
		return result;
	}

	holidayTimeMap = cJSON_GetObjectItem ( templateData, "hoildayTimeMap" );
	if ( holidayTimeMap == NULL )
	{
		log_d ( "hoildayTimeMap NULL\r\n" );
		result = CJSON_GETITEM_ERR;
		cJSON_Delete ( root );
		return result;
	}

	peakTimeMap = cJSON_GetObjectItem ( templateData, "peakTimeMap" );
	if ( peakTimeMap ==NULL )
	{
		log_d ( "peakTimeMap NULL\r\n" );
		result = CJSON_GETITEM_ERR;
		cJSON_Delete ( root );
		return result;
	}


//--------------------------------------------------
	//获取templateMap数据
	cJSON* json_item = cJSON_GetObjectItem ( templateMap, "id" );
	templateParam->id = json_item->valueint;
//    sprintf(tmpbuf,"%8d",templateParam->id);
//    ef_set_env_blob("templateID",tmpbuf,8);
	log_d ( "templateParam->id = %d\r\n",templateParam->id );

	json_item = cJSON_GetObjectItem ( templateMap, "templateCode" );
	strcpy ( ( char* ) templateParam->templateCode,json_item->valuestring );
//    ef_set_env_blob("templateCode",templateParam->templateCode,strlen((const char *)templateParam->templateCode));
	log_d ( "templateParam->templateCode = %s\r\n",templateParam->templateCode );

	json_item = cJSON_GetObjectItem ( templateMap, "templateName" );
	strcpy ( ( char* ) templateParam->templateName,json_item->valuestring );
//    ef_set_env_blob("templateName",templateParam->templateName,strlen(templateParam->templateName));
	log_d ( "templateParam->templateName = %s\r\n",templateParam->templateName );

	json_item = cJSON_GetObjectItem ( templateMap, "templateStatus" );
	templateParam->templateStatus = json_item->valueint;
//    memset(tmpbuf,0x00,sizeof(tmpbuf));
//    sprintf(tmpbuf,"%8d",templateParam->templateStatus);
//    ef_set_env_blob("templateStatus",tmpbuf,8);
	log_d ( "templateParam->templateStatus = %d\r\n",templateParam->templateStatus );

	json_item = cJSON_GetObjectItem ( templateMap, "callingWay" );
	strcpy ( ( char* ) templateParam->callingWay,json_item->valuestring );
	//解析模板识别方式
	split ( ( char* ) templateParam->callingWay,",",callingWay,&callingWayNum ); //调用函数进行分割
	while ( callingWayNum )
	{
		switch ( atoi ( callingWay[callingWayNum-1] ) )
		{
			case 1:
				templateParam->templateCallingWay.isFace = 1;
				break;
			case 2:
				templateParam->templateCallingWay.isQrCode = 1;
				break;
			case 3:
				templateParam->templateCallingWay.isIcCard = 1;
				break;
		}
		callingWayNum-- ;
	}
//    ef_set_env_blob("T_callingWay",templateParam->callingWay,strlen((const char*)templateParam->callingWay));
	log_d ( "templateParam->callingWay = %s\r\n",templateParam->callingWay );

	json_item = cJSON_GetObjectItem ( templateMap, "offlineProcessing" );
	templateParam->offlineProcessing = json_item->valueint;
//    memset(tmpbuf,0x00,sizeof(tmpbuf));
//    sprintf(tmpbuf,"%8d",templateParam->offlineProcessing);
//    ef_set_env_blob("offlineStatus",tmpbuf,8);
	log_d ( "templateParam->offlineProcessing = %d\r\n",templateParam->offlineProcessing );


	json_item = cJSON_GetObjectItem ( templateMap, "modeType" );
	strcpy ( ( char* ) templateParam->modeType,json_item->valuestring );

	//解析是否开启节假日模式
	memset ( callingWay,0x00,sizeof ( callingWay ) );
	callingWayNum = 0;
	split ( ( char* ) templateParam->modeType,",",callingWay,&callingWayNum ); //调用函数进行分割
	while ( callingWayNum )
	{
		switch ( atoi ( callingWay[callingWayNum-1] ) )
		{
			case 1:
				templateParam->workMode.isPeakMode = 1;
				break;
			case 2:
				templateParam->workMode.isHolidayMode = 1;
				break;
			case 5:
				templateParam->workMode.isNormalMode = 1;
				break;
		}
		callingWayNum-- ;
	}

//    ef_set_env_blob("modeType",templateParam->modeType,strlen((const char*)templateParam->modeType));
	log_d ( "templateParam->modeType = %s\r\n",templateParam->modeType );

	json_item = cJSON_GetObjectItem ( templateMap, "peakCallingWay" );
	strcpy ( ( char* ) templateParam->peakInfo[0].callingWay,json_item->valuestring );

	//解析识别方式
	memset ( callingWay,0x00,sizeof ( callingWay ) );
	callingWayNum = 0;
	split ( ( char* ) templateParam->peakInfo[0].callingWay,",",callingWay,&callingWayNum ); //调用函数进行分割
	while ( callingWayNum )
	{
		switch ( atoi ( callingWay[callingWayNum-1] ) )
		{
			case 1:
				templateParam->peakCallingWay.isFace = 1;
				break;
			case 2:
				templateParam->peakCallingWay.isQrCode = 1;
				break;
			case 3:
				templateParam->peakCallingWay.isIcCard = 1;
				break;
		}
		callingWayNum-- ;
	}

//    ef_set_env_blob("peakCallingWay",templateParam->peakInfo[0].callingWay,strlen((const char *)templateParam->peakInfo[0].callingWay));
	log_d ( "templateParam->peakInfo[0].callingWay = %s\r\n",templateParam->peakInfo[0].callingWay );

	json_item = cJSON_GetObjectItem ( templateMap, "peakStartDate" );
	strcpy ( ( char* ) templateParam->peakInfo[0].beginTime,json_item->valuestring );
//    ef_set_env_blob("peakStartDate",templateParam->peakInfo[0].beginTime,strlen((const char*)templateParam->peakInfo[0].beginTime));
	log_d ( "templateParam->peakInfo[0].beginTime = %s\r\n",templateParam->peakInfo[0].beginTime );

	json_item = cJSON_GetObjectItem ( templateMap, "peakEndDate" );
	strcpy ( ( char* ) templateParam->peakInfo[0].endTime,json_item->valuestring );
//    ef_set_env_blob("peakEndDate",templateParam->peakInfo[0].endTime,strlen((const char*)templateParam->peakInfo[0].endTime));
	log_d ( "templateParam->peakInfo[0].endTime = %s\r\n",templateParam->peakInfo[0].endTime );

	json_item = cJSON_GetObjectItem ( templateMap, "peakHolidaysType" );
	strcpy ( ( char* ) templateParam->peakInfo[0].outsideTimeMode,json_item->valuestring );
//    ef_set_env_blob("peakHolidaysType",templateParam->peakInfo[0].outsideTimeMode,strlen((const char*)templateParam->peakInfo[0].outsideTimeMode));
	log_d ( "templateParam->peakInfo[0].outsideTimeMode = %s\r\n",templateParam->peakInfo[0].outsideTimeMode );

	json_item = cJSON_GetObjectItem ( templateMap, "peakHolidays" );
	strcpy ( ( char* ) templateParam->peakInfo[0].outsideTimeData,json_item->valuestring );
//    ef_set_env_blob("peakHolidays",templateParam->peakInfo[0].outsideTimeData,strlen((const char*)templateParam->peakInfo[0].outsideTimeData));
	log_d ( "templateParam->peakInfo[0].outsideTimeData = %s\r\n",templateParam->peakInfo[0].outsideTimeData );
//------------------------------------------------------------------------------
	json_item = cJSON_GetObjectItem ( templateMap, "holidayCallingWay" );
	strcpy ( ( char* ) templateParam->hoildayInfo[0].callingWay,json_item->valuestring );
//    ef_set_env_blob("holidayCallingWay",templateParam->hoildayInfo[0].callingWay,strlen((const char*)templateParam->hoildayInfo[0].callingWay));
	log_d ( "templateParam->hoildayInfo[0].callingWay = %s\r\n",templateParam->hoildayInfo[0].callingWay );

	json_item = cJSON_GetObjectItem ( templateMap, "holidayStartDate" );
	strcpy ( ( char* ) templateParam->hoildayInfo[0].beginTime,json_item->valuestring );
//    ef_set_env_blob("holidayStartDate",templateParam->hoildayInfo[0].beginTime,strlen((const char*)templateParam->hoildayInfo[0].beginTime));
	log_d ( "templateParam->hoildayInfo[0].beginTime = %s\r\n",templateParam->hoildayInfo[0].beginTime );

	json_item = cJSON_GetObjectItem ( templateMap, "holidayEndDate" );
	strcpy ( ( char* ) templateParam->hoildayInfo[0].endTime,json_item->valuestring );
//    ef_set_env_blob("holidayEndDate",templateParam->hoildayInfo[0].endTime,strlen((const char*)templateParam->hoildayInfo[0].endTime));
	log_d ( "templateParam->hoildayInfo[0].endTime = %s\r\n",templateParam->hoildayInfo[0].endTime );

	json_item = cJSON_GetObjectItem ( templateMap, "holidayHolidaysType" );
	strcpy ( ( char* ) templateParam->hoildayInfo[0].outsideTimeMode,json_item->valuestring );
//    ef_set_env_blob("holidayHolidaysType",templateParam->hoildayInfo[0].outsideTimeMode,strlen((const char*)templateParam->hoildayInfo[0].outsideTimeMode));
	log_d ( "templateParam->hoildayInfo[0].outsideTimeMode = %s\r\n",templateParam->hoildayInfo[0].outsideTimeMode );

	json_item = cJSON_GetObjectItem ( templateMap, "holidayHolidays" );
	strcpy ( ( char* ) templateParam->hoildayInfo[0].outsideTimeData,json_item->valuestring );
//    ef_set_env_blob("holidayHolidays",templateParam->hoildayInfo[0].outsideTimeData,strlen((const char*)templateParam->hoildayInfo[0].outsideTimeData));
	log_d ( "templateParam->hoildayInfo[0].outsideTimeData = %s\r\n",templateParam->hoildayInfo[0].outsideTimeData );

//--------------------------------------------------
	//存储hoildayTimeMap中数据
	holidayTimeMapCnt = cJSON_GetArraySize ( holidayTimeMap ); /*获取数组长度*/
	log_d ( "array len = %d\r\n",holidayTimeMapCnt );

//    //存储不受控时间段的个数
//    if(holidayTimeMapCnt > 0)
//    {
//        sprintf(tmpKey,"%04d",holidayTimeMapCnt);
//        ef_set_env_blob("holidayTimeMapCnt",tmpKey,4);
//    }

	for ( index=0; index<holidayTimeMapCnt; index++ )
	{
		tmpArray = cJSON_GetArrayItem ( holidayTimeMap, index );

		memset ( tmpKey,0x00,sizeof ( tmpKey ) );
		memset ( tmpIndex,0x00,sizeof ( tmpIndex ) );

		arrayElement = cJSON_GetObjectItem ( tmpArray, "templateType" );
		templateParam->holidayMode[index].templateType = arrayElement->valueint;
		log_d ( "templateType = %d\r\n",templateParam->holidayMode[index].templateType );

		arrayElement = cJSON_GetObjectItem ( tmpArray, "voiceSize" );
		templateParam->holidayMode[index].voiceSize = arrayElement->valueint;
		log_d ( "voiceSize = %d\r\n",templateParam->holidayMode[index].voiceSize );

		arrayElement = cJSON_GetObjectItem ( tmpArray, "modeType" );
		templateParam->holidayMode[index].channelType = arrayElement->valueint;
		log_d ( "modeType = %d\r\n",templateParam->holidayMode[index].channelType );

		arrayElement = cJSON_GetObjectItem ( tmpArray, "startTime" );

		//因为节假日跟高峰共用，所以只记录到FLASH一种就可以了
		strcpy ( ( char* ) templateParam->holidayMode[index].startTime,arrayElement->valuestring );

		sprintf ( tmpIndex,"%d",index );
		strcpy ( tmpKey,"hoildayModeStartTime" );
		strcat ( tmpKey,tmpIndex );
//        ef_set_env_blob(tmpKey,templateParam->holidayMode[index].startTime,strlen((const char*)templateParam->holidayMode[index].startTime));
		log_d ( "%s = %s\r\n",tmpKey,templateParam->holidayMode[index].startTime );


		arrayElement = cJSON_GetObjectItem ( tmpArray, "endTime" );
		strcpy ( ( char* ) templateParam->holidayMode[index].endTime,arrayElement->valuestring );

		memset ( tmpKey,0x00,sizeof ( tmpKey ) );
		strcpy ( tmpKey,"hoildayModeEndTime" );
		strcat ( tmpKey,tmpIndex );
//        ef_set_env_blob(tmpKey,templateParam->holidayMode[index].endTime,strlen((const char*)templateParam->holidayMode[index].endTime));
		log_d ( "%s= %s\r\n",tmpKey,templateParam->holidayMode[index].endTime );
	}

	log_d ( "=====================================================\r\n" );
//--------------------------------------------------
	peakTimeMapCnt = cJSON_GetArraySize ( peakTimeMap ); /*获取数组长度*/
	log_d ( "peakTimeMapCnt len = %d\r\n",peakTimeMapCnt );

	for ( index=0; index<peakTimeMapCnt; index++ )
	{
		tmpArray = cJSON_GetArrayItem ( peakTimeMap, index );

		memset ( tmpKey,0x00,sizeof ( tmpKey ) );
		memset ( tmpIndex,0x00,sizeof ( tmpIndex ) );

		arrayElement = cJSON_GetObjectItem ( tmpArray, "templateType" );
		templateParam->peakMode[index].templateType = arrayElement->valueint;
		log_d ( "templateType = %d\r\n",templateParam->holidayMode[index].templateType );

		arrayElement = cJSON_GetObjectItem ( tmpArray, "voiceSize" );
		templateParam->peakMode[index].voiceSize = arrayElement->valueint;
		log_d ( "voiceSize = %d\r\n",templateParam->holidayMode[index].voiceSize );

		arrayElement = cJSON_GetObjectItem ( tmpArray, "modeType" );
		templateParam->peakMode[index].channelType = arrayElement->valueint;
		log_d ( "modeType = %d\r\n",templateParam->holidayMode[index].channelType );

		arrayElement = cJSON_GetObjectItem ( tmpArray, "startTime" );

		//因为节假日跟高峰共用，所以只记录到FLASH一种就可以了
		strcpy ( ( char* ) templateParam->peakMode[index].startTime,arrayElement->valuestring );
		sprintf ( tmpIndex,"%d",index );
		strcpy ( tmpKey,"hoildayModeStartTime" );
		strcat ( tmpKey,tmpIndex );
		log_d ( "%s = %s\r\n",tmpKey,templateParam->peakMode[index].startTime );


		arrayElement = cJSON_GetObjectItem ( tmpArray, "endTime" );
		strcpy ( ( char* ) templateParam->peakMode[index].endTime,arrayElement->valuestring );

		memset ( tmpKey,0x00,sizeof ( tmpKey ) );
		strcpy ( tmpKey,"hoildayModeEndTime" );
		strcat ( tmpKey,tmpIndex );
		log_d ( "%s= %s\r\n",tmpKey,templateParam->peakMode[index].endTime );

	}

	cJSON_Delete ( root );

	gtemplateParam.initFlag.iFlag = DEFAULT_INIVAL;

	ret = optTemplateParam ( templateParam,WRITE_PRARM,sizeof ( TEMPLATE_PARAM_STRU ),DEVICE_TEMPLATE_PARAM_ADDR );
	log_d ( "saveTemplateParam took %d ms to save\r\n",xTaskGetTickCount()-curtick );

	if ( ret != NO_ERR )
	{
		return FLASH_W_ERR;
	}

	return result;
}

void initTemplateParam ( void )
{
	uint8_t ret = 0;

	ClearTemplateParam();
	ret = opParam ( &gtemplateParam,READ_PRARM,sizeof ( TEMPLATE_PARAM_STRU ),DEVICE_TEMPLATE_PARAM_ADDR );

	if ( ret == false )
	{
		log_e ( "read param error\r\n" );
		return;
	}


	if ( gtemplateParam.initFlag.iFlag != DEFAULT_INIVAL )
	{
		//模板数据赋值
		gtemplateParam.id = 1;
		gtemplateParam.templateStatus = 1;
		gtemplateParam.offlineProcessing = 2;

		strcpy ( ( char* ) gtemplateParam.modeType,"5" );
		strcpy ( ( char* ) gtemplateParam.templateCode,"100000" );
		strcpy ( ( char* ) gtemplateParam.templateName,DEFAULT_DEV_NAME );
		strcpy ( ( char* ) gtemplateParam.callingWay,"1,2,3" );

		gtemplateParam.peakCallingWay.isIcCard = 1;
		gtemplateParam.peakCallingWay.isFace  = 1;
		gtemplateParam.peakCallingWay.isQrCode  = 1;

		log_d ( "-----------\r\n" );

		ret = opParam ( &gtemplateParam,WRITE_PRARM,sizeof ( TEMPLATE_PARAM_STRU ),DEVICE_TEMPLATE_PARAM_ADDR );
		if ( ret == false )
		{
			log_e ( "read param error\r\n" );
			return;
		}
	}
	log_d ( "init param success\r\n" );
}

static uint8_t opParam ( void* Param,uint8_t mode,uint32_t len,uint32_t addr )
{
	uint8_t ret = 0;

	char* buff = mymalloc ( SRAMIN,len );

	if ( buff == NULL )
	{
		log_e ( "malloc memery error\r\n" );
		myfree ( SRAMIN,buff );
		return false;
	}

	if ( mode == WRITE_PRARM )
	{
		memcpy ( buff,Param,len );

		ret = FRAM_Write ( FM24V10_1, addr, buff,len );
		if ( ret == 0 )
		{
			log_e ( "write template param error\r\n" );
			myfree ( SRAMIN,buff );
			return ret;
		}

		log_d ( "write param success\r\n" );
	}
	else
	{
		ret = FRAM_Read ( FM24V10_1, addr, buff, len );
		if ( ret != 1 )
		{
			log_e ( "read param error\r\n" );
			myfree ( SRAMIN,buff );
			return ret ;
		}

		memcpy ( Param, buff,  len );

		log_d ( "read param success\r\n" );
	}

	myfree ( SRAMIN,buff );

	return ret;
}


void ClearTemplateParam ( void )
{
	memset ( &gtemplateParam,'\0',sizeof ( TEMPLATE_PARAM_STRU ) );

	log_d ( "TEMPLATE_PARAM_STRU = %d\r\n",sizeof ( TEMPLATE_PARAM_STRU ) );
}

uint8_t optTemplateParam ( void* stParam,uint8_t mode,uint32_t len,uint32_t addr )
{
	return opParam ( stParam,mode,len,addr );
}



//uint8_t optParam(void *stParam,uint8_t mode,uint32_t len,uint32_t addr,CallBackParam callBack)
//{
//    return callBack(stParam,mode,len,addr);
//}

void ClearDevBaseParam ( void )
{
	memset ( &gDevBaseParam,'\0',sizeof ( DEV_BASE_PARAM_STRU ) );
}


void initDevBaseParam ( void )
{
	uint8_t ret = 0;
	char mac[6+1] = {0};
	char temp[20] = {0};
	char asc[12+1] = {0};

	ClearDevBaseParam();

	ret = opParam ( &gDevBaseParam,READ_PRARM,sizeof ( DEV_BASE_PARAM_STRU ),DEVICE_BASE_PARAM_ADDR );

	if ( ret == false )
	{
		log_e ( "read param error\r\n" );
		return;
	}

	if ( gDevBaseParam.deviceCode.downLoadFlag.iFlag != DEFAULT_BASE_INIVAL )
	{
		log_d ( "wirte default base data\r\n" );
		ClearDevBaseParam();

		//设备状态为启用状态
		gDevBaseParam.deviceState.iFlag = DEVICE_ENABLE;

		calcMac ( ( unsigned char* ) mac );
		bcd2asc ( ( unsigned char* ) asc, ( unsigned char* ) mac, 12, 0 );
		Insertchar ( asc,temp,':' );
		gDevBaseParam.deviceCode.deviceSnLen = strlen ( temp )-1 ;
		memcpy ( gDevBaseParam.deviceCode.deviceSn,temp,gDevBaseParam.deviceCode.deviceSnLen );
		strcpy ( gDevBaseParam.mqttTopic.publish,DEV_FACTORY_PUBLISH );
		strcpy ( gDevBaseParam.mqttTopic.subscribe,DEV_FACTORY_SUBSCRIBE );
		strncat ( gDevBaseParam.mqttTopic.subscribe,gDevBaseParam.deviceCode.deviceSn,gDevBaseParam.deviceCode.deviceSnLen );
		memcpy ( gDevBaseParam.deviceCode.qrSn,asc,8 ); //使用前8位做为本机的QRID

		log_d ( "gDevBaseParam.deviceCode.deviceSn = %s,len = %d\r\n",gDevBaseParam.deviceCode.deviceSn,gDevBaseParam.deviceCode.deviceSnLen );
		log_d ( "gDevBaseParam.mqttTopic.publish = %s\r\n",gDevBaseParam.mqttTopic.publish );
		log_d ( "gDevBaseParam.mqttTopic.subscribe = %s\r\n",gDevBaseParam.mqttTopic.subscribe );
		log_d ( "gDevBaseParam.deviceCode.qrSn = %s\r\n",gDevBaseParam.deviceCode.qrSn );



		log_d ( "wirte default base data\r\n" );

		ret = opParam ( &gDevBaseParam,WRITE_PRARM,sizeof ( DEV_BASE_PARAM_STRU ),DEVICE_BASE_PARAM_ADDR );

		if ( ret == false )
		{
			log_e ( "read param error\r\n" );
			return;
		}
	}

	log_d ( "init param success\r\n" );
}


uint8_t optDevBaseParam ( void* stParam,uint8_t mode,uint32_t len,uint32_t addr )
{
	return opParam ( stParam,mode,len,addr );
}


void clearTemplateFRAM ( void )
{
	FRAM_Erase ( FM24V10_1,RECORD_INDEX_ADDR,2176 );
}


void initRecordIndex ( void )
{
	uint8_t ret = 0;

	ClearRecordIndex();
	ret = opParam ( &gRecordIndex,READ_PRARM,sizeof ( RECORDINDEX_STRU ),RECORD_INDEX_ADDR );

	if ( ret == false )
	{
		log_e ( "read param error\r\n" );
		return;
	}

	log_d ( "gCurCardHeaderIndex = %d\r\n",gRecordIndex.cardNoIndex );
	log_d ( "gCurUserHeaderIndex = %d\r\n",gRecordIndex.userIdIndex );
	log_d ( "gDelCardHeaderIndex = %d\r\n",gRecordIndex.delCardNoIndex );
	log_d ( "gDelUserHeaderIndex = %d\r\n",gRecordIndex.delUserIdIndex );


	log_d ( "init param success\r\n" );
}
void ClearRecordIndex ( void )
{
	memset ( &gRecordIndex,0x00,sizeof ( RECORDINDEX_STRU ) );
}
uint8_t optRecordIndex ( RECORDINDEX_STRU* recoIndex,uint8_t mode )
{
	uint8_t ret = 0;

	char* buff = mymalloc ( SRAMIN,sizeof ( RECORDINDEX_STRU ) );

	if ( buff == NULL )
	{
		log_e ( "malloc memery error\r\n" );
		myfree ( SRAMIN,buff );
		return false;
	}

	if ( mode == WRITE_PRARM )
	{
		memcpy ( buff,recoIndex,sizeof ( RECORDINDEX_STRU ) );

		log_d ( "write index %d,%d,%d,%d\r\n",recoIndex->cardNoIndex,recoIndex->delCardNoIndex,recoIndex->userIdIndex,recoIndex->delUserIdIndex );

		ret = FRAM_Write ( FM24V10_1, RECORD_INDEX_ADDR, buff,sizeof ( RECORDINDEX_STRU ) );
		if ( ret == 0 )
		{
			log_e ( "write template param error\r\n" );
			myfree ( SRAMIN,buff );
			return ret;
		}

		log_d ( "write param success\r\n" );
	}
	else
	{
		ret = FRAM_Read ( FM24V10_1, RECORD_INDEX_ADDR, buff, sizeof ( RECORDINDEX_STRU ) );
		if ( ret != 1 )
		{
			log_e ( "read param error\r\n" );
			myfree ( SRAMIN,buff );
			return ret ;
		}

		memcpy ( recoIndex, buff,  sizeof ( RECORDINDEX_STRU ) );

		log_d ( "read param success\r\n" );
	}

	myfree ( SRAMIN,buff );

	return ret;
}


