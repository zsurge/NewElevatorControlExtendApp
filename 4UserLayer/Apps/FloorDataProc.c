/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : FloorDataProc.c
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

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#define LOG_TAG    "FloorData"
#include "elog.h"
#include "FloorDataProc.h"

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define AUTO_REG            1
#define MANUAL_REG          2

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
ELEVATOR_TRANBUFF_STRU gElevtorData,gRecvElevtorData;


/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/



void sendQueueToDev(ELEVATOR_TRANBUFF_STRU *devSendData)
{
    /* ʹ����Ϣ����ʵ��ָ������Ĵ��� */
    if(xQueueSend(xTransDataQueue,              /* ��Ϣ���о�� */
               (void *)devSendData,   /* ����ָ�����recv_buf�ĵ�ַ */
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
    char authLayer[64] = {0}; //Ȩ��¥�㣬���64��
    int num = 0;       
    
    memcpy(authLayer,localUserData->accessFloor,FLOOR_ARRAY_LEN);
    
    num = strlen((const char*)authLayer);

    log_d("localUserData->accessFloor num = %d\r\n",num);
    dbh("authLayer",authLayer,num);

    
    if(num > 1)//���Ȩ�ޣ��ֶ�
    {
        result = calcMultilFloor(authLayer,num,devSendData);
    }
    else    //����Ȩ�ޣ�ֱ�Ӻ�Ĭ��Ȩ��¥�㣬�Զ�
    {
        if(localUserData->defaultFloor != authLayer[0])
        {
        
            log_d("defaultFloor != authLayer,%d,%d\r\n",localUserData->defaultFloor,authLayer[0]);
            localUserData->defaultFloor = authLayer[0];
        }   
        
	    if(localUserData->defaultFloor == 0)
	    {
	        return INVALID_FLOOR;//��Ч��¥��
	    }

	    log_d("localUserData->defaultFloor = %d\r\n",localUserData->defaultFloor);
	    
        result = calcSingleFloor(localUserData->defaultFloor,devSendData); 
        
    } 

    log_d("1 value = %x,devsn=%d\r\n",devSendData->data[0].value,devSendData->data[0].devSn);
    log_d("2 value = %x,devsn=%d\r\n",devSendData->data[1].value,devSendData->data[1].devSn);
    log_d("3 value = %x,devsn=%d\r\n",devSendData->data[2].value,devSendData->data[2].devSn);
    log_d("4 value = %x,devsn=%d\r\n",devSendData->data[3].value,devSendData->data[3].devSn);
    log_d("1 value = %x,devsn=%d\r\n",devSendData->data[4].value,devSendData->data[4].devSn);
    log_d("2 value = %x,devsn=%d\r\n",devSendData->data[5].value,devSendData->data[5].devSn);
    log_d("3 value = %x,devsn=%d\r\n",devSendData->data[6].value,devSendData->data[6].devSn);
    log_d("4 value = %x,devsn=%d\r\n",devSendData->data[7].value,devSendData->data[7].devSn);
   
    
    return result;
}



//��һλȨ�ޣ��ڶ�λ����

//����Ȩ��
SYSERRORCODE_E calcSingleFloor(uint8_t layer,ELEVATOR_BUFF_STRU *eBuf)
{
    uint8_t floor = 0;

    uint8_t offset = ((bsp_dipswitch_read()>>2) & 0x03);
    
    if(layer == 0)
    {
         return INVALID_FLOOR;//��Ч��¥��
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
        floor = layer + ((bsp_dipswitch_read()>>2) & 0x03); //���ݲ��벹��¥����
    }

    log_d("calc Single Floor = %d\r\n",floor);

    if(floor > 0 && floor<=8)
    {
        eBuf->data[0].devSn = 1;
        eBuf->data[0].value = setbit(0,(floor-1)*2);   //��һλȨ��    
        eBuf->data[0].value = setbit(eBuf->data[0].value,(floor-1)*2+1);//�ڶ�λ����
        log_d("send desc floor = %d,%d\r\n",eBuf->data[0].value,eBuf->data[0].devSn);  
        
    }
    else if(floor >=9 && floor<=16)
    {
        eBuf->data[1].devSn = 2;
        eBuf->data[1].value = setbit(0,(floor-9)*2); 
        eBuf->data[1].value = setbit(eBuf->data[1].value,(floor-9)*2+1);
    }
    else if(floor >=17 && floor<=24)
    {
        eBuf->data[2].devSn = 3;
        eBuf->data[2].value = setbit(0,(floor-17)*2);  
        eBuf->data[2].value = setbit(eBuf->data[2].value,(floor-17)*2+1);        
    }
    else if(floor >=25 && floor<=32)
    {
        eBuf->data[3].devSn = 4;
        eBuf->data[3].value = setbit(0,(floor-25)*2);    
        eBuf->data[3].value = setbit(eBuf->data[3].value,(floor-25)*2+1);        
    }        
    else if(floor >=33 && floor<=40)
    {
        eBuf->data[4].devSn = 5;
        eBuf->data[4].value = setbit(0,(floor-33)*2);    
        eBuf->data[4].value = setbit(eBuf->data[4].value,(floor-33)*2+1);        
    }
    else if(floor >=41 && floor<=48)
    {
        eBuf->data[5].devSn = 6;
        eBuf->data[5].value = setbit(0,(floor-41)*2);    
        eBuf->data[5].value = setbit(eBuf->data[5].value,(floor-41)*2+1);        
    }
    else if(floor >=49 && floor<=56)
    {
        eBuf->data[6].devSn = 7;
        eBuf->data[6].value = setbit(0,(floor-49)*2);    
        eBuf->data[6].value = setbit(eBuf->data[6].value,(floor-49)*2+1);        
    } 
    else if(floor >=57 && floor<=64)
    {
        eBuf->data[7].devSn = 8;
        eBuf->data[7].value = setbit(0,(floor-57)*2);    
        eBuf->data[7].value = setbit(eBuf->data[7].value,(floor-57)*2+1);        
    }     
    return NO_ERR;
}


//SYSERRORCODE_E calcSingleFloor(uint8_t floor,ELEVATOR_BUFF_STRU *eBuf)
//{    
//   
//    if(floor == 0)
//    {
//         return INVALID_FLOOR;//��Ч��¥��
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

//���Ȩ��
//SYSERRORCODE_E calcMultilFloor(uint8_t *floorBuf,uint8_t num,ELEVATOR_BUFF_STRU *eBuf)
//{
//    uint8_t i = 0;    

//    //��Ŀǰ���󣬳���1��Ȩ�ޣ��Ͱ�����¥�㴦��
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



//���Ȩ��
SYSERRORCODE_E calcMultilFloor(uint8_t *floorBuf,uint8_t num,ELEVATOR_BUFF_STRU *eBuf)
{
    uint8_t i = 0;
    uint8_t tmpFloor = 0;
    uint8_t curFloor = 0;
    //�貹����¥��
    uint8_t offset = ((bsp_dipswitch_read()>>2) & 0x03);

    //��Ŀǰ���󣬳���1��Ȩ�ޣ��Ͱ�����¥�㴦��  
    for(i=0;i<num;i++)
    {

//        if(floorBuf[i] == 253)
//        {
//            curFloor = 1;
//        }
//        else if(floorBuf[i] == 254)
//        {
//            curFloor = 2;
//        }
//        else if(floorBuf[i] == 255)
//        {
//            curFloor = 3;
//        }

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
            curFloor = floorBuf[i] + ((bsp_dipswitch_read()>>2) & 0x03); //���ݲ��벹��¥����
        }

        
        log_d("calc  Multil Floor = %d\r\n",curFloor);
        
        if(curFloor > 0 && curFloor<=8)
        {
            eBuf->data[0].devSn  = 1;
            eBuf->data[0].value = setbit(eBuf->data[0].value,(curFloor-1)*2);
//            eBuf->data[0].value = setbit(eBuf->data[0].value,(curFloor-1)*2+1);
            
        }
        else if(curFloor >=9 && curFloor<=16)
        {
            eBuf->data[1].devSn  = 2;
            eBuf->data[1].value = setbit(eBuf->data[1].value,(curFloor-9)*2);
//            eBuf->data[1].value = setbit(eBuf->data[1].value,(curFloor-1)*2+1);
        }
        else if(curFloor >=17 && curFloor<=24)
        {
            eBuf->data[2].devSn  = 3;
            eBuf->data[2].value = setbit(eBuf->data[2].value,(curFloor-17)*2);
//            eBuf->data[2].value = setbit(eBuf->data[2].value,(curFloor-1)*2+1);
        }
        
        else if(curFloor >=25 && curFloor<=32)
        {
            eBuf->data[3].devSn  = 4;
            eBuf->data[3].value = setbit(eBuf->data[3].value,(curFloor-25)*2);
//            eBuf->data[3].value = setbit(eBuf->data[3].value,(curFloor-1)*2+1);
        }
        else if(curFloor >=33 && curFloor<=40)
        {
            eBuf->data[4].devSn  = 5;
            eBuf->data[4].value = setbit(eBuf->data[4].value,(curFloor-33)*2);
//            eBuf->data[4].value = setbit(eBuf->data[4].value,(curFloor-1)*2+1);
        } 
        if(curFloor > 41 && curFloor<=48)
        {
            eBuf->data[5].devSn  = 6;
            eBuf->data[5].value = setbit(eBuf->data[5].value,(curFloor-41)*2);
//            eBuf->data[5].value = setbit(eBuf->data[5].value,(curFloor-1)*2+1);
            
        }
        else if(curFloor >=49 && curFloor<=56)
        {
            eBuf->data[6].devSn  = 7;
            eBuf->data[6].value = setbit(eBuf->data[6].value,(curFloor-49)*2);
//            eBuf->data[6].value = setbit(eBuf->data[6].value,(curFloor-1)*2+1);
        }
        else if(curFloor >=57 && curFloor<=64)
        {
            eBuf->data[7].devSn  = 8;
            eBuf->data[7].value = setbit(eBuf->data[7].value,(curFloor-57)*2);
//            eBuf->data[7].value = setbit(eBuf->data[7].value,(curFloor-1)*2+1);
        }
        
    }
   
    
    log_d("1 value = %x,devsn=%d\r\n",eBuf->data[0].value,eBuf->data[0].devSn);
    log_d("2 value = %x,devsn=%d\r\n",eBuf->data[1].value,eBuf->data[1].devSn);
    log_d("3 value = %x,devsn=%d\r\n",eBuf->data[2].value,eBuf->data[2].devSn);
    log_d("4 value = %x,devsn=%d\r\n",eBuf->data[3].value,eBuf->data[3].devSn);
    log_d("5 value = %x,devsn=%d\r\n",eBuf->data[4].value,eBuf->data[4].devSn);
    log_d("6 value = %x,devsn=%d\r\n",eBuf->data[5].value,eBuf->data[5].devSn);
    log_d("7 value = %x,devsn=%d\r\n",eBuf->data[6].value,eBuf->data[6].devSn);
    log_d("8 value = %x,devsn=%d\r\n",eBuf->data[7].value,eBuf->data[7].devSn);

    return NO_ERR;
}



