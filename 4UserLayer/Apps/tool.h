/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : tool.h
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年6月20日
  最近修改   :
  功能描述   : 工具类
  函数列表   :
  修改历史   :
  1.日    期   : 2019年6月20日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/
#ifndef __TOOL_H
#define __TOOL_H
/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "stdio.h"
#include "stdlib.h" 
#include <stdint.h>



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
 
#define DEBUG_PRINT


#define _myAssert(char,int) printf("Error:%s,%d\r\n",char,int)
#define MyAssert(x) if((x)==0) _myASSERT(__FILE__,__LINE__)



#define SETBIT(x,y) x|=(1<<y)         //将X的第Y位置1    
#define CLRBIT(x,y) x&=~(1<<y)        //将X的第Y位清0

#define DBG(...) dbg(__FILE__, __LINE__, __VA_ARGS__)

typedef struct {
    char **str;     //the PChar of string array
    size_t num;     //the number of string
}IString;



/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
uint16_t CRC16_Modbus(uint8_t *pBuf, uint16_t usLen);

uint8_t xorCRC(uint8_t *buf,uint8_t len);

void asc2bcd(unsigned char *bcd_buf, unsigned char *ascii_buf, int conv_len, unsigned char type);
void bcd2asc(unsigned char *ascii_buf,unsigned char * bcd_buf, int conv_len, unsigned char type);

void dbg(const char *file, const long line, const char *format, ...);


void dbh(char *title,char *buf,int len);

uint8_t is_bit_set(uint16_t      value, uint16_t bit);

void int2Str(uint8_t* str, int32_t intnum);
int32_t str2int(const char* str);
uint8_t bcd2byte(uint8_t ucBcd);
void HexToStr(uint8_t *pbDest, uint8_t *pbSrc, int nLen);
uint8_t BCDToInt ( unsigned char bcd );//BCD转十进制
uint8_t IntToBCD ( unsigned char intdat );//十进制转BCD

void split(char *src,const char *separator,char **dest,int *num);
char *strstr_t(const char *str, const char *sub_str);
int setbit(int num, int bit);
int clearbit(int num, int bit);

void calcMac(unsigned char *mac);

void Insertchar(char *src,char*desc,char c);


uint8_t compareArray(uint8_t *array1,uint8_t *array2,int len);




#endif
