/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : calcDevNO.c
  版 本 号   : 初稿
  作    者   :  
  生成日期   : 2020年4月8日
  最近修改   :
  功能描述   : 计算设备数，做为判断二维码权限的依据
  函数列表   :
  修改历史   :
  1.日    期   : 2020年4月8日
    作    者   :  
    修改内容   : 创建文件

******************************************************************************/
/******************************************************************************
90进制,使用以下字符
"!", "#", "$", "%", "&", "(", ")", "*", "+", ",", 
"-", ".", "/", "0", "1", "2", "3", "4", "5", "6", 
"7", "8", "9", ":", ";", "<", "=", ">", "?", "@", 
"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", 
"K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", 
"U", "V", "W", "X", "Y", "Z", "[", "]", "^", "_", 
"`", "a", "b", "c", "d", "e", "f", "g", "h", "i", 
"j", "k", "l", "m", "n", "o", "p", "q", "r", "s", 
"t", "u", "v", "w", "x", "y", "z", "{", "}", "~"

保留字符: |"'\

例子：
10进制 0     = 90进制  !
10进制 89    = 90进制  ~
10进制 90    = 90进制  #!
10进制 8099  = 90进制  ~~
10进制 8100  = 90进制  #!!
10进制 18883 = 90进制  $@m
******************************************************************************/



/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include<stdio.h>
#include<string.h>
#include "calcDevNO.h"
#include "deviceinfo.h"

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define DEVICE_NO_FMT       90
/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void StrReversal(char *str);
static int findChar(const char acher);


static void StrReversal(char *str)
{
    char tmp;
    int len = strlen(str);
    for(int i=0,j=len-1;i<=j;i++,j--)
    {
        tmp=str[i];
        str[i]=str[j];
        str[j]=tmp;
    }
    str[len] = '\0';
}

//转为字符串
void myItoa(char *str,int n,int radix)
{
    char chars[] = {'!','#','$','%','&','(',')','*','+',',','-','.','/','0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[',']','^','_','`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','{','}','~'};
    int i=0;

    if(n == 0)
    {
        str[0] = chars[0];
    }
    else
    {
        while(n!=0)
        {
            str[i++]=chars[n%radix];
            n/=radix;
        }
    }
    StrReversal(str);
}

int findChar(const char acher)
{
    int i = 0;
    char chars[] = {'!','#','$','%','&','(',')','*','+',',','-','.','/','0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[',']','^','_','`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','{','}','~'};
    for (i = 0; i < strlen(chars);i++)
    {
        if(acher == chars[i])
            return i;
    }

    return -1;
}

//90进制转换为10进制
int myAtoi(const char *str)
{
    int total = 0;          //保存转换后的数值    
    int length = 0;         //记录字符串的长度
    //char chars[] = {'!','#','$','%','&','(',')','*','+',',','-','.','/','0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?','@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','[',']','^','_','`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','{','}','~'};
    const char *p = str;

    char temp = '!';

    if(NULL == p)           //判断指针的合法性
    {
        printf("error");
        return -1;
    }
    
    while(*p++!='\0')       //计算字符串的长度
    {
        length++;
    }

    p = str;            //重新指向字符串的首地址


    
    for(int i = 0;i < length;i++)
    {        
        temp = findChar(*p++);

        //if(total != 0)     //滤除字符串开始的0字符
        if(total != 0||temp != -1)     //滤除字符串开始的0字符
        {
            //temp -= '0';
            total = total*DEVICE_NO_FMT + temp;
        }
    }

    return total;               //返回转换后的数值
}


unsigned char findDev(unsigned char *str,unsigned char dataFormat)
{
    unsigned char tmpBuff[9] = {0};
    unsigned char isFind = 0;
    int len = strlen((const char*)str);
    int i = 0;
    int devSN = 0;

    int tmpValue = 0;

    memset(tmpBuff,0x00,sizeof(tmpBuff));
    
    memcpy(tmpBuff,gDevBaseParam.deviceCode.qrSn,gDevBaseParam.deviceCode.qrSnLen);


    devSN = atoi((const char*)tmpBuff);

    char buf[6] = {0};

    for(i=0;i<len/dataFormat;i++)
    {
        memset(buf,0x00,sizeof(buf));
        memcpy(buf,str+i*dataFormat,dataFormat);
        tmpValue = myAtoi(buf);
        printf("current devSn = %d,format(90) = %s,decimal = %d\r\n",devSN,buf,tmpValue);
        if(devSN == tmpValue)
        {
            isFind = 1;
        }
    }


    return isFind;
}



char *parseAccessFloor(char *str)
{
    static char result[64] = {0};;
    char buf[1] = {0};
    int len = strlen((const char*)str);
    int i = 0;

    if(len >64)
    {
        len = 64;
    }

    for(i=0;i<len;i++)
    {
        memset(buf,0x00,sizeof(buf));
        memcpy(buf,str+i,1);
        result[i] = myAtoi(buf); 
        printf("%d ",result[i]);
    }

    
    printf("\r\n");
    
    return result;    
}



