/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : LocalData.c
  版 本 号   : 初稿
  作    者   :
  生成日期   : 2020年3月21日
  最近修改   :
  功能描述   : 管理本地数据，对卡号，用户ID为索引的数-
                   据进行增删改查
  函数列表   :
  修改历史   :
  1.日    期   : 2020年3月21日
    作    者   :
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "LocalData.h"
#include "bsp_spi_flash.h"
#include "easyflash.h"
#include "stdio.h"
#include "tool.h"
#include "malloc.h"
#include "bsp_MB85RC128.h"
#include "deviceInfo.h"
#include "quickSort.h"



#define LOG_TAG    "localData"
#include "elog.h"


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
// *----------------------------------------------*/
//volatile uint16_t gCurCardHeaderIndex = 0;
//volatile uint16_t gCurUserHeaderIndex = 0;
//volatile uint16_t gCurRecordIndex = 0;
//volatile uint16_t gDelCardHeaderIndex = 0;    //已删除卡号索引
//volatile uint16_t gDelUserHeaderIndex = 0;    //已删除用户ID索引

//USERDATA_STRU gUserDataStru = {0};

HEADINFO_STRU gSectorBuff[512] = {0};

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static uint8_t checkFlashSpace ( uint8_t mode );
static void eraseUserDataIndex ( void );
static ISFIND_ENUM findIndex ( uint8_t* header,uint32_t address,uint16_t curIndex,uint16_t* index );
static uint32_t readDelIndexValue ( uint8_t mode,uint16_t curIndex );

static uint8_t delSourceHeader ( uint16_t index,uint8_t mode );
static ISFIND_ENUM fIndex( uint8_t* header,uint8_t mode,uint16_t* index );

typedef ISFIND_ENUM(*opFindIndex)(uint8_t* header,uint8_t mode,uint16_t* index); //获取index回调

static int Bin_Search(HEADINFO_STRU *num,int numsSize,int target);
static int Bin_Search_addr(uint32_t startAddr,int numsSize,int target);






uint32_t readDelIndexValue ( uint8_t mode,uint16_t curIndex )
{
	uint8_t readBuff[HEAD_lEN+1] = {0};
	uint8_t temp[HEAD_lEN*2+1] = {0};
	uint32_t addr = 0;
	uint32_t value = 0;

	memset ( readBuff,0x00,sizeof ( readBuff ) );
	//写入已删除的空间中,根据已删除索引，获取到当前索引下卡或者用户的索引

	if ( mode == CARD_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR + curIndex * HEAD_lEN;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_DEL_HEAD_ADDR + curIndex * HEAD_lEN;
	}

	FRAM_Read ( FM24V10_1, addr, readBuff, HEAD_lEN );


	bcd2asc ( temp, readBuff,CARD_NO_LEN_ASC, 0 );

	value = atoi ( ( const char* ) temp );

	//log_d("readDelIndexValue = %s,value =%d\r\n",temp,value);

	return value;

}


void eraseUserDataAll ( void )
{
	int32_t iTime1, iTime2;
	iTime1 = xTaskGetTickCount();	/* 记下开始时间 */
	eraseHeadSector();
	eraseDataSector();
	clearTemplateFRAM();
    initTemplateParam();	
	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d ( "eraseUserDataAll成功，耗时: %dms\r\n",iTime2 - iTime1 );
}

static void eraseUserDataIndex ( void )
{
    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,WRITE_PRARM);
}


void eraseHeadSector ( void )
{
	FRAM_Erase ( FM24V10_1,0,122880 );	
}
void eraseDataSector ( void )
{
	uint16_t i = 0;

	for ( i=0; i<DATA_SECTOR_NUM; i++ )
	{
		bsp_sf_EraseSector ( CARD_NO_DATA_ADDR+i*SECTOR_SIZE );
		bsp_sf_EraseSector ( USER_ID_DATA_ADDR+i*SECTOR_SIZE );
	}
}


