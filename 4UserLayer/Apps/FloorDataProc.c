/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : FloorDataProc.c
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

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#define LOG_TAG    "FloorData"
#include "elog.h"
#include "FloorDataProc.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define AUTO_REG            1
#define MANUAL_REG          2

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
ELEVATOR_TRANBUFF_STRU gElevtorData,gRecvElevtorData;


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/



void sendQueueToDev(ELEVATOR_TRANBUFF_STRU *devSendData)
{
    /* 使用消息队列实现指针变量的传递 */
    if(xQueueSend(xTransDataQueue,              /* 消息队列句柄 */
               (void *)devSendData,   /* 发送指针变量recv_buf的地址 */
               (TickType_t)10) != pdPASS )
    {
        log_d("the queue is full!\r\n");                
        xQueueReset(xTransDataQueue);
    } 
    else
    {
        log_d("send queue value = %x,devsn=%d\r\n",devSendData->value,devSendData->devSn);
    }

}

SYSERRORCODE_E packetToElevatorExtend(USERDATA_STRU *localUserData,ELEVATOR_BUFF_STRU *devSendData)
{     
    SYSERRORCODE_E result = NO_ERR;
    char authLayer[64] = {0}; //权限楼层，最多64层
    int num = 0;       
    
    memcpy(authLayer,localUserData->accessFloor,FLOOR_ARRAY_LEN);
    
    num = strlen((const char*)authLayer);

    log_d("localUserData->accessFloor num = %d\r\n",num);
    dbh("authLayer",authLayer,num);

    
    if(num > 1)//多层权限，手动
    {
        result = calcMultilFloor((uint8_t *)authLayer,num,devSendData);
    }
    else    //单层权限，直接呼默认权限楼层，自动
    {
        if(localUserData->defaultFloor != authLayer[0])
        {
        
            log_d("defaultFloor != authLayer,%d,%d\r\n",localUserData->defaultFloor,authLayer[0]);
            localUserData->defaultFloor = authLayer[0];
        }   
        
	    if(localUserData->defaultFloor == 0)
	    {
	        return INVALID_FLOOR;//无效的楼层
	    }

	    log_d("localUserData->defaultFloor = %d\r\n",localUserData->defaultFloor);
	    
        result = calcSingleFloor(localUserData->defaultFloor,devSendData); 
        
    } 

    log_d("1 value = %x,devsn=%d\r\n",devSendData->data[0].value,devSendData->data[0].devSn);
    log_d("2 value = %x,devsn=%d\r\n",devSendData->data[1].value,devSendData->data[1].devSn);
    log_d("3 value = %x,devsn=%d\r\n",devSendData->data[2].value,devSendData->data[2].devSn);
    log_d("4 value = %x,devsn=%d\r\n",devSendData->data[3].value,devSendData->data[3].devSn);   
    
    return result;
}



//第一位权限，第二位按键

//单层权限
SYSERRORCODE_E calcSingleFloor(uint8_t layer,ELEVATOR_BUFF_STRU *eBuf)
{
    uint8_t floor = 0;

    uint8_t offset = ((bsp_dipswitch_read()>>2) & 0x03);
    
    if(layer == 0)
    {
         return INVALID_FLOOR;//无效的楼层
    }

//    if(layer == 253)
//    {
//        floor = 1;
//    }
//    else if(layer == 254)
//    {
//        floor = 2;
//    }
//    else if(layer == 255)
//    {
//        floor = 3;
//    }
    

    if(layer > 200)
    {
        if(256-layer == offset)
        {
            floor = offset - (256-layer)+1;
        }
        else if(256-layer < offset)
        {
            floor = offset-(256-layer)+1;
        }
        else
        {
            floor = offset+1;
        }        
    }
    else
    {
        floor = layer + ((bsp_dipswitch_read()>>2) & 0x03); //根据拨码补偿楼层数
    }

    log_d("calc Single Floor = %d\r\n",floor);

    if(floor > 0 && floor<=16)
    {
        eBuf->data[0].devSn = 1;
        eBuf->data[0].value = setbit(0,floor-1);   //前16字节是权限，后16字节是按键    
        eBuf->data[0].value = setbit(eBuf->data[0].value<<16UL,floor-1);
        
        log_d("send desc floor = %d,%d\r\n",eBuf->data[0].value,eBuf->data[0].devSn);  
        
    }
    else if(floor >=17 && floor<=32)
    {
        eBuf->data[1].devSn = 2;
        eBuf->data[1].value = setbit(0,floor-17); 
        eBuf->data[1].value = setbit(eBuf->data[1].value<<16UL,floor-17);
    }
    else if(floor >=33 && floor<=48)
    {
        eBuf->data[2].devSn = 3;
        eBuf->data[2].value = setbit(0,floor-33); 
        eBuf->data[2].value = setbit(eBuf->data[2].value<<16UL,floor-33);    
    }
    else if(floor >=49 && floor<=64)
    {
        eBuf->data[3].devSn = 4;
        eBuf->data[3].value = setbit(0,floor-49); 
        eBuf->data[3].value = setbit(eBuf->data[3].value<<16UL,floor-49);    
    } 
    return NO_ERR;
}


//SYSERRORCODE_E calcSingleFloor(uint8_t floor,ELEVATOR_BUFF_STRU *eBuf)
//{    
//   
//    if(floor == 0)
//    {
//         return INVALID_FLOOR;//无效的楼层
//    }

