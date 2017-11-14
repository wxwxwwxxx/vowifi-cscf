#include"cscf.h"
#include "pj.h"
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
pj_status_t init_stack()
{
	pj_init();
	init_config();
	pj_sockaddr addr;
	pj_status_t status;
	pj_log_set_level(6);
	status = pjlib_util_init();
	CHECK_STATUS();
	pj_caching_pool_init(&app.cp, NULL, 0);
	app.pool = pj_pool_create(&app.cp.factory, "CSCF", 512, 512, 0);
	status = pjsip_endpt_create(&app.cp.factory, NULL, &app.sip_endpt);
	CHECK_STATUS();
	pj_log_set_level(6);
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
	CHECK_STATUS();
	status=register_module();
	CHECK_STATUS();
	pj_thread_create(app.pool, "CSCF", &worker_proc, NULL, 0, 0,&app.worker_thread);
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
