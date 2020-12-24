#include "bsp_ds1302.h"
#include "tool.h"
#include "time.h"
#include "string.h"

static uint8_t WeekDay ( unsigned char y, unsigned char m, unsigned char d );



const uint8_t read[7] = {0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};//读秒、分、时、日、月、周、年的寄存器地址
const uint8_t write[7] = {0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};//写秒、分、时、日、月、周、年的寄存器地址

void write_1302byte ( uint8_t dat ) //写一个字节的数据sck上升沿写数据
{
	uint8_t i=0;
	GPIO_ResetBits ( DS1302_PORT,DS1302CLK );
	//ds1302clk=0;
	delay_us ( 2 ); //延时大约2us
	for ( i=0; i<8; i++ )
	{
		GPIO_ResetBits ( DS1302_PORT,DS1302CLK );
		//ds1302clk=0;
		if ( dat&0x01 )
		{
			GPIO_SetBits ( DS1302_PORT,DS1302DAT );
		}
		else
		{
			GPIO_ResetBits ( DS1302_PORT,DS1302DAT );
		}
		//ds1302dat=(dat&0x01);
		delay_us ( 2 );
		GPIO_SetBits ( DS1302_PORT,DS1302CLK );
		//ds1302clk=1;
		dat>>=1;
		delay_us ( 1 );
	}
}

uint8_t read_1302 ( uint8_t add ) //读数据
{
	uint8_t i=0,dat1=0x00;
	GPIO_ResetBits ( DS1302_PORT,DS1302RST );
	GPIO_ResetBits ( DS1302_PORT,DS1302CLK );
	//ds1302rst=0;
	//ds1302clk=0;
	delay_us ( 3 ); //略微延时2us
	GPIO_SetBits ( DS1302_PORT,DS1302RST );
	//ds1302rst=1;
	delay_us ( 3 ); //时间要大约3us
	write_1302byte ( add ); //先写寄存器的地址
	for ( i=0; i<8; i++ )
	{
		GPIO_SetBits ( DS1302_PORT,DS1302CLK );
		//ds1302clk=1;
		delay_us ( 5 );
		dat1>>=1;
		delay_us ( 5 );
		GPIO_ResetBits ( DS1302_PORT,DS1302CLK );
		//ds1302clk=0;//拉低时钟线，以便于数据的读入
		if ( GPIO_ReadInputDataBit ( DS1302_PORT,DS1302DAT ) ==1 ) //数据线此时为高电平
		{
			dat1=dat1|0x80;
		}
	}
	delay_us ( 1 );
	GPIO_ResetBits ( DS1302_PORT,DS1302RST );
	//ds1302rst=0;GPIOF
	return dat1;
}

void write_1302 ( uint8_t add,uint8_t dat ) //向指定寄存器写入一个字节的数据
{
	GPIO_ResetBits ( DS1302_PORT,DS1302RST );
	GPIO_ResetBits ( DS1302_PORT,DS1302CLK );
	//ds1302rst=0;
	//ds1302clk=0;
	delay_us ( 1 ); //略微延时
	GPIO_SetBits ( DS1302_PORT,DS1302RST );
	//ds1302rst=1;
	delay_us ( 2 ); //时间大约2us
	write_1302byte ( add );
	write_1302byte ( dat );
	GPIO_ResetBits ( DS1302_PORT,DS1302RST );
	GPIO_ResetBits ( DS1302_PORT,DS1302CLK );
	//ds1302clk=0;
	//ds1302rst=0;
	delay_us ( 5 );

}

///*************************************************************************************************************************
//*函数         :
//*功能         :     2000年~2099年星期算法
//*参数         :
//*返回         : 无
//*依赖 : 底层宏定义
//* 作者 :
//* 时间 :
//* 最后修改时间:
//*说明         :
//*************************************************************************************************************************/
static uint8_t WeekDay ( unsigned char y, unsigned char m, unsigned char d )
{
	unsigned char A;
	if ( m==1||m==2 )
	{
		m+=12;
		y--;
	}
	A= ( d+2*m+3* ( m+1 ) /5+y+y/4-y/100+y/400+1 ) %7;
	return A;

}



