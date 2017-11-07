#pragma once
#include"pj.h"
struct app_t
{
	pj_caching_pool	 cp;
	pj_pool_t		*pool;
	pjsip_endpoint	*sip_endpt;
	pj_bool_t		 quit;
	pj_thread_t		*worker_thread;
	pj_thread_desc  worker_thread_desc;
	pj_bool_t		 enable_msg_logging;
} app;

extern int worker_proc(void *arg);
extern pj_status_t init_config();
extern pj_status_t init_stack();
extern void destroy_stack(void);