#pragma once
//宏定义的设置

#define THIS_FILE   "VOWIFI_CSCF"	//本文件的文件名，用于输出LOG，暂时统一定义为"VOWIFI_CSCF"
#define PJ_SERVER_TEST				//运行于linux时需要定义本宏定义
									//由于windows下的本程序已经被废弃，请务必定义该宏定义

//各个消息队列的key
#define CSCF_SEND 1025
#define CSCF_RECV 1024 
#define RTP_SEND 1983
#define RTP_RECV 1984

#ifdef PJ_SERVER_TEST
	#define PJ_SERVER_ADDRESS "59.110.217.175"	//本机外网地址
	#define PJ_SERVER_PORT 9170;				//网络端口
#else
	#define PJ_SERVER_PORT 5060;
#endif // DEBUG

