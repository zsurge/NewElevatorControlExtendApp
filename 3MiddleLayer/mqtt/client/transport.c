#include "transport.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "string.h"
#include <netdb.h>


static int mysock;

/************************************************************************
** 函数名称: transport_sendPacketBuffer									
** 函数功能: 以TCP方式发送数据
** 入口参数: unsigned char* buf：数据缓冲区
**           int buflen：数据长度
** 出口参数: <0发送数据失败							
************************************************************************/
s32 transport_sendPacketBuffer( u8* buf, s32 buflen)
{
	s32 rc;
	rc = write(mysock, buf, buflen);
	return rc;
}

/************************************************************************
** 函数名称: transport_getdata									
** 函数功能: 以阻塞的方式接收TCP数据
** 入口参数: unsigned char* buf：数据缓冲区
**           int count：数据长度
** 出口参数: <=0接收数据失败									
************************************************************************/
s32 transport_getdata(u8* buf, s32 count)
{
	s32 rc;
	//这个函数在这里不阻塞
  rc = recv(mysock, buf, count, 0);
	return rc;
}


/************************************************************************
** 函数名称: transport_open									
** 函数功能: 打开一个接口，并且和服务器 建立连接
** 入口参数: char* servip:服务器域名
**           int   port:端口号
** 出口参数: <0打开连接失败										
************************************************************************/
s32 transport_open(s8* servip, s32 port)
{

    int32_t* sock = &mysock;
    struct hostent *server;
    struct sockaddr_in serv_addr;

    *sock = socket(AF_INET, SOCK_STREAM, 0);
    if(*sock < 0)
        printf("[ERROR] Create socket failed\n");

    server = gethostbyname(servip);
    if(server == NULL)
        printf("[ERROR] Get host ip failed\n");

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);
    if(connect(*sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    {
        //关闭链接
        close(*sock);        
        printf("[ERROR] connect failed\n");
        return -1;
    }

    //返回套接字
    return *sock;

//    int* sock = &mysock;
//	struct hostent *server;
//	struct sockaddr_in serv_addr;
//	static struct  timeval tv;
//	int timeout = 1000;
//	fd_set readset;
//	fd_set writeset;
//	*sock = socket(AF_INET, SOCK_STREAM, 0);
//	if(*sock < 0)
//		printf("[ERROR] Create socket failed\n");
//	
//	server = gethostbyname(servip);
//	if(server == NULL)
//		printf("[ERROR] Get host ip failed\n");
//	
//	memset(&serv_addr,0,sizeof(serv_addr));
//	serv_addr.sin_family = AF_INET;
//	serv_addr.sin_port = htons(port);
//	memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);
//	if(connect(*sock,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
//	{
//		//关闭链接
//		close(*sock);        
//		printf("[ERROR] connect failed\n");
//        return -1;
//	}
//	tv.tv_sec = 10;  /* 1 second Timeout */
//	tv.tv_usec = 0; 
//	setsockopt(*sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout,sizeof(timeout));
//    
//    return *sock;    
}


/************************************************************************
** 函数名称: transport_close									
** 函数功能: 关闭套接字
** 入口参数: unsigned char* buf：数据缓冲区
**           int buflen：数据长度
** 出口参数: <0发送数据失败							
************************************************************************/
int transport_close(void)
{
	int rc;
	rc = close(mysock);
	return rc;
}
