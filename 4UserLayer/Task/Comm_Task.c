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
            RS485_SendBuf(COM6,recvBuf->data,sizeof(recvBuf->data)); 
            dbh("send com6 buff", buf, bufLen); 
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








