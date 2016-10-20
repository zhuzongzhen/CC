/******************************************************************
** 文件名:	infor_out.c
** Copyright (c) 2012-2014 *********公司技术开发部
** 创建人:	Austzhu
** 日　期:	2016.10
** 修改人:
** 日　期:
** 描　述:	16  进制打印
** ERROR_CODE:
**
** 版　本:	V1.0
*******************************************************************/

#ifndef __ETHER_H__
#define __ETHER_H__
#include "include.h"

typedef struct ethernet_t{
	int ether_sock;					//网络连接的socket
	int ether_recvlen;				//缓存数据的长度
	int ether_recvhead;			//缓存数据中的读指针偏移量
	u8 *ether_recvbuf;				//数据的缓存区
	pthread_mutex_t ether_lock;
	void *parent;

	int (*ether_init)(struct ethernet_t*, void *parent);
	int (*ether_connect)(struct ethernet_t*);
	int (*ether_logon)(struct ethernet_t*);
	int (*ether_packagecheck)(void *package,int size);
	int (*ether_send)(struct ethernet_t*,u8 *buffer,int size);
	int (*ether_heartbeat)(struct ethernet_t*);
	int (*ether_getchar)(struct ethernet_t*,u8*);
	int (*ether_recv)(struct ethernet_t*,u8*,int);
	void (*ether_relese)(struct ethernet_t*);
	void (*ether_close)(struct ethernet_t*);
} ethernet_t;

extern ethernet_t ethernet;








#if 0

#include "link_method.h"
#include "infor_out.h"
#include "cc_param.h"
#include "loadfile.h"
#include "gprs_ppp.h"


extern int sock_ether;

SINT Ether_Disconnect(void);				//ok-20120316	socket_comm_line_stat MODIFY OK
SINT Ether_Connect(void);				//ok-20120316  socket_comm_line_stat MODIFY OK
SINT Ether_Logon(void);				//OK-20120317	 cc_register_stat
SINT Ether_HeartBeat(void);				//OK-20120317	 cc_register_stat
SINT Ether_Linestat(void);				//socket_comm_line_stat MODIFY OK
SINT Ether_RawSend(UCHAR *buf, int len);		//ok-20120316 socket_comm_line_stat
SINT Ether_Getchar(UCHAR *buf);			//ok-20120316  socket_comm_line_stat
SINT Ether_Keepalive(void);



#define CLOSE_SOCKET(sock)   do{ if((sock) >= 0) { close(sock); sock = -1; }}while(0)

#define CONNECT_WAIT_SECOND 	5
#define SEND_BUF_EMPTY_TIMES 	20
#define SEND_WHEN_EMPTY_TIME 	10
#define SOCKET_SET_OPT_TIMEOUT 	30

#endif

#endif
