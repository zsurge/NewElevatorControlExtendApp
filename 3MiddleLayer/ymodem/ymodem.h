//Define to prevent recursive inclusion
#ifndef __YMODEM_H_
#define __YMODEM_H_

#include "stdint.h"
#include "stmflash.h"
#include "string.h"
#include "stdlib.h"


//Exported macro
#define PACKET_SEQNO_INDEX      (1)
#define PACKET_SEQNO_COMP_INDEX (2)

#define PACKET_HEADER           (3)
#define PACKET_TRAILER          (2)
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)

#define FILE_NAME_LENGTH        (256)
#define FILE_SIZE_LENGTH        (16)

#define SOH                     (0x01)  /* start of 128-byte data packet */
#define STX                     (0x02)  /* start of 1024-byte data packet */
#define EOT                     (0x04)  /* end of transmission */
#define ACK                     (0x06)  /* acknowledge */
#define NACK                    (0x15)  /* negative acknowledge */
#define CA                      (0x18)  /* two of these in succession aborts transfer */
#define CRC16                   (0x43)  /* 'C' == 0x43, request 16-bit CRC */

#define ABORT1                  (0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  (0x61)  /* 'a' == 0x61, abort by user */

#define NAK_TIMEOUT             (0x100000)
#define MAX_ERRORS              (5)

//写入FLASH 完成标志
//#define W_SPI_FLASH_OK      5555   //APP 写入SPI FLASH OK
//#define W_MCU_FLASH_OK      6666   //APP 写入MCU FLASH OK

#define W_SPI_FLASH_OK      "5555"   //APP 写入SPI FLASH OK
#define W_MCU_FLASH_OK      "6666"   //APP 写入MCU FLASH OK
#define W_SPI_FLASH_NEED    "5050"   //需要写入SPI FLASH
#define W_MCU_FLASH_NEED    "6060"   //需要写入MCU_FLASH



//User Define Area
#ifndef IAP_PORT
	#define IAP_PORT USART1
#endif 

extern unsigned char FileName[FILE_NAME_LENGTH];


//Exported functions
int32_t Ymodem_Receive (uint8_t *buf,uint32_t appaddr);

#endif 
