#pragma once
#include"pj.h"
//保存基本的pjsip模块，包括缓存池，endpoint，线程池等
struct app_t
{
	pj_caching_pool	 cp;
	pj_pool_t		*pool;
	pjsip_endpoint	*sip_endpt;
	pj_bool_t		 quit;
	pj_thread_t		*worker_thread;
	//是否开启log
	pj_bool_t		 enable_msg_logging;
	unsigned		 name_cnt;
	pjsip_host_port	 name[16];
	int				port;
	pj_bool_t record_route;
} app;

extern int worker_proc(void *arg);
extern pj_status_t init_stack();
extern void destroy_stack(void);
extern pj_status_t init_proxy(void);
/*APP*/