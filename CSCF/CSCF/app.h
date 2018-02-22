#pragma once
#include"pj.h"
//保存基本的pjsip模块，包括缓存池，endpoint，线程池等
typedef struct
{
	//sip协议栈需要的基本数据
	pj_caching_pool	 cp;
	pj_pool_t		*pool;
	pjsip_endpoint	*sip_endpt;
	pj_bool_t		 quit;
	pj_thread_t		*worker_thread;
	pj_thread_t		*clean_thread;
	pj_bool_t		 enable_msg_logging;	//是否开启log
	unsigned		 name_cnt;				//本机地址数量（用于转发）
	pjsip_host_port	 name[16];				//本机地址（用于转发）
	int				port;					//本机sip端口
	pj_bool_t record_route;					//是否开启record_route
	//pj_hash_table_t * routing_chart;
	//pj_lock_t* routing_lock;
	//各个消息队列的fd
	int send_id;
	int recv_id;
	int rtp_send_id;
	int rtp_recv_id;
} app_t;
extern app_t app;
extern int worker_proc(void *arg);
extern pj_status_t init_stack();
extern void destroy_stack(void);
extern pj_status_t init_proxy(void);
/*APP*/