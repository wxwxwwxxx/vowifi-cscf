#include"cscf.h"
#include "pj.h"
#include <sys/types.h>   
#include <sys/ipc.h>   
#include <sys/msg.h>   
//utf8
int worker_proc(void *arg)
{
	PJ_UNUSED_ARG(arg);
	while (!app.quit) 
	{
		pj_time_val interval = { 0, 20 };
		pjsip_endpt_handle_events(app.sip_endpt, &interval);
	}
	return 0;
}
int clean_proc(void *arg)
{
	PJ_UNUSED_ARG(arg);
	int i = 0;
	pj_hash_iterator_t it;
	pj_hash_iterator_t* itptr;
	while (!app.quit)
	{
		pj_thread_sleep(10000);
		i++;
		if (i == 30)
		{
			pj_lock_acquire(app.routing_lock);
			for (itptr = pj_hash_first(app.routing_chart, &it); itptr != NULL; itptr = pj_hash_next(app.routing_chart, itptr))
			{
				struct IPC_userinfo* ui = pj_hash_this(app.routing_chart, itptr);
				if (ui->valid)
				{
					ui->expires -= 300;
					if (ui->expires < -300)
					{
						ui->valid = 0;
						PJ_LOG(3,(THIS_FILE,"CLEAN:%s",pj_strdup4(app.pool,&ui->user)));
					}
				}
			}
			pj_lock_release(app.routing_lock);
			i = 0;
		}
	}
	return 0;
}
pj_status_t init_stack()
{
	pj_init();
	init_config();
	pj_sockaddr addr;
	pj_status_t status;
	pj_log_set_level(3);
	app.send_id = msgget(CSCF_SEND, IPC_CREAT);
	app.recv_id = msgget(CSCF_RECV, IPC_CREAT);
	status = pjlib_util_init();
	CHECK_STATUS();
	pj_caching_pool_init(&app.cp, NULL, 0);
	app.pool = pj_pool_create(&app.cp.factory, "CSCF", 512, 512, 0);
	status = pjsip_endpt_create(&app.cp.factory, NULL, &app.sip_endpt);
	CHECK_STATUS();
	pj_sockaddr_init((pj_uint16_t)network_config.sip_af, &addr, NULL, (pj_uint16_t)network_config.sip_port);
	if (network_config.sip_af == pj_AF_INET())
	{
		if (network_config.sip_tcp)
		{
			status = pjsip_tcp_transport_start(app.sip_endpt, &addr.ipv4, 1,NULL);
		}
		else
		{
			status = pjsip_udp_transport_start(app.sip_endpt, &addr.ipv4,NULL, 1, NULL);
		}
	}
	else if (network_config.sip_af == pj_AF_INET6())
	{
		status = pjsip_udp_transport_start6(app.sip_endpt, &addr.ipv6,NULL, 1, NULL);
	}
	else 
	{
		status = PJ_EAFNOTSUP;
	}
	status = init_proxy();
	CHECK_STATUS();
	status=register_module();
	CHECK_STATUS();
	pj_thread_create(app.pool, "CSCF", &worker_proc, NULL, 0, 0,&app.worker_thread);
	CHECK_STATUS();
//	app.routing_chart = pj_hash_create(app.pool, 20);
//	pj_lock_create_recursive_mutex(app.pool, "lock", &app.routing_lock);
//	pj_thread_create(app.pool, "Clean", &clean_proc, NULL, 0, 0, &app.clean_thread);
	CHECK_STATUS();
	return PJ_SUCCESS;
}
void destroy_stack(void)
{
	enum { WAIT_CLEAR = 5000, WAIT_INTERVAL = 500 };

	PJ_LOG(3, (THIS_FILE, "Shutting down.."));


	app.quit = PJ_TRUE;
	if (app.worker_thread) {
		pj_thread_join(app.worker_thread);
		app.worker_thread = NULL;
	}
	if (app.sip_endpt)
		pjsip_endpt_destroy(app.sip_endpt);
	if (app.pool)
		pj_pool_release(app.pool);
	pj_caching_pool_destroy(&app.cp);
}
pj_status_t init_proxy(void)
{
	pj_sockaddr pri_addr;
	pj_sockaddr addr_list[16];
	unsigned addr_cnt = PJ_ARRAY_SIZE(addr_list);
	unsigned i;

	/* List all names matching local endpoint.
	* Note that PJLIB version 0.6 and newer has a function to
	* enumerate local IP interface (pj_enum_ip_interface()), so
	* by using it would be possible to list all IP interfaces in
	* this host.
	*/

	/* The first address is important since this would be the one
	* to be added in Record-Route.
	*/
#ifdef PJ_SERVER_TEST
	app.name[app.name_cnt].host = pj_str(PJ_SERVER_ADDRESS);
	app.name[app.name_cnt].port = app.port;
	app.name_cnt++;
#endif	
	if (pj_gethostip(pj_AF_INET(), &pri_addr) == PJ_SUCCESS) {
		char addr[PJ_INET_ADDRSTRLEN];
		pj_inet_ntop(pj_AF_INET(), &pri_addr.ipv4.sin_addr, addr,
			sizeof(addr));
		pj_strdup2(app.pool, &app.name[app.name_cnt].host, addr);
		app.name[app.name_cnt].port = app.port;
		app.name_cnt++;
	}

	/* Get the rest of IP interfaces */
	if (pj_enum_ip_interface(pj_AF_INET(), &addr_cnt, addr_list) == PJ_SUCCESS)
	{
		for (i = 0; i<addr_cnt; ++i) {
			char addr[PJ_INET_ADDRSTRLEN];

			if (addr_list[i].ipv4.sin_addr.s_addr == pri_addr.ipv4.sin_addr.s_addr)
				continue;

			pj_inet_ntop(pj_AF_INET(), &addr_list[i].ipv4.sin_addr, addr,
				sizeof(addr));
			pj_strdup2(app.pool, &app.name[app.name_cnt].host,
				addr);
			app.name[app.name_cnt].port = app.port;
			app.name_cnt++;
		}
	}

	/* Add loopback address. */
#if PJ_IP_HELPER_IGNORE_LOOPBACK_IF
	app.name[app.name_cnt].host = pj_str("127.0.0.1");
	app.name[app.name_cnt].port = app.port;
	app.name_cnt++;
#endif

	app.name[app.name_cnt].host = *pj_gethostname();
	app.name[app.name_cnt].port = app.port;
	app.name_cnt++;

	app.name[app.name_cnt].host = pj_str("localhost");
	app.name[app.name_cnt].port = app.port;
	app.name_cnt++;

	PJ_LOG(3, (THIS_FILE, "Proxy started, listening on port %d", app.port));
	PJ_LOG(3, (THIS_FILE, "Local host aliases:"));
	for (i = 0; i<app.name_cnt; ++i) {
		PJ_LOG(3, (THIS_FILE, " %.*s:%d",
			(int)app.name[i].host.slen,
			app.name[i].host.ptr,
			app.name[i].port));
	}

	if (app.record_route) {
		PJ_LOG(3, (THIS_FILE, "Using Record-Route mode"));
	}

	return PJ_SUCCESS;
}