void bsp_ds1302_mdifytime ( uint8_t* descTime ) //初始化1302
{
	uint8_t i=0;
	uint8_t tmp[8] = {0};
	uint8_t buf[8] = {0};

	// 初始化日期
	memset ( tmp,0x00,sizeof ( tmp ) );
	memcpy ( tmp,descTime+2,2 );
	buf[6] = IntToBCD ( atoi ( (const char*)tmp ) );
    
	memset ( tmp,0x00,sizeof ( tmp ) );
	memcpy ( tmp,descTime+5,2 );
	buf[4] = IntToBCD ( atoi ( (const char*)tmp ) );

	memset ( tmp,0x00,sizeof ( tmp ) );
	memcpy ( tmp,descTime+8,2 );
	buf[3] = IntToBCD ( atoi ( (const char*)tmp ) );

	//周
	buf[5] = WeekDay(BCDToInt(buf[6]),BCDToInt(buf[4]),BCDToInt(buf[3]));    

	memset ( tmp,0x00,sizeof ( tmp ) );
	memcpy ( tmp,descTime+11,2 );
	buf[2] = IntToBCD ( atoi ( (const char*)tmp ) );

	memset ( tmp,0x00,sizeof ( tmp ) );
	memcpy ( tmp,descTime+14,2 );
	buf[1] = IntToBCD ( atoi ( (const char*)tmp ) );

	memset ( tmp,0x00,sizeof ( tmp ) );
	memcpy ( tmp,descTime+17,2 );
	buf[0] = IntToBCD ( atoi ( (const char*)tmp ) );

//	dbh ( "bsp_ds1302_mdifytime", (char *)buf, 8 );

	write_1302 ( 0x8e,0x00 ); //去除写保护
	for ( i=0; i<7; i++ ) //进行对时
	{
		write_1302 ( write[i],buf[i] );
	}

	write_1302 ( 0x8e,0x80 ); //加写保护

}

char* bsp_ds1302_readtime ( void )
{
	static char pBuf[20] = {0};
    
	pBuf[0]=read_1302 ( read[0] ); //秒
	pBuf[1]=read_1302 ( read[1] ); //分
	pBuf[2]=read_1302 ( read[2] ); //时
	pBuf[4]=read_1302 ( read[3] ); //日
	pBuf[5]=read_1302 ( read[4] ); //月
	pBuf[3]=read_1302 ( read[5] ); //周
	pBuf[6]=read_1302 ( read[6] ); //年

    sprintf(pBuf,"20%02x-%02x-%02x %02x:%02x:%02x",pBuf[6],pBuf[5],pBuf[4],pBuf[2],pBuf[1],pBuf[0]);    


	return pBuf;
}

void bsp_ds1302_init ( void )
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd ( RCC_AHB1Periph_GPIOE, ENABLE ); //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = DS1302CLK|DS1302RST;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; //IO口速度为50MHz
	GPIO_Init ( DS1302_PORT, &GPIO_InitStructure ); //根据设定参数初始化GPIOB.8


	GPIO_ResetBits ( DS1302_PORT, DS1302CLK );
	GPIO_ResetBits ( DS1302_PORT, DS1302RST );


	GPIO_InitStructure.GPIO_Pin = DS1302DAT;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;//开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init ( DS1302_PORT, &GPIO_InitStructure );
	GPIO_ResetBits ( DS1302_PORT, DS1302DAT );
}



char* time_to_timestamp(void)
{
    struct tm stm;  
    static char ret[10] = {0};  
    memset(&stm,0,sizeof(stm)); 
    
    stm.tm_year = BCDToInt(read_1302 ( read[6] ))+100;  
    stm.tm_mon = BCDToInt(read_1302 ( read[4] ))-1;  
    stm.tm_mday = BCDToInt(read_1302 ( read[3] ));  
    stm.tm_hour = BCDToInt(read_1302 ( read[2] ))-8;  
    stm.tm_min = BCDToInt(read_1302 ( read[1] ));  
    stm.tm_sec = BCDToInt(read_1302 ( read[0] ));  
    
    sprintf(ret,"%d",mktime(&stm));
//    printf("time_to_timestamp = %s\r\n",ret);
    return ret;
}

void timestamp_to_time(unsigned int timestamp)
{
	struct tm *stm= NULL;
    char buf[32] = {0};
    time_t seconds = timestamp;

//	printf("timestamp = %d\r\n",timestamp);
	stm = localtime(&seconds);
	
	sprintf(buf,"%04d-%02d-%02d  %02d:%02d:%02d",\
	stm->tm_year+1900,stm->tm_mon+1,stm->tm_mday,\
	stm->tm_hour+8,stm->tm_min,stm->tm_sec);

//    printf("timestamp_to_time = %s\r\n",buf);
}

