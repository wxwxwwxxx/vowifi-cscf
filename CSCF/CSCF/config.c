#include "pj.h"
#include "cscf.h"
pj_status_t init_config()
{
	network_config.sip_af = pj_AF_INET();
	network_config.sip_port = 5060;
	network_config.sip_tcp = PJ_FALSE;
	app.enable_msg_logging = PJ_TRUE;
	return PJ_SUCCESS;
}