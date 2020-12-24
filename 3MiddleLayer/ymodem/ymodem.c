/******************************************************************************

                  版权所有 (C), 2013-2023, 深圳博思高科技有限公司

 ******************************************************************************
  文 件 名   : ymodem.c
  版 本 号   : 初稿
  作    者   : 张舵
  生成日期   : 2019年6月3日
  最近修改   :
  功能描述   : Ymodem协议
            IAP_PORT在ymodem.h中定义            
            IAP_PORT设置:USART1, 115200bps, 8, n, 1
            IAP_PORT需要单独初始化
  函数列表   :
              Receive_Byte
              Receive_Packet
              Send_Byte
              SerialKeyPressed
              Ymodem_Receive
  修改历史   :
  1.日    期   : 2019年6月3日
    作    者   : 张舵
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "ymodem.h"
#include "easyflash.h"

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

uint8_t FileName[FILE_NAME_LENGTH] = {0};
static uint32_t update_file_cur_size;

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
//check if one byte rx successfully
//key: variabl to store rx data
//return: 1=success, 0=fail

static uint32_t SerialKeyPressed ( uint8_t* key )
{
	if ( USART_GetFlagStatus ( IAP_PORT, USART_FLAG_RXNE ) != RESET )
	{
		*key = ( uint8_t ) IAP_PORT->DR;
		return 1;
	}
	else
	{
		return 0;
	}
}


//Rx a byte from sender
//c: variable to store data
//timeout: read time out
//return: 0=success, -1=fail
static  int32_t Receive_Byte ( uint8_t* c, uint32_t timeout )
{
	while ( timeout-- > 0 )
	{
		if ( SerialKeyPressed ( c ) == 1 )
		{
			return 0;
		}
	}
	return -1;
}

//send a byte via IAP_PORT
//c: byte to send
//return: 0
static uint32_t Send_Byte ( uint8_t c )
{
	USART_SendData ( IAP_PORT, c );
	while ( USART_GetFlagStatus ( IAP_PORT, USART_FLAG_TXE ) == RESET );
    //USART_ClearFlag (USART1, USART_FLAG_TXE);    
	return 0;
}


//Rx a packet from sender
//data: pointer to store rx data
//length: packet length
//timeout: rx time out
//return:0=normally return
//      -1=timeout or packet error
//       1=abort by user
static int32_t Receive_Packet ( uint8_t* data, int32_t* length, uint32_t timeout )
{
	uint16_t i, packet_size;
	uint8_t c;

	*length = 0;
	if ( Receive_Byte ( &c, timeout ) != 0 )
	{
		return -1;  //超时返回-1
	}
    
	switch ( c )        //c表示接收到的数据的第一个字节
	{
		case SOH:       //数据包开始
			packet_size = PACKET_SIZE;
			break;
		case STX:       //正文开始
			packet_size = PACKET_1K_SIZE;
			break;
		case EOT:       //数据包结束
			return 0;
		case CA:        //发送方中止传输
			if ( ( Receive_Byte ( &c, timeout ) == 0 ) && ( c == CA ) )
			{
				*length = -1;
				return 0;
			}
			else
			{
				return -1;
			}
		case ABORT1:    //A
		case ABORT2:    //a
			return 1;
		default:
			return -1;
	}
    
	*data = c;
	for ( i = 1; i < ( packet_size + PACKET_OVERHEAD ); i ++ )  //获取剩下的数据（以字节为单位）
	{
		if ( Receive_Byte ( data + i, timeout ) != 0 )  
		{
			return -1;  //接收数据超时
		}
	}
//  if ((uint8_t)data[PACKET_SEQNO_INDEX] != ((uint8_t)(data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff))
//  {
//    return -1;
//  }
	*length = packet_size;

    
    
	return 0;
}


/*****************************************************************************
 函 数 名  : Ymodem_Receive
 功能描述  : 使用ymodem协议接收文件
 输入参数  : uint8_t* buf 数据存储指针     
           uint32_t appaddr  要写入的地址
 输出参数  : 无
 返 回 值  : size 接收文件大小
 
 修改历史      :
  1.日    期   : 2019年6月3日
    作    者   : 张舵

    修改内容   : 新生成函数

*****************************************************************************/
int32_t Ymodem_Receive ( uint8_t* buf, uint32_t appaddr )
{
	uint8_t packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD], *file_ptr, *buf_ptr,flag_EOT;
	int32_t i, packet_length, session_done, file_done, packets_received, errors, session_begin, size = 0;
	char file_size[FILE_SIZE_LENGTH];
	//uint32_t flashdestination, ramsource;   

	//Initialize flashdestination variable
	//flashdestination = appaddr;
	
	flag_EOT = 0;
	for ( session_done = 0, errors = 0, session_begin = 0; ; )
	{
		for ( packets_received = 0, file_done = 0, buf_ptr = buf; ; )
		{
			switch ( Receive_Packet ( packet_data, &packet_length, NAK_TIMEOUT ) )//接收数据包
			{
				case 0:
					errors = 0;
					switch ( packet_length )
					{
						case - 1:   //发送端中止传输
							Send_Byte ( ACK );
							return 0;

						case 0: //接收结束或接收错误
							if ( flag_EOT==0 ) //first EOT
							{
								Send_Byte ( NACK );
								flag_EOT = 1;
							}
							else if ( flag_EOT==1 ) //second EOT
							{
								Send_Byte ( ACK );
								Send_Byte ( 'C' );
								file_done = 1;
							}
							break;
						/* Normal packet */
						default:     //接收数据中
							if ( ( packet_data[PACKET_SEQNO_INDEX] & 0xff ) != ( packets_received & 0xff ) )
							{
                                //接收错误的数据，回复NAK
								Send_Byte ( NACK ); //local data sequence number is different to rx data packet.
							}
							else
							{
								if ( packets_received == 0 ) //接收第一帧数据
								{
									/* Filename packet */
									if ( packet_data[PACKET_HEADER] != 0 ) //包含文件信息：文件名，文件长度等
									{
										/* Filename packet has valid data */
										for ( i = 0, file_ptr = packet_data + PACKET_HEADER; ( *file_ptr != 0 ) && ( i < FILE_NAME_LENGTH ); )
										{
											FileName[i++] = *file_ptr++;    //保存文件名
										}
										FileName[i++] = '\0';   //文件名以'\0'结束
										for ( i = 0, file_ptr ++; ( *file_ptr != ' ' ) && ( i < FILE_SIZE_LENGTH ); )
										{
											file_size[i++] = *file_ptr++;   //保存文件大小
										}
										file_size[i++] = '\0';  //文件大小以'\0'结束
										size = atoi ( file_size );  //将文件大小字符串转换成整型 

										/* Test the size of the image to be sent */
										/* Image size is greater than Flash size */
										if ( size > ( int32_t ) ( USER_FLASH_SIZE + 1 ) )   //升级固件过大
										{
											/* End session */
											Send_Byte ( CA );
											Send_Byte ( CA );   //连续发送2次中止符CA
											return -1;
										}
										/* erase user application area */
//										STM_FLASH_Erase ( appaddr ); //擦除片内FLASH
                                        ef_erase_bak_app( size );   //擦除相应的flash空间

                                        //把文件大小写入FLASH参数区
                                        ef_set_env((const char *)"FileName",(const char *)FileName);
                                        ef_set_env((const char *)"FileSize",(const char *)file_size);
                                        
										Send_Byte ( ACK );      //回复ACk
										Send_Byte ( CRC16 );    //发送'C',询问数据
									}
									/* Filename packet is empty, end session */
									else    //文件名数据包为空，结束传输
									{
										Send_Byte ( ACK );
										file_done = 1;      //停止接收
										session_done = 1;   //结束对话
										break;
									}
								}
								/* Data packet */
								else        //收到数据包
								{
									memcpy ( buf_ptr, packet_data + PACKET_HEADER, packet_length );
                                        
//                                    //ramsource = ( uint32_t ) buf_ptr; //直接写内部FLASH需要，但是外部FLASH不需要

									/* Write received data in Flash */
                                    if(ef_write_data_to_bak(buf_ptr, packet_length,&update_file_cur_size,size) == EF_NO_ERR)                           
//    								if ( STM_FLASH_Write ( &flashdestination, ( uint32_t* ) ramsource, ( uint16_t ) packet_length/4 )  == 0 ) //直接写片内FLASH
									{
										Send_Byte ( ACK );
									}
									else /* An error occurred while writing to Flash memory */
									{
										/* End session */
										Send_Byte ( CA );
										Send_Byte ( CA );
										return -2;
									}
								}
								packets_received ++;    //收到数据包的个数
								session_begin = 1;      //设置接收中标志
							}
					}
					break;
				case 1:     //用户中止
					Send_Byte ( CA );
					Send_Byte ( CA );
					return -3;
				default:
					if ( session_begin > 0 )    //传输过程中发生错误
					{
						errors ++;
					}
					if ( errors > MAX_ERRORS )  //错误超过上限
					{
						Send_Byte ( CA );
						Send_Byte ( CA );
						return 0;   //传输过程发生过多错误
					}
					Send_Byte ( CRC16 );
					break;
			}

            
			if ( file_done != 0 )   //文件接收完毕，退出循环
			{
				break;
			}
		}
        
		if ( session_done != 0 )    //对话结束，跳出循环
		{
			break;
		}
	}
	return ( int32_t ) size;    //返回接收到文件的大小
}