uint8_t writeUserData ( USERDATA_STRU *userData,uint8_t mode )
{
	uint8_t wBuff[255] = {0};
	uint8_t rBuff[255] = {0};
	uint8_t isFull = 0;
	uint8_t crc=0;
	uint8_t ret = 0;
	uint8_t times = 3;
	uint32_t addr = 0;
	uint32_t index = 0;
    uint8_t head[8] = {0};
    int tmpIndex = 0;
	HEADINFO_STRU tmpHead;

	int32_t iTime1, iTime2;

	iTime1 = xTaskGetTickCount();	/* 记下开始时间 */

	//检查存储空间是否已满；
	isFull = checkFlashSpace ( mode );

	if ( isFull == 1 )
	{
		return 1; //提示已经满了
	}

	if ( mode == CARD_MODE )
	{
	    asc2bcd ( head, userData->cardNo,CARD_NO_LEN_ASC, 1 );	
	}
	else
	{
	    asc2bcd ( head, userData->userId,CARD_NO_LEN_ASC, 1 );		
	}	

    memcpy(tmpHead.headData.sn,head,CARD_NO_LEN_BCD);
    tmpIndex = readHead(&tmpHead,mode);

    //卡号或者是USERID已存在
    if(tmpIndex != NO_FIND_HEAD)
    {
        return 4;
    }

    //写表头
	addHead(head,mode);

    //读当前FLASH的索引
	ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);

	//获取地址
	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_DATA_ADDR + (gRecordIndex.cardNoIndex-1) * ( sizeof ( USERDATA_STRU ) );;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_DATA_ADDR + (gRecordIndex.userIdIndex-1) * ( sizeof ( USERDATA_STRU ) );
	}

	//packet write buff
	memset ( wBuff,0x00,sizeof ( wBuff ) );

	//copy to buff
	memcpy ( wBuff, userData, sizeof ( USERDATA_STRU )-1 );


	//这里应该打印一下WBUFF，看数据是否正确

	//calc crc
	crc = xorCRC ( wBuff, sizeof ( USERDATA_STRU )-1 );

	//copy crc
	wBuff[sizeof ( USERDATA_STRU )-1] = crc;


	//write flash
	while ( times )
	{

		bsp_sf_WriteBuffer ( wBuff, addr, sizeof ( USERDATA_STRU ) );

		//再读出来，对比是否一致
		memset ( rBuff,0x00,sizeof ( rBuff ) );
		bsp_sf_ReadBuffer ( rBuff, addr, sizeof ( USERDATA_STRU ) );
		ret = compareArray ( wBuff,rBuff,sizeof ( USERDATA_STRU ) );

		if ( ret == 0 )
		{
			break;
		}


		if ( ret != 0 && times == 1 )
		{
			//log_d("写交易记录表失败!\r\n");
			return 3;
		}

		times--;
	}

	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_e ( "writeUserData成功，耗时: %dms\r\n",iTime2 - iTime1 );

//    dbh("writeUserData", wBuff, sizeof(USERDATA_STRU));

	return 0;
}


//add 2020.12.24 查找卡号，若有，修直接覆盖当前卡号对应的FLASH区域，若无，则新建
int modifyCardData(USERDATA_STRU *userData)
{
	uint8_t wBuff[255] = {0};
	uint8_t rBuff[255] = {0};
	uint8_t isFull = 0;
	uint8_t crc=0;
	uint8_t ret = 0;
	uint8_t times = 3;
	uint32_t addr = CARD_NO_DATA_ADDR;	
	uint32_t index = 0;
    uint8_t head[4] = {0};
    int tmpIndex = 0;
    uint32_t flashIndex = 0;
	HEADINFO_STRU tmpHead;
    
	int32_t iTime1, iTime2;

	iTime1 = xTaskGetTickCount();	/* 记下开始时间 */

    asc2bcd ( head, userData->cardNo,CARD_NO_LEN_ASC, 1 );	
    
    memcpy(tmpHead.headData.sn,head,CARD_NO_LEN_BCD);
    
    tmpIndex = readHead(&tmpHead,CARD_MODE);

    //卡号已存在,直接改写相应的FLASH内容
    if(tmpIndex != NO_FIND_HEAD)
    {
        //覆写FLASH
        addr = CARD_NO_HEAD_ADDR;
        addr += tmpIndex  * sizeof(HEADINFO_STRU);
        
        tmpHead.headData.id = 0;    
        tmpHead.flashAddr = 0;
        ret = FRAM_Read (FM24V10_1, addr, &tmpHead,sizeof(HEADINFO_STRU)); 
    	if(ret == 0)
        {
            log_e("read fram error\r\n");
            return NO_FIND_HEAD; 
        }  
    	//读取FLASH索引
        flashIndex = tmpHead.flashAddr;	  

        log_d("delHead head =%x,delHead flashIndex = %d\r\n",tmpHead.headData.id,tmpHead.flashAddr); 
        
        addr = CARD_NO_DATA_ADDR + flashIndex * ( sizeof ( USERDATA_STRU ) );

        bsp_sf_ReadBuffer ( rBuff, addr, sizeof ( USERDATA_STRU ) );

//        dbh("modifyCardData", rBuff, sizeof(USERDATA_STRU));  
    }
    else
    {
        //添新加卡号，并写FLASH
        //写表头
    	addHead(head,CARD_MODE);

        //读当前FLASH的索引
    	ClearRecordIndex();
        optRecordIndex(&gRecordIndex,READ_PRARM);
        
    	addr = CARD_NO_DATA_ADDR + (gRecordIndex.cardNoIndex-1) * ( sizeof ( USERDATA_STRU ) );
    }

	//packet write buff
	memset ( wBuff,0x00,sizeof ( wBuff ) );

	//copy to buff
	memcpy ( wBuff, userData, sizeof ( USERDATA_STRU )-1 );

	//calc crc
	crc = xorCRC ( wBuff, sizeof ( USERDATA_STRU )-1 );

	//copy crc
	wBuff[sizeof ( USERDATA_STRU )-1] = crc;


	//write flash
	while ( times )
	{

		bsp_sf_WriteBuffer ( wBuff, addr, sizeof ( USERDATA_STRU ) );

		//再读出来，对比是否一致
		memset ( rBuff,0x00,sizeof ( rBuff ) );
		bsp_sf_ReadBuffer ( rBuff, addr, sizeof ( USERDATA_STRU ) );
		ret = compareArray ( wBuff,rBuff,sizeof ( USERDATA_STRU ) );

		if ( ret == 0 )
		{
			break;
		}


		if ( ret != 0 && times == 1 )
		{
			//log_d("写交易记录表失败!\r\n");
			return 3;
		}

		times--;
	}

	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_e ( "writeUserData成功，耗时: %dms\r\n",iTime2 - iTime1 );

//    dbh("writeUserData", wBuff, sizeof(USERDATA_STRU));    

	return 0;
}



