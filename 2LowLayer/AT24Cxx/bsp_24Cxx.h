/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : bsp_24Cxx.h
  版 本 号   : 初稿
  作    者   : armfly
  生成日期   : 2020年1月16日
  最近修改   :
  功能描述   : bsp_24Cxx.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2020年1月16日
    作    者   : armfly
    修改内容   : 创建文件

******************************************************************************/
#ifndef __BSP_24CXX_H__
#define __BSP_24CXX_H__

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "stm32f4xx.h" 

/* #define AT24C02 */
/* #define AT24C04 */
// #define AT24C08 
/* #define AT24C16 */
/* #define AT24C32 */
/* #define AT24C64 */

#define MB85RC1MT 


#ifdef AT24C02
	#define EE_MODEL_NAME		"AT24C02"
	#define EE_DEV_ADDR			0xA0			/* 设备地址 */
	#define EE_PAGE_SIZE		8					/* 页面大小(字节) */
	#define EE_SIZE				  256				/* 总容量(字节) */
	#define EE_ADDR_BYTES		1					/* 地址字节个数 */
	#define EE_ADDR_A8			0					/* 地址字节的高8bit不在首字节 */
#endif

#ifdef AT24C04
	#define EE_MODEL_NAME		"AT24C04"
	#define EE_DEV_ADDR			0xA0			/* 设备地址 */
	#define EE_PAGE_SIZE		16				/* 页面大小(字节) */
	#define EE_SIZE				  512				/* 总容量(字节) */
	#define EE_ADDR_BYTES		1					/* 地址字节个数 */
	#define EE_ADDR_A8			1					/* 地址字节的高8bit在首字节 */
#endif

#ifdef AT24C08
	#define EE_MODEL_NAME		"AT24C08"
	#define EE_DEV_ADDR			0xA0	    /* 设备地址 */
	#define EE_PAGE_SIZE		16		   	/* 页面大小(字节) */
	#define EE_SIZE				(1*1024)	/* 总容量(字节) */
	#define EE_ADDR_BYTES		1		    	/* 地址字节个数 */
	#define EE_ADDR_A8			1			    /* 地址字节的高8bit在首字节 */
#endif

#ifdef AT24C16
	#define EE_MODEL_NAME		"AT24C16"
	#define EE_DEV_ADDR			0xA0	    /* 设备地址 */
	#define EE_PAGE_SIZE		16		   	/* 页面大小(字节) */
	#define EE_SIZE				(2*1024) 	/* 总容量(字节) */
	#define EE_ADDR_BYTES		1			    /* 地址字节个数 */
	#define EE_ADDR_A8			1			    /* 地址字节的高8bit在首字节 */
#endif

#ifdef AT24C32
	#define EE_MODEL_NAME		"AT24C32"
	#define EE_DEV_ADDR			0xA0	    /* 设备地址 */
	#define EE_PAGE_SIZE		32		   	/* 页面大小(字节) */
	#define EE_SIZE				  (4*1024)  /* 总容量(字节) */
	#define EE_ADDR_BYTES		2			    /* 地址字节个数 */
	#define EE_ADDR_A8			0			    /* 地址字节的高8bit在首字节 */
#endif

#ifdef AT24C64
	#define EE_MODEL_NAME		"AT24C64"
	#define EE_DEV_ADDR			0xA0	    /* 设备地址 */
	#define EE_PAGE_SIZE		32			  /* 页面大小(字节) */
	#define EE_SIZE				  (8*1024)	/* 总容量(字节) */
	#define EE_ADDR_BYTES		2			    /* 地址字节个数 */
	#define EE_ADDR_A8			0			    /* 地址字节的高8bit在首字节 */
#endif

#ifdef AT24C128
	#define EE_MODEL_NAME		"AT24C128"
	#define EE_DEV_ADDR			0xA0	  	/* 设备地址 */
	#define EE_PAGE_SIZE		64			  /* 页面大小(字节) */
	#define EE_SIZE				(16*1024)	/* 总容量(字节) */
	#define EE_ADDR_BYTES		2			    /* 地址字节个数 */
	#define EE_ADDR_A8			0			    /* 地址字节的高8bit不在首字节 */
#endif

#ifdef MB85RC1MT
	#define EE_MODEL_NAME		"MB85RC1MT"
	#define EE_DEV_ADDR			0xA0	  	/* 设备地址 */
	#define EE_PAGE_SIZE		256			  /* 页面大小(字节) */
	#define EE_SIZE				(1024*1024)	/* 总容量(字节) */
	#define EE_ADDR_BYTES		2			    /* 地址字节个数 */
	#define EE_ADDR_A8			0			    /* 地址字节的高8bit不在首字节 */
#endif




/* Exported macro ------------------------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
/* EEPROM检测    主函数调用 */
uint8_t ee_CheckOk(void);
/* EEPROM连续读  主函数调用 */
uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize);
/* EEPROM连续写  主函数调用 */
uint8_t ee_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize);

#endif /* __BSP_24CXX_H__ */
