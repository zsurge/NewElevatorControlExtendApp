#ifndef __RTC_H__
#define __RTC_H__

#include "stm32f4xx.h"


// 时钟源宏定义
//#define RTC_CLOCK_SOURCE_LSE      
#define RTC_CLOCK_SOURCE_LSI

// 异步分频因子
#define ASYNCHPREDIV         0X7F
// 同步分频因子
#define SYNCHPREDIV          0XFF



// 时间宏定义
#define RTC_H12_AMorPM			 RTC_H12_AM  
//默认时间为 2020年1月8号早上8：30：30
#define RTC_DEFAULT_TIEM        "2020-01-08 08:30:00"

//#define HOURS                12          // 0~23
//#define MINUTES              00          // 0~59
//#define SECONDS              00          // 0~59

//  //日期宏定义
//#define WEEKDAY              5          // 1~7
//#define DATE                 03         // 1~31
//#define MONTH                01         // 1~12
//#define YEAR                 20         // 0~99

// 时间格式宏定义
#define RTC_Format_BINorBCD  RTC_Format_BIN

// 备份域寄存器宏定义
#define RTC_BKP_DRX          RTC_BKP_DR0
// 写入到备份寄存器的数据宏定义
#define RTC_BKP_DATA         0X32F2
 

void bsp_rtc_init(void);

void RTC_CLK_Config(void);
void RTC_TimeAndDate_Set(char *defalutTime);
void RTC_TimeAndDate_Show(void);

uint8_t *GetLocalTime(void);


#endif // __RTC_H__