uint8_t readUserData ( uint8_t* header,uint8_t mode,USERDATA_STRU* userData )
{
	uint8_t rBuff[255] = {0};
	uint8_t crc = 0;
	uint8_t ret = 0;
	int index = 0;
	uint32_t addr = 0;
	HEADINFO_STRU head;
	uint32_t flashIndex = 0;

    int32_t iTime1, iTime2;

	if ( header == NULL )
	{
		//log_d("invalid head\r\n");
		return 1; //提示错误的参数
	}
	
	asc2bcd (head.headData.sn, header,CARD_NO_LEN_ASC, 1 );
	
	
    iTime1 = xTaskGetTickCount();	/* 记下开始时间 */

    log_d("1 want find head = %x\r\n",head.headData.id);

    //这里是head的序列号，要想知道FLASH的索引，还需要读取
	index = readHead(&head,mode);
	
    iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d("readUserData成功，耗时: %dms\r\n",iTime2 - iTime1);	

	if(index == NO_FIND_HEAD)
	{
        log_d("no find head\r\n");
        return 2;
	}

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_HEAD_ADDR;
	}
	
    head.flashAddr = 0;
    head.headData.id = 0;	
    ret = FRAM_Read (FM24V10_1, addr+index*CARD_USER_LEN, &head,CARD_USER_LEN);    
    if(ret == 0)
    {
        log_e("read fram error\r\n");
        return NO_FIND_HEAD; 
    }

    flashIndex = head.flashAddr;

    log_d("readUserData head = %x,readUserData flashIndex = %d\r\n",head.headData.id,flashIndex);

	if ( mode == CARD_MODE )
	{
        addr = CARD_NO_DATA_ADDR + flashIndex * ( sizeof ( USERDATA_STRU ) );
	}
	else if(mode == USER_MODE)
	{
		addr = USER_ID_DATA_ADDR + flashIndex * ( sizeof ( USERDATA_STRU ) );
	}		

	bsp_sf_ReadBuffer ( rBuff, addr, sizeof ( USERDATA_STRU ) );

	//judge head
	if ( rBuff[0] != TABLE_HEAD )
	{
		//log_d("invalid start char\r\n");
		return 3;
	}

	//calc crc
	crc = xorCRC ( rBuff, sizeof ( USERDATA_STRU )-1 );

	//judge crc
	if ( crc != rBuff[sizeof ( USERDATA_STRU ) - 1] )
	{
		//log_d("crc check error\r\n");
		return 4;
	}

	memcpy ( userData, rBuff, sizeof ( USERDATA_STRU ) );



//    iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
//	log_d("readUserData成功，耗时: %dms\r\n",iTime2 - iTime1);

	//dbh("readUserData", (char *)rBuff, sizeof(USERDATA_STRU));

	return 0;

}


static uint8_t checkFlashSpace ( uint8_t mode )
{
	if ( mode == CARD_MODE )
	{
		if ( gRecordIndex.cardNoIndex > MAX_HEAD_RECORD-5 )
		{
			//log_d("card flash is full\r\n");

			return 1;
		}
	}
	else
	{
		if ( gRecordIndex.userIdIndex > MAX_HEAD_RECORD-5 )
		{
			//log_d("card flash is full\r\n");
			return 1;
		}
	}


	return 0;
}





