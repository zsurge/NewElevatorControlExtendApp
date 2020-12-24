/******************************************************************************

                  ��Ȩ���� (C), 2013-2023, ���ڲ�˼�߿Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : Comm_Task.c
  �� �� ��   : ����
  ��    ��   :  
  ��������   : 2020��2��28��
  ����޸�   :
  ��������   : ������ͨѶ���������ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2020��2��28��
    ��    ��   :  
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#define LOG_TAG    "CommTask"
#include "elog.h"

#include "Comm_Task.h"
#include "CmdHandle.h"
#include "bsp_uart_fifo.h"
#include "bsp_dipSwitch.h"
#include "FloorDataProc.h"
#include "bsp_usart6.h"
#include "malloc.h"
#include "tool.h"


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

#define UNFINISHED		        	    0x00
#define FINISHED          	 			0x55



#define STEP1   0
#define STEP2   10
#define STEP3   20
#define STEP4   30

typedef struct FROMHOST
{
    uint8_t rxStatus;                   //����״̬
    uint8_t rxCRC;                      //У��ֵ
    uint8_t rxBuff[16];                 //�����ֽ���
    uint16_t rxCnt;                     //�����ֽ���    
}FROMHOST_STRU;

 
#define COMM_TASK_PRIO		(tskIDLE_PRIORITY + 6) 
#define COMM_STK_SIZE 		(configMINIMAL_STACK_SIZE*4)

uint16_t packetBuf(ELEVATOR_TRANBUFF_STRU *src,uint8_t *desc);
uint16_t packetDefault(uint8_t devSn ,uint8_t *desc);

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/
const char *CommTaskName = "vCommTask"; 

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
TaskHandle_t xHandleTaskComm = NULL;  

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/
static void vTaskComm(void *pvParameters);
static uint8_t deal_Serial_Parse(void);

static FROMHOST_STRU rxFromHost;


void CreateCommTask(void)
{
    xTaskCreate((TaskFunction_t )vTaskComm,         
                (const char*    )CommTaskName,       
                (uint16_t       )COMM_STK_SIZE, 
                (void*          )NULL,              
                (UBaseType_t    )COMM_TASK_PRIO,    
                (TaskHandle_t*  )&xHandleTaskComm);
}


static void vTaskComm(void *pvParameters)
{
    TickType_t xLastWakeTime;
    ELEVATOR_TRANBUFF_STRU *recvBuf = &gRecvElevtorData;
    uint8_t devSn = 0;

    uint32_t i = 0;
    uint8_t buf[32] = {0};
    uint16_t bufLen = 0;    
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(30); /* �������ȴ�ʱ��Ϊ200ms */  

    xLastWakeTime = xTaskGetTickCount();
    
    while (1)
    {
        memset(&gRecvElevtorData,0x00,sizeof(gRecvElevtorData));       

  
        xReturn = xQueueReceive( xTransDataQueue,    /* ��Ϣ���еľ�� */
                                 (void *)recvBuf,  /*�����ȡ���ǽṹ��ĵ�ַ */
                                 xMaxBlockTime); /* ��������ʱ�� */
        if(pdTRUE == xReturn)
        {
            //��Ϣ���ճɹ������ͽ��յ�����Ϣ   
            memset(buf,0x00,sizeof(buf));
            bufLen = packetBuf(recvBuf ,buf);
            RS485_SendBuf(COM6,buf,bufLen); 
            dbh("send com6 buff", buf, bufLen);  

        }
        else
        {
            //����Ϣ����������Ϣ            
            memset(buf,0x00,sizeof(buf));
            if(devSn > 7)
            {
              devSn = 0;
            }
            devSn++;
            bufLen = packetDefault(devSn,buf);
            RS485_SendBuf(COM6,buf,bufLen); 
        }  
        
        vTaskDelay(50);
        
        if(deal_Serial_Parse() == FINISHED)
        { 
            log_d("recv extend return data\r\n");
        }  

		/* �����¼���־����ʾ������������ */        
		xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_1);  
    }

}

static uint8_t deal_Serial_Parse(void)
{
    uint8_t ch = 0;
    uint8_t devID = 0;

    
    while(RS485_Recv(COM6,&ch,1))
    {
       switch (rxFromHost.rxStatus)
        {                
            case STEP1:
                if(0xA6 == ch) /*���հ�ͷ*/
                {
                    rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;
                    rxFromHost.rxCRC ^= ch;
                    if(rxFromHost.rxCnt==3)
                        rxFromHost.rxStatus = STEP2;
                }

                break;
           case STEP2:
                devID = (bsp_dipswitch_read() & 0x03)+1;

                if(devID == ch) //�ж��ڶ����ֽ��Ƿ��ǵ�ǰ�豸ID����ȡ���뿪�ص�ֵ
                {
                    rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;
                    rxFromHost.rxCRC ^= ch;               
                    rxFromHost.rxStatus = STEP3;                
                }
                else
                {                
                   memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));                   
                }
                break;           
            default:      /* �����������ݰ� */
            
                rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;  
                
                if(rxFromHost.rxCnt == 8)
                {
                
                    if(rxFromHost.rxCRC == rxFromHost.rxBuff[rxFromHost.rxCnt-1])
                    { 
                        memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));
                        return FINISHED;                         
                    }  
                    memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));
                }
                else
                {
                     rxFromHost.rxCRC ^= ch;
                }
             
                break;
         }
         
    }   

    return UNFINISHED;
}


uint16_t packetBuf(ELEVATOR_TRANBUFF_STRU *src,uint8_t *desc)
{
    uint8_t buf[32] = {0};
    uint16_t len = 0;    

    buf[len++] = 0xA6;
    buf[len++] = 0xA6;
    buf[len++] = 0xA6;
    buf[len++] = src->devSn;
    buf[len++] = 0x03;    
    buf[len++] = 0xA1;
    buf[len++] = src->value/256;//��8λ
    buf[len++] = src->value%256;//��8λ
    buf[len++] = xorCRC(buf,8);

    memcpy(desc,buf,len);

    return len;    
}

uint16_t packetDefault(uint8_t devSn ,uint8_t *desc)
{
    uint8_t buf[32] = {0};
    uint16_t len = 0;

    buf[len++] = 0xA6;
    buf[len++] = 0xA6;
    buf[len++] = 0xA6;
    buf[len++] = devSn;
    buf[len++] = 0x03;    
    buf[len++] = 0x06;
    buf[len++] = 0x00;//��8λ
    buf[len++] = 0x00;//��8λ
    buf[len++] = xorCRC(buf,8);

    memcpy(desc,buf,len);

    return len;    
}





