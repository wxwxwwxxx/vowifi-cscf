#include "cscf.h"
#include "pj.h"
//模块注册函数
//注：添加模块时请编辑该函数，增加模块的注册
pj_status_t register_module()
{
	pj_status_t status;
	status = pjsip_tsx_layer_init_module(app.sip_endpt);
	CHECK_STATUS();
	status = pjsip_endpt_register_module(app.sip_endpt, &module_proxy);
	CHECK_STATUS();
	status = pjsip_endpt_register_module(app.sip_endpt, &mod_tu);
	CHECK_STATUS();
	status = pjsip_endpt_register_module(app.sip_endpt, &module_registrar);
	CHECK_STATUS();
	status = pjsip_endpt_register_module(app.sip_endpt, &module_log);
	CHECK_STATUS();
	status = pjsip_endpt_register_module(app.sip_endpt, &module_route);
	CHECK_STATUS();
	status = pjsip_endpt_register_module(app.sip_endpt, &module_rtpproxy);
	CHECK_STATUS();
	return PJ_SUCCESS;
}