uint8_t delUserData ( uint8_t* header,uint8_t mode )
{
	uint8_t wBuff[255] = {0};
	uint8_t rBuff[255] = {0};
	uint8_t isFull = 0;
	uint8_t ret = 0;
	uint8_t times = 3;
	uint32_t addr = 0;
	uint16_t index = 0;
    uint8_t headBcd[4] = 0;
    int flashIndex = 0;
	uint8_t multiple = 0;
	uint16_t remainder = 0;  
	uint32_t address = 0;

	int32_t iTime1, iTime2;
	//log_d("sizeof(USERDATA_STRU) = %d\r\n",sizeof(USERDATA_STRU));

	iTime1 = xTaskGetTickCount();   /* 记下开始时间 */

    asc2bcd (headBcd, header,CARD_NO_LEN_ASC, 1 );

	//读取FLASH索引
    flashIndex = delHead(headBcd,mode);	

	if ( flashIndex == NO_FIND_HEAD)
	{
		return 6;//提示未找到索引
	}	
	
	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_DATA_ADDR + flashIndex * ( sizeof ( USERDATA_STRU ) );
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_DATA_ADDR + flashIndex * ( sizeof ( USERDATA_STRU ) );
	}

	//packet write buff
	memset ( wBuff,0x00,sizeof ( wBuff ) );

	//write flash
	while ( times )
	{
		bsp_sf_WriteBuffer ( wBuff, addr, sizeof ( USERDATA_STRU ) );

		//再读出来，对比是否一致
		memset ( rBuff,0x00,sizeof ( rBuff ) );
		bsp_sf_ReadBuffer ( rBuff, addr, sizeof ( USERDATA_STRU ) );

		ret = compareArray ( wBuff,rBuff,sizeof ( USERDATA_STRU ) );

		if ( ret == 0 )
		{
			break;
		}


		if ( ret != 0 && times == 1 )
		{
			//log_d("modify record is error\r\n");
			return 3;
		}

		times--;
	}
	
	iTime2 = xTaskGetTickCount();   /* 记下结束时间 */
	log_d ( "修改记录成功，耗时: %dms\r\n",iTime2 - iTime1 );

	return 0;
}

int delHead(uint8_t *headBuff,uint8_t mode)
{
	int ret = 0;
	uint8_t multiple = 0;
	uint16_t remainder = 0;
    HEADINFO_STRU tmpCard;
    uint32_t addr = 0;    
    int num = 0;
    uint32_t flashIndex = 0;

    if ( headBuff == NULL )
	{
		return NO_FIND_HEAD;
	}	
	
	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_HEAD_ADDR;
	}
	else if ( mode == CARD_DEL_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR;
	}
	else if ( mode == USER_DEL_HEAD_ADDR )
	{
		addr = USER_DEL_HEAD_ADDR;
	}	

    memcpy(tmpCard.headData.sn,headBuff,sizeof(tmpCard.headData.sn));

    //1.查找要删除的对的索引
    ret = readHead(&tmpCard,mode);

    if(ret == NO_FIND_HEAD)
    {
        return NO_FIND_HEAD;
    }

    //2.计算要删除卡号的地址
    multiple = ret / HEAD_NUM_SECTOR;
	remainder = ret % HEAD_NUM_SECTOR;	

	log_d("multiple = %d,remainder=%d\r\n",multiple,remainder);
	
    addr += ret  * sizeof(HEADINFO_STRU);
    
    tmpCard.headData.id = 0;    
    tmpCard.flashAddr = 0;
    ret = FRAM_Read (FM24V10_1, addr, &tmpCard,sizeof(HEADINFO_STRU)); 
	if(ret == 0)
    {
        log_e("read fram error\r\n");
        return NO_FIND_HEAD; 
    }  
	//读取FLASH索引
    flashIndex = tmpCard.flashAddr;	  

    log_d("delHead head =%x,delHead flashIndex = %d\r\n",tmpCard.headData.id,tmpCard.flashAddr);

    //3.将要删除的头置全0    
    tmpCard.headData.id = 0;    
    tmpCard.flashAddr = 0;
    ret = FRAM_Write ( FM24V10_1, addr, &tmpCard,1* sizeof(HEADINFO_STRU));    
    if(ret == 0)
    {
        log_e("write fram error\r\n");
        return NO_FIND_HEAD;
    } 


	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
		
        ClearRecordIndex();
        optRecordIndex(&gRecordIndex,READ_PRARM);        
        num = gRecordIndex.cardNoIndex % HEAD_NUM_SECTOR;		
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_HEAD_ADDR;
        ClearRecordIndex();
        optRecordIndex(&gRecordIndex,READ_PRARM);        
        num = gRecordIndex.userIdIndex % HEAD_NUM_SECTOR;		
	}
    	
    //4.对这一页重新排序
    if(multiple >= 1)
    {
        addr += (multiple-1) * HEAD_NUM_SECTOR  * sizeof(HEADINFO_STRU);
        num = HEAD_NUM_SECTOR;
    }

    memset(gSectorBuff,0x00,sizeof(gSectorBuff));  
    log_d("del add = %x,num = %d\r\n",addr,num);
    
    //读一页数据
    ret = FRAM_Read (FM24V10_1, addr, gSectorBuff, num* sizeof(HEADINFO_STRU));
    if(ret == 0)
    {
        log_e("read fram error\r\n");
        return NO_FIND_HEAD;
    }
    
