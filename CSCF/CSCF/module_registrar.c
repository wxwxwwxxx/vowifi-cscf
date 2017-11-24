#include "module.h"
#include "cscf.h"
#include "pj.h"

pj_bool_t regs_rx_request(pjsip_rx_data *rdata)
{
	pjsip_msg *msg = rdata->msg_info.msg;
	pjsip_hdr hdr_list;
	int code;
	pj_status_t status;
	

	if (msg->line.req.method.id != PJSIP_REGISTER_METHOD )
		if (msg->line.req.method.id == PJSIP_OTHER_METHOD)
		{
			/*如果是subscribe方法，暂时回应200*/
			status = pjsip_endpt_respond(app.sip_endpt, NULL, rdata, 200, NULL,
				NULL, NULL, NULL);
		}
		else
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
		if (registrar_config.contact_op == EXACT ||registrar_config.contact_op == MODIFIED)
		{
			pjsip_hdr *hsrc;

			for (hsrc = msg->hdr.next; hsrc != &msg->hdr; hsrc = hsrc->next) {
				pjsip_contact_hdr *hdst;

				if (hsrc->type != PJSIP_H_CONTACT)
					continue;

				hdst = (pjsip_contact_hdr*)
					pjsip_hdr_clone(rdata->tp_info.pool, hsrc);
				if (hdst->expires == 0)
					continue;

				if (registrar_config.contact_op == MODIFIED) {
					if (PJSIP_URI_SCHEME_IS_SIP(hdst->uri) ||
						PJSIP_URI_SCHEME_IS_SIPS(hdst->uri))
					{
						pjsip_sip_uri *sip_uri = (pjsip_sip_uri*)
							pjsip_uri_get_uri(hdst->uri);
						sip_uri->host = pj_str("x-modified-host");
						sip_uri->port = 1;

					}
				}
				if (registrar_config.expires_param)
					hdst->expires = registrar_config.expires_param;

				pj_list_push_back(&hdr_list, hdst);
			}
		}

		if (registrar_config.more_contacts.slen) {
			pjsip_generic_string_hdr *hcontact;
			const pj_str_t hname = pj_str("Contact");

			hcontact = pjsip_generic_string_hdr_create(rdata->tp_info.pool, &hname,
				&registrar_config.more_contacts);
			pj_list_push_back(&hdr_list, hcontact);
		}

		if (registrar_config.expires) {
			pjsip_expires_hdr *hexp;

			hexp = pjsip_expires_hdr_create(rdata->tp_info.pool,
				registrar_config.expires);
			pj_list_push_back(&hdr_list, hexp);
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
	PJSIP_MOD_PRIORITY_UA_PROXY_LAYER-1, /* Priority			*/
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
