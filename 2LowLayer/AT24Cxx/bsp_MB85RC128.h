/**
  ******************************************************************************
  *               Copyright(C) 2016-2026 GDKY All Rights Reserved
  *
  * @file     fm24clxx.h
  * @author   ZouZH
  * @version  V1.08
  * @date     17-March-2016
  * @brief    RAMTRON FRAM Serial Memory Driver.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __FM24CLXX_H
#define __FM24CLXX_H

#ifdef __cplusplus
 extern "C" {
#endif  /* __cplusplus */

/* INCLUDES ------------------------------------------------------------------- */
#include <stdint.h>
#include "stm32f4xx.h" 

/* TYPEDEFS ------------------------------------------------------------------- */

typedef enum
{
  FM24CL04_1    = 1,    /* 4Kb(512B)  */
  FM24CL64_1    = 2,    /* 64Kb(8KB)  */
  FM24CL64_2    = 3,    /* 64Kb(8KB)  */
  FM24W256_1    = 4,    /* 256Kb(32KB)  */
  FM24W256_2    = 5,    /* 256Kb(32KB)  */
  FM24V10_1     = 6,    /* 1Mb(128KB) */
  FM24V10_2     = 7,
  FM24V10_3     = 8,
  FM24V10_4     = 9,
}FRAMDev_t;

typedef enum
{
  FRAM_OK = 0,
  FRAM_WRITE,
  FRAM_READ,
  FRAM_TMOUT,
  FRAM_MUTEX,

}FRAMErr_t;

/* MACROS  -------------------------------------------------------------------- */

/* CONSTANTS  ----------------------------------------------------------------- */

#define FM24CL04_MAX_ADDR    ((uint32_t)0x1FFul)
#define FM24CL64_MAX_ADDR    ((uint32_t)0x1FFFul)
#define FM24W256_MAX_ADDR    ((uint32_t)0x7FFFul)
#define FM24V10_MAX_ADDR     ((uint32_t)0x1FFFFul)

#define FRAM_WRITE_ADDR      ((uint8_t)0xA0u)
#define FRAM_READ_ADDR       ((uint8_t)0xA1u)


/* GLOBAL VARIABLES ----------------------------------------------------------- */

/* GLOBAL FUNCTIONS ----------------------------------------------------------- */

void    FRAM_Init(void);
uint8_t FRAM_Write(FRAMDev_t FM24xx, uint32_t addr, const void *pvbuf, uint32_t size);
uint8_t FRAM_Read (FRAMDev_t FM24xx, uint32_t addr, void *pvbuf, uint32_t size);
uint8_t FRAM_Erase(FRAMDev_t FM24xx, uint32_t startAddr, uint32_t numBytes);

//void farm_test(void);

//void farm_read(void);

//void eeeeee(void);

/* LOCAL VARIABLES ------------------------------------------------------------ */

/* LOCAL FUNCTIONS ------------------------------------------------------------ */


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* __FM24CLXX_H */

/***************************** END OF FILE *************************************/