//    for(int i=0;i<num;i++)
//    {
//        log_d("befare head id =%x,flashAddr = %d\r\n",gSectorBuff[i].headData.id,gSectorBuff[i].flashAddr);
//    }  
    
    
    //排序
    //qSortCard(gSectorBuff,num);  
    sortHead(gSectorBuff,num);  
    //写回数据
    ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,num* sizeof(HEADINFO_STRU));
    if(ret == 0)
    {
        log_e("write fram error\r\n");
        return NO_FIND_HEAD;
    }      

//    for(int i=0;i<num;i++)
//    {
//        log_d("after head id =%x,flashAddr = %d\r\n",gSectorBuff[i].headData.id,gSectorBuff[i].flashAddr);
//    }  


    log_d("qSortCard success\r\n");
    

    
    return flashIndex;
  
}



uint8_t writeDelHeader ( uint8_t* header,uint8_t mode )
{
	uint8_t times = 3;
	uint8_t readBuff[HEAD_lEN+1] = {0};
	uint8_t ret = 0;
	uint8_t headCnt[CARD_USER_LEN] = {0};
	uint8_t temp[HEAD_lEN+1] = {0};
	uint32_t addr = 0;

	if ( mode == CARD_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR + gRecordIndex.delCardNoIndex * HEAD_lEN;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_DEL_HEAD_ADDR + gRecordIndex.delUserIdIndex * HEAD_lEN;
	}

	if ( header == NULL || strlen ( ( const char* ) header ) == 0 )
	{
		//log_d("<<<<<<<<<<<<<<cardNoHeader is empty>>>>>>>>>>>>>>\r\n");
		return 1;
	}

	log_d ( "<<<<<<<<<<<<<<write del Header = %s,addr = %d>>>>>>>>>>>>>>\r\n",header,addr );

	memset ( temp,0x00,sizeof ( temp ) );
	asc2bcd ( temp, header,CARD_NO_LEN_ASC, 0 );

	while ( times )
	{
//		ret = bsp_sf_WriteBuffer (temp, addr, HEAD_lEN);
		ret = FRAM_Write ( FM24V10_1, addr, temp,HEAD_lEN );

		//再读出来，对比是否一致
		memset ( readBuff,0x00,sizeof ( readBuff ) );
//		bsp_sf_ReadBuffer (readBuff, addr, HEAD_lEN);
		FRAM_Read ( FM24V10_1, addr, readBuff, HEAD_lEN );

		ret = compareArray ( temp,readBuff,HEAD_lEN );

		if ( ret == 0 )
		{
			break;
		}

		if ( ret != 0 && times == 1 )
		{
			//log_d("write header error\r\n");
			return 3;
		}

		times--;
	}

	//这里需要保存
//	if ( mode == CARD_MODE )
//	{
//		gRecordIndex.delCardNoIndex++;
//		memset ( headCnt,0x00,sizeof ( headCnt ) );
//		sprintf ( ( char* ) headCnt,"%08d",gRecordIndex.delCardNoIndex );
//		ef_set_env_blob ( "DelCardHeaderIndex",headCnt,CARD_USER_LEN );

//	}
//	else
//	{
//		gRecordIndex.delUserIdIndex++;
//		memset ( headCnt,0x00,sizeof ( headCnt ) );
//		sprintf ( ( char* ) headCnt,"%08d",gRecordIndex.delUserIdIndex );
//		ef_set_env_blob ( "DelUserHeaderIndex",headCnt,CARD_USER_LEN );
//	}

//	log_d ( "gRecordIndex.delCardNoIndex = %d,gRecordIndex.delUserIdIndex = %d\r\n",gRecordIndex.delCardNoIndex,gRecordIndex.delUserIdIndex );


	return 0;

}

static uint8_t delSourceHeader ( uint16_t index,uint8_t mode )
{
	uint8_t times = 3;
	uint8_t readBuff[HEAD_lEN+1] = {0};
	uint8_t ret = 0;
	uint8_t temp[HEAD_lEN+1] = {0};
	uint32_t addr = 0;

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR + index * HEAD_lEN;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_HEAD_ADDR + index * HEAD_lEN;
	}

	memset ( temp,0xFF,sizeof ( temp ) );

	while ( times )
	{
//        ret = bsp_sf_WriteBuffer (temp, addr, HEAD_lEN);
		ret = FRAM_Write ( FM24V10_1, addr, temp,HEAD_lEN );

		//再读出来，对比是否一致
		memset ( readBuff,0x00,sizeof ( readBuff ) );
//        bsp_sf_ReadBuffer (readBuff, addr, HEAD_lEN);
		FRAM_Read ( FM24V10_1, addr, readBuff, HEAD_lEN );

		ret = compareArray ( temp,readBuff,HEAD_lEN );

		if ( ret == 0 )
		{
			break;
		}

		if ( ret != 0 && times == 1 )
		{
			log_d ( "write header error\r\n" );
			return 3;
		}

		times--;
	}

	return 0;

}




