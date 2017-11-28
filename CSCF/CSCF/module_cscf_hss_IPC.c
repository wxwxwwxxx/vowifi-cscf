#include "module.h"
#include "cscf.h"
#include "pj.h"
//utf8
char* pj_strdup4(pj_pool_t* pool,const pj_str_t * pjstr)
{
	char* key = (char*)pj_pool_alloc(pool, sizeof(char)*pj_strlen(pjstr) + 1);
	const char* buf = pj_strbuf(pjstr);
	pj_size_t len = pj_strlen(pjstr);
	for (unsigned int i = 0; i < len; i++)
	{
		key[i] = buf[i];
	}
	key[len] = '\0';
	return key;
}
pj_status_t set_chart(struct IPC_userinfo *ui)
{
	char* key = pj_strdup4(app.pool, &ui->user);
	pj_hash_set(app.pool, app.routing_chart, key, PJ_HASH_KEY_STRING, 0, ui);
	return PJ_SUCCESS;
}
//key ÊÇC·ç¸ñ×Ö·û´®
struct IPC_userinfo * get_chart(const char* key)
{
	return (struct IPC_userinfo *)pj_hash_get(app.routing_chart, key, PJ_HASH_KEY_STRING, 0);
}
pj_bool_t route_on_rx_msg(pjsip_rx_data *rdata)
{
	pjsip_method_e methodid= rdata->msg_info.msg->line.req.method.id;
	if (methodid == PJSIP_INVITE_METHOD || methodid == PJSIP_ACK_METHOD || methodid == PJSIP_BYE_METHOD || methodid == PJSIP_CANCEL_METHOD||pj_strcmp2(&rdata->msg_info.msg->line.req.method.name,"MESSAGE")==0)
	{

	}
	else
	{
		return PJ_FALSE;
	}
	pj_status_t status;
	pjsip_sip_uri * req_uri=pjsip_uri_get_uri(rdata->msg_info.msg->line.req.uri);
	char* key=pj_strdup4(app.pool, &req_uri->user);
	struct IPC_userinfo* userinfo = get_chart(key);
	if (userinfo == NULL)
	{
		status = pjsip_endpt_respond(app.sip_endpt, NULL, rdata, 404, NULL,NULL, NULL, NULL);
		return PJ_TRUE;
	}
	else
	{
		pj_strcpy(&req_uri->host, &userinfo->host);
		req_uri->port = userinfo->port;
	}
	return PJ_FALSE;
}

pjsip_module module_route =
{
	NULL, NULL,				/* prev, next.		*/
	{ "module_route", 12 },		/* Name.		*/
	-1,					/* Id			*/
	PJSIP_MOD_PRIORITY_UA_PROXY_LAYER - 1,/* Priority	        */
	NULL,				/* load()		*/
	NULL,				/* start()		*/
	NULL,				/* stop()		*/
	NULL,				/* unload()		*/
	&route_on_rx_msg,			/* on_rx_request()	*/
	&route_on_rx_msg,			/* on_rx_response()	*/
	NULL,
	NULL,
	NULL,				/* on_tsx_state()	*/

};