#include "pj.h"
#include "cscf.h"
network_config_t network_config;
registrar_cfg registrar_config;

//初始化各个设置项，各个设置项的意义请参考设置结构体的注释
pj_status_t init_config()
{
	network_config.sip_af = pj_AF_INET();
	//network_config.sip_port = 
	app.port = PJ_SERVER_PORT;
	network_config.sip_tcp = PJ_FALSE;
	app.enable_msg_logging = PJ_FALSE;
	registrar_config.authenticate=PJ_TRUE;
	registrar_config.respond = PJ_TRUE;
	registrar_config.status_code = 200;
	//registrar_config.expires = 300;
	//registrar_config.expires_param = 300;
	//registrar_config.contact_op = MODIFIED;
	app.record_route=PJ_TRUE;
	return PJ_SUCCESS;
}