void TestFlash ( uint8_t mode )
{
	char buff[156] = {0};
    HEADINFO_STRU tmp;
	uint32_t addr = 0;
	uint32_t data_addr = 0;
	uint16_t i = 0;
	uint32_t num = 0;

	if ( buff == NULL )
	{
		//log_d("my_malloc error\r\n");
		return ;
	}
	
	ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);
    


	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
		data_addr = CARD_NO_DATA_ADDR;
		num = gRecordIndex.cardNoIndex;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_HEAD_ADDR;
		data_addr = USER_ID_DATA_ADDR;
		num = gRecordIndex.userIdIndex;
	}
	else if ( mode == CARD_DEL_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR;
		num = gRecordIndex.delCardNoIndex;
	}
	else if ( mode == USER_DEL_MODE )
	{
		addr = USER_DEL_HEAD_ADDR;
		num = gRecordIndex.delUserIdIndex;
	}

	for ( i=0; i<num; i++ )
	{
        memset ( &tmp,0x00,sizeof ( HEADINFO_STRU ) );
		memset ( buff,0x00,sizeof ( buff ) );
		
		FRAM_Read ( FM24V10_1, addr+i*sizeof ( HEADINFO_STRU ), &tmp, sizeof ( HEADINFO_STRU ) );
		log_d("the %d card id = %x,the flash addr = %d\r\n",i,tmp.headData.id,tmp.flashAddr);

	}

	for ( i=0; i<num; i++ )
	{
		memset ( buff,0x00,sizeof ( buff ) );
		bsp_sf_ReadBuffer ( ( uint8_t* ) buff, data_addr+i * ( sizeof ( USERDATA_STRU ) ), sizeof ( USERDATA_STRU ) );
		printf ( "the %d data ====================== \r\n",i );
		dbh ( "data", buff, ( sizeof ( USERDATA_STRU ) ) );

	}

}

