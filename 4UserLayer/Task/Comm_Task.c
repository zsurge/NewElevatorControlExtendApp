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

    ELEVATOR_BUFF_STRU *recvBuf = &gRecvElevtorData;
    uint32_t i = 0;
    uint16_t bufLen = 0;    
    uint8_t sendBuf[104] = {0x7E,0x55,0x53,0x5A,0x8D,0x4B,0x57,0xA1,0x00,0x00,0x00,0x00,0x02,0x01,0x04,0xBC,0x00,0x00,0x00,0x00,0x00,0x13,0x0C,0x1B,0x42,0x01,0x01,0x00,0x00,0x00,0x01,0x00,0x00}; 
//    uint8_t sendBuf[104] =  { 0x7E,0x55,0x53,0x64,0x1E,0xFD,0x19,0xA1,0x00,0x00,0x00,0x00,0x02,0x00,0x04,0xC5,0x00,0x00,0x00,0x00,0x00,0x15,0x07,0x0C,0x42,0x01,0x01,0x00,0x00,0x00,0x01,0x00,0x00};
    uint8_t tmpBuf[104] = {0};
    BaseType_t xReturn = pdTRUE;/* ����һ��������Ϣ����ֵ��Ĭ��ΪpdPASS */
    const TickType_t xMaxBlockTime = pdMS_TO_TICKS(50); /* �������ȴ�ʱ��Ϊ200ms */  
    uint8_t all[104] = { 0x7E,0x51,0x57,0x00,0x00,0x00,0x00,0x0D,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A };


    xLastWakeTime = xTaskGetTickCount();
    
    while (1)
    {

        memset(&gRecvElevtorData,0x00,sizeof(gRecvElevtorData)); 
        memset(tmpBuf,0x00,sizeof(tmpBuf));
        
        xReturn = xQueueReceive( xTransDataQueue,    /* ��Ϣ���еľ�� */
                                 (void *)&recvBuf,  /*�����ȡ���ǽṹ��ĵ�ַ */
                                 xMaxBlockTime); /* ��������ʱ�� */
        if(pdTRUE == xReturn)
        {
            //��Ϣ���ճɹ������ͽ��յ�����Ϣ  
            log_d("recvBuf->type = %d\r\n",recvBuf->type);
            if(recvBuf->type == 0) //��㣬����
            {
                sendBuf[13] = recvBuf->type;
                memcpy(sendBuf+33,recvBuf->data,71);

                memcpy(tmpBuf,sendBuf,54);
                memcpy(tmpBuf+55,sendBuf+54,49);
                tmpBuf[103] = 0x0d;
                dbh("recv queue buff", tmpBuf,104); 
                RS485_SendBuf(COM6,tmpBuf,104);    
            }
            else if(recvBuf->type == 1) //����ֱ��
            {
                sendBuf[13] = recvBuf->type;
                memcpy(sendBuf+33,recvBuf->data,71);
                dbh("recv queue buff", sendBuf,104); 
                RS485_SendBuf(COM6,sendBuf,104);               
            }
            else //����
            {
                RS485_SendBuf(COM6,all,104);   
            }

        }

        
        vTaskDelay(300);
        
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
       printf("%02x ",ch);
       switch (rxFromHost.rxStatus)
        {                
            case STEP1:
                if(0xFE == ch) /*���հ�ͷ*/
                {
                    rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;                            
                }
                break;       
            default:      /* �����������ݰ� */
            
                rxFromHost.rxBuff[rxFromHost.rxCnt++] = ch;  
                rxFromHost.rxCRC ^= ch;
                
                if(rxFromHost.rxCnt == 12)
                {
                
                    if(rxFromHost.rxCRC == rxFromHost.rxBuff[rxFromHost.rxCnt-1])
                    { 
                        memset(&rxFromHost,0x00,sizeof(FROMHOST_STRU));
                        printf("\r\n");
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








