#include "pj.h"
#include "cscf.h"
pj_status_t init_config()
{
	network_config.sip_af = pj_AF_INET();
	network_config.sip_port = PJ_SERVER_PORT;
	app.port = network_config.sip_port;
	network_config.sip_tcp = PJ_FALSE;
	app.enable_msg_logging = PJ_TRUE;
	registrar_config.authenticate=PJ_TRUE;
	registrar_config.respond = PJ_TRUE;
	registrar_config.status_code = 200;
	registrar_config.expires = 300;
	registrar_config.expires_param = 300;
	registrar_config.contact_op = MODIFIED;
	app.record_route=PJ_FALSE;
	return PJ_SUCCESS;
}