//ret success 当前ID在FLASH中的索引;fail NO_FIND_HEAD
int readHead(HEADINFO_STRU *head,uint8_t mode)
{
	uint8_t i = 0;
	uint8_t multiple = 0;
	uint16_t remainder = 0;
	uint16_t loop = 0;
	uint32_t address = 0;
	uint32_t curIndex = 0;
	int ret = 0;
    int32_t iTime1, iTime2;

    //512个头，1页的空间
    HEADINFO_STRU firstData,lastData;
    
	if ( head == NULL )
	{
		return NO_FIND_HEAD;
	}	

    log_d("2 want find head = %x\r\n",head->headData.id);
	

    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);
    

	if ( mode == CARD_MODE )
	{
		address = CARD_NO_HEAD_ADDR;
		curIndex = gRecordIndex.cardNoIndex;
	}
	else if ( mode == USER_MODE )
	{
		address = USER_ID_HEAD_ADDR;
		curIndex = gRecordIndex.userIdIndex;		
	}
	else if ( mode == CARD_DEL_MODE )
	{
		address = CARD_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delCardNoIndex;		
	}
	else if ( mode == USER_DEL_HEAD_ADDR )
	{
		address = USER_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delUserIdIndex;
	}	
	
	multiple = curIndex / HEAD_NUM_SECTOR;
	remainder = curIndex % HEAD_NUM_SECTOR;

    //1.读取单页或者多页最后一页的地址
    address += multiple * HEAD_NUM_SECTOR  * HEAD_lEN;

    log_d("addr = %x,multiple = %d,remainder=%d\r\n",address,multiple,remainder);
    
    
    //2.读取最后一页第一个卡号和最后一个卡号；
    firstData.headData.id = 0;
    lastData.headData.id = 0;   

    iTime1 = xTaskGetTickCount();   /* 记下开始时间 */
    
    ret = FRAM_Read (FM24V10_1, address, &firstData,CARD_USER_LEN);
    if(ret == 0)
    {
        log_e("read fram error\r\n");
        return NO_FIND_HEAD; 
    }
    
    ret = FRAM_Read (FM24V10_1, address+(remainder-1)* CARD_USER_LEN, &lastData,CARD_USER_LEN); 
    if(ret == 0)
    {
        log_e("read fram error\r\n");
        return NO_FIND_HEAD;
    }  
    log_d("head = %x,last page %x,%x\r\n",head->headData.id,firstData.headData.id,lastData.headData.id);


    
    if((head->headData.id >= firstData.headData.id) && (head->headData.id <= lastData.headData.id))
    {
//        ret = Bin_Search(sectorBuff,remainder,head->headData.id);
        ret = Bin_Search_addr(address,remainder,head->headData.id); 
        
        if(ret != NO_FIND_HEAD)
        {
            //直接返回当前目标值的FLASH索引
            iTime2 = xTaskGetTickCount();   /* 记下结束时间 */
            log_d ( "find it，use %d ms,head addr = %d\r\n",iTime2 - iTime1,multiple*HEAD_NUM_SECTOR+ret);              
            return multiple*HEAD_NUM_SECTOR+ret;
        }
    }    
    
    for(i=0;i<multiple;i++)
    {
        address = CARD_NO_HEAD_ADDR;//从零开始读;
        address += i * HEAD_NUM_SECTOR  * CARD_USER_LEN;
        
        
        //2.读取第一个卡号和最后一个卡号；
        firstData.headData.id = 0;
        lastData.headData.id = 0;   
        
        ret = FRAM_Read (FM24V10_1, address, &firstData,CARD_USER_LEN);
        if(ret == 0)
        {
            log_e("read fram error\r\n");
            return NO_FIND_HEAD; 
        }
        
        ret = FRAM_Read (FM24V10_1, address+(HEAD_NUM_SECTOR-1)* CARD_USER_LEN, &lastData,CARD_USER_LEN); 
        if(ret == 0)
        {
            log_e("read fram error\r\n");
            return NO_FIND_HEAD;
        }  
        log_d("head = %x,last page %x,%x\r\n",head->headData.id,firstData.headData.id,lastData.headData.id);

        
        if(head->headData.id >= firstData.headData.id && head->headData.id <= lastData.headData.id)
        {
//            ret = Bin_Search(sectorBuff,HEAD_NUM_SECTOR,head->headData.id);
            ret = Bin_Search_addr(address,HEAD_NUM_SECTOR,head->headData.id);       
            if(ret != NO_FIND_HEAD)
            {
            	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
            	log_d ( "find it，use %d ms,index = %d\r\n",iTime2 - iTime1,i*HEAD_NUM_SECTOR+ret);      
                
                return i*HEAD_NUM_SECTOR+ret;
            }

        }
    
    }
    
	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_d ( "read all Head，use %d ms\r\n",iTime2 - iTime1 );    

    return NO_FIND_HEAD;

}



void sortHead(HEADINFO_STRU *head,int length)
{
    int left, right, mid;
    HEADINFO_STRU tmp;

    memset(&tmp,0x00,sizeof(tmp));
    
    for (int i = 1; i < length; i++)
    {
        /* 找到数组中第一个无序的数，保存为tmp */
        if (head[i].headData.id < head[i - 1].headData.id)
        {
            tmp = head[i];
        }
        else
        {
            continue;
        }
		/* 找到数组中第一个无序的数，保存为tmp */

		/* 二分查询开始 */
        left = 0;
        right = i - 1;
        while (left <= right)
        {
            mid = (left + right) / 2;
            if (head[mid].headData.id > tmp.headData.id)
            {
                right = mid - 1;
            }
            else
            {
                left = mid + 1;
            }
        }
		/* 二分查询结束,此时a[left]>=a[i],记录下left的值 */

		/* 将有序数组中比要插入的数大的数右移 */
        for (int j = i; j > left; j--)
        {
            head[j] = head[j - 1];
        }
		/* 将有序数组中比要插入的数大的数右移 */

		// 将left位置赋值为要插入的数
        head[left] = tmp;
    }
}

//return:<0,未找到，>=0在FLASH中的索引值
static int Bin_Search(HEADINFO_STRU *num,int numsSize,int target)
{
	int left = 0,right = numsSize-1,mid;
	
	while(left <= right)
	{
		mid = (left + right) / 2;//确定中间元素	
		if(num[mid].headData.id > target)
		{
			right = mid-1; //mid已经交换过了,right往前移一位
		}
		else if(num[mid].headData.id < target)
		{
			left = mid+1;//mid已经交换过了,left往后移一位
		}	
		else //判断是否相等
		{		    
            return num[mid].flashAddr;
		}
	}        
    return NO_FIND_HEAD;
}