//    if(floor > 0 && floor<=16)
//    {
//        eBuf->data[0].devSn = 1;
//        eBuf->data[0].value = setbit(0,floor-1);
//    }
//    else if(floor >=17 && floor<=32)
//    {
//        eBuf->data[0].devSn = 2;
//        eBuf->data[0].value = setbit(0,floor-17);    
//    }
//    else if(floor >=33 && floor<=48)
//    {
//        eBuf->data[0].devSn = 3;
//        eBuf->data[0].value = setbit(0,floor-33);    
//    }
//    else if(floor >=49 && floor<=64)
//    {
//        eBuf->data[0].devSn = 4;
//        eBuf->data[0].value = setbit(0,floor-49);    
//    }        

//    return NO_ERR;
//}

//多层权限
//SYSERRORCODE_E calcMultilFloor(uint8_t *floorBuf,uint8_t num,ELEVATOR_BUFF_STRU *eBuf)
//{
//    uint8_t i = 0;    

//    //按目前需求，超过1层权限，就按所有楼层处理
//    #if 1
//    for(i=0;i<num;i++)
//    {
//        if(floorBuf[i] > 0 && floorBuf[i]<=16)
//        {
//            eBuf->data[0].devSn  = 1;
//            eBuf->data[0].value = setbit(eBuf->data[0].value,floorBuf[i]-1);
//        }
//        else if(floorBuf[i] >=17 && floorBuf[i]<=32)
//        {
//            eBuf->data[1].devSn  = 1;
//            eBuf->data[1].value = setbit(eBuf->data[1].value,floorBuf[i]-17);    
//        }
//        else if(floorBuf[i] >=33 && floorBuf[i]<=48)
//        {
//            eBuf->data[2].devSn  = 1;
//            eBuf->data[2].value = setbit(eBuf->data[2].value,floorBuf[i]-33);    
//        }
//        else if(floorBuf[i] >=49 && floorBuf[i]<=64)
//        {
//            eBuf->data[3].devSn  = 1;
//            eBuf->data[3].value = setbit(eBuf->data[3].value,floorBuf[i]-49);    
//        } 
//    }
//    #endif
//    
//    log_d("1 value = %x,devsn=%d\r\n",eBuf->data[0].value,eBuf->data[0].devSn);
//    log_d("2 value = %x,devsn=%d\r\n",eBuf->data[1].value,eBuf->data[1].devSn);
//    log_d("3 value = %x,devsn=%d\r\n",eBuf->data[2].value,eBuf->data[2].devSn);
//    log_d("4 value = %x,devsn=%d\r\n",eBuf->data[3].value,eBuf->data[3].devSn);


//    return NO_ERR;
//}



//多层权限
SYSERRORCODE_E calcMultilFloor(uint8_t *floorBuf,uint8_t num,ELEVATOR_BUFF_STRU *eBuf)
{
    uint8_t i = 0;
    uint8_t tmpFloor = 0;
    uint8_t curFloor = 0;
    //需补偿的楼层
    uint8_t offset = ((bsp_dipswitch_read()>>2) & 0x03);

    //按目前需求，超过1层权限，就按所有楼层处理  
    for(i=0;i<num;i++)
    {
        if(floorBuf[i] > 200)
        {
            if(256-floorBuf[i] == offset)
            {
                curFloor = offset - (256-floorBuf[i])+1;
            }
            else if(256-floorBuf[i] < offset)
            {
                curFloor = offset-(256-floorBuf[i])+1;
            }
            else
            {
                curFloor = offset+1;
            }
        }        
        else
        {
            curFloor = floorBuf[i] + ((bsp_dipswitch_read()>>2) & 0x03); //根据拨码补偿楼层数
        }

        
        log_d("calc  Multil Floor = %d\r\n",curFloor);        


        if(curFloor > 0 && curFloor<=16)
        {
            eBuf->data[0].devSn = 1;
            eBuf->data[0].value = setbit(eBuf->data[0].value<<16UL,curFloor-1);
            
            log_d("send desc floor = %d,%d\r\n",eBuf->data[0].value,eBuf->data[0].devSn);  
            
        }
        else if(curFloor >=17 && curFloor<=32)
        {
            eBuf->data[1].devSn = 2;
            eBuf->data[1].value = setbit(eBuf->data[1].value<<16UL,curFloor-17);
        }
        else if(curFloor >=33 && curFloor<=48)
        {
            eBuf->data[2].devSn = 3;
            eBuf->data[2].value = setbit(eBuf->data[2].value<<16UL,curFloor-33);    
        }
        else if(curFloor >=49 && curFloor<=64)
        {
            eBuf->data[3].devSn = 4;
            eBuf->data[3].value = setbit(eBuf->data[3].value<<16UL,curFloor-49);    
        }        
        
    }
   
    
    log_d("1 value = %x,devsn=%d\r\n",eBuf->data[0].value,eBuf->data[0].devSn);
    log_d("2 value = %x,devsn=%d\r\n",eBuf->data[1].value,eBuf->data[1].devSn);
    log_d("3 value = %x,devsn=%d\r\n",eBuf->data[2].value,eBuf->data[2].devSn);
    log_d("4 value = %x,devsn=%d\r\n",eBuf->data[3].value,eBuf->data[3].devSn);


    return NO_ERR;
}



