#include "module.h"
#include "cscf.h"
#include "pj.h"

/*与HSS通信所需数据结构*/
IPC_userinfo ui;

pj_bool_t regs_rx_request(pjsip_rx_data *rdata)
{
//	PJ_LOG(3, ("REG", "%s", pj_strdup4(app.pool, &rdata->msg_info.msg->line.req.method.name)));
	pjsip_msg *msg = rdata->msg_info.msg;
	pjsip_hdr hdr_list;
	int code;
	pj_status_t status;
	if (pj_strcmp2(&msg->line.req.method.name,"SUBSCRIBE")==0)
	{
		/*如果是subscribe方法，暂时回应200*/
		status = pjsip_endpt_respond(app.sip_endpt, NULL, rdata, 200, NULL,
			NULL, NULL, NULL);
		return PJ_TRUE;
	}
	if (msg->line.req.method.id != PJSIP_REGISTER_METHOD)
		return PJ_FALSE;	
	if (!registrar_config.respond)
		return PJ_TRUE;

	pj_list_init(&hdr_list);

	if (registrar_config.authenticate &&
		pjsip_msg_find_hdr(msg, PJSIP_H_AUTHORIZATION, NULL) == NULL)
	{
		pjsip_generic_string_hdr *hwww;
		const pj_str_t hname = pj_str("WWW-Authenticate");
		const pj_str_t hvalue = pj_str("Digest realm=\"192.168.1.101\",nonce=\"d54e4bb9-fc22-4e08-8b69-442e1b8774eb\",algorithm=MD5, qop=\"auth\"");

		hwww = pjsip_generic_string_hdr_create(rdata->tp_info.pool, &hname,
			&hvalue);
		pj_list_push_back(&hdr_list, hwww);

		code = 401;

	}
	else {
		pjsip_hdr *hsrc;
		for (hsrc = msg->hdr.next; hsrc != &msg->hdr; hsrc = hsrc->next) {
			pjsip_contact_hdr *hdst;
			if (hsrc->type != PJSIP_H_CONTACT)
				continue;

			hdst = (pjsip_contact_hdr*)pjsip_hdr_clone(rdata->tp_info.pool, hsrc);
			//save it to routing_chart
			pjsip_sip_uri *contact = (pjsip_sip_uri*)pjsip_uri_get_uri(hdst->uri);

			pj_strdup(app.pool, &ui.user, &contact->user);
			pj_strdup2(app.pool, &ui.host, rdata->pkt_info.src_name);
			ui.port = rdata->pkt_info.src_port;
			pjsip_hdr *h = pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_EXPIRES, NULL);
			//////Need Check
			//优先采用Expire字段
			if (h == NULL)
			{
				ui.expires = hdst->expires;
			}
			else
			{
				ui.expires = ((pjsip_expires_hdr*)h)->ivalue;
			}
			if (ui.expires == 0)
			{
				ui.valid = 0;
			}
			else
			{
				ui.valid = 1;
			}
			PJ_LOG(3, (THIS_FILE, " REGISTER SUCCESS:%s  %d", pj_strdup4(app.pool, &ui.user), ui.expires));
			status=set_chart();
		}
		code = registrar_config.status_code;
	}

	status = pjsip_endpt_respond(app.sip_endpt, NULL, rdata, code, NULL,
		&hdr_list, NULL, NULL);
	return PJ_TRUE;
}
pjsip_module module_registrar =
{
	NULL, NULL,			    /* prev, next.		*/
	{ "module_registrar", 16 },	    /* Name.			*/
	-1,				    /* Id			*/
	PJSIP_MOD_PRIORITY_UA_PROXY_LAYER-3, /* Priority			*/
	NULL,			    /* load()			*/
	NULL,			    /* start()			*/
	NULL,			    /* stop()			*/
	NULL,			    /* unload()			*/
	&regs_rx_request,		    /* on_rx_request()		*/
	NULL,			    /* on_rx_response()		*/
	NULL,			    /* on_tx_request.		*/
	NULL,			    /* on_tx_response()		*/
	NULL,			    /* on_tsx_state()		*/
};