//返回查找到值的FLASH索引
static int Bin_Search_addr(uint32_t startAddr,int numsSize,int target)
{
	int low = 0,high = numsSize-1,mid;
	HEADINFO_STRU tmpData;
	uint8_t ret = 0;
	    
	while(low <= high)
	{
		mid = low + ((high - low) >> 1); //获取中间值

        ret = FRAM_Read (FM24V10_1, startAddr+(mid)*CARD_USER_LEN, &tmpData, (1)* CARD_USER_LEN);
      
        if(ret == 0)
        {
            log_e("read fram error\r\n");
            return NO_FIND_HEAD; 
        }  
        
		if(tmpData.headData.id == target)
		{
		    //return tmpData.flashAddr;
		    return mid;
		}
		else if(tmpData.headData.id < target)
		{
		    low = mid+1;//mid已经交换过了,low往后移一位
		}
		else
		{
		    high = mid-1; //mid已经交换过了,right往前移一位
		}
	}        
    return NO_FIND_HEAD;
}


void addHead(uint8_t *head,uint8_t mode)
{
    uint8_t multiple = 0;
	uint16_t remainder = 0;
	uint32_t addr = 0;
	uint8_t ret = 0;
	uint32_t curIndex = 0;
	uint8_t i = 0;

    int32_t iTime1, iTime2;	
    HEADINFO_STRU rxCard;
    
    
   iTime1 = xTaskGetTickCount();   /* 记下开始时间 */
   //1.先判定当前有多少个卡号;
    ClearRecordIndex();
    optRecordIndex(&gRecordIndex,READ_PRARM);

	if ( mode == CARD_MODE )
	{
		addr = CARD_NO_HEAD_ADDR;
		curIndex = gRecordIndex.cardNoIndex;
	}
	else if ( mode == USER_MODE )
	{
		addr = USER_ID_HEAD_ADDR;
		curIndex = gRecordIndex.userIdIndex;		
	}
	else if ( mode == CARD_DEL_MODE )
	{
		addr = CARD_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delCardNoIndex;		
	}
	else if ( mode == USER_DEL_MODE )
	{
		addr = USER_DEL_HEAD_ADDR;
		curIndex = gRecordIndex.delUserIdIndex;
	}	    
    
    multiple = curIndex / HEAD_NUM_SECTOR;
    remainder = curIndex % HEAD_NUM_SECTOR;

    memset(gSectorBuff,0x00,sizeof(gSectorBuff));

    //索引要从1开始    
    if(multiple==0 && remainder==0)
    {
        //第一条记录赋值
        gSectorBuff[0].flashAddr = curIndex;
        memcpy(gSectorBuff[0].headData.sn,head,CARD_NO_LEN_BCD);
        
        
        //写入到存储区域
        ret = FRAM_Write ( FM24V10_1, addr,  gSectorBuff,1 * sizeof(HEADINFO_STRU));
        
        if(ret == 0)
        {
            log_e("write fram error\r\n");
        }        
    }
    else 
    {
        //2.追加到最后一页；
        if(multiple > 0)
        {
            addr += multiple * HEAD_NUM_SECTOR  * sizeof(HEADINFO_STRU);  
        }
        
        memset(gSectorBuff,0x00,sizeof(gSectorBuff));
        
        //3.读取最后一页
        ret = FRAM_Read (FM24V10_1, addr, gSectorBuff, (remainder)* sizeof(HEADINFO_STRU));
        if(ret == 0)
        {
            log_e("read fram error\r\n");
        }
        
        //4.赋值,追加需要添加的卡号到最后一个位置
        gSectorBuff[remainder].flashAddr = curIndex;
        memcpy(gSectorBuff[remainder].headData.sn,head,CARD_NO_LEN_BCD);

        log_d("gSectorBuff[remainder].headData.id = %x gSectorBuff[remainder].flashAddr = %d\r\n",gSectorBuff[remainder].headData.id,gSectorBuff[remainder].flashAddr);

       //5.排序
        sortHead(gSectorBuff,remainder+1);

        //6.写入到存储区域
        ret = FRAM_Write ( FM24V10_1, addr, gSectorBuff,(remainder+1)* sizeof(HEADINFO_STRU));
        
        if(ret == 0)
        {
            log_e("write fram error\r\n");
        }          

    }
    
	if ( mode == CARD_MODE )
	{
        gRecordIndex.cardNoIndex++;
	}
	else if ( mode == USER_MODE )
	{
        gRecordIndex.userIdIndex++;		
	}
	else if ( mode == CARD_DEL_MODE )
	{
        gRecordIndex.delCardNoIndex++;		
	}
	else if ( mode == USER_DEL_MODE )
	{
        gRecordIndex.delUserIdIndex++;
	}

    log_d("cardNoIndex = %d,userIdIndex = %d\r\n",gRecordIndex.cardNoIndex,gRecordIndex.userIdIndex);	
    optRecordIndex(&gRecordIndex,WRITE_PRARM);

	iTime2 = xTaskGetTickCount();	/* 记下结束时间 */
	log_e ( "add head成功，耗时: %dms\r\n",iTime2 - iTime1 );

  
}

void qSortCard(HEADINFO_STRU *head,uint32_t length)
{
    quickSortNor(head,0, length-1);
}


