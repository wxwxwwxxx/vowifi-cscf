#include "module.h"
#include "cscf.h"
#include "pj.h"
//utf8
pj_status_t set_chart(struct IPC_userinfo *ui)
{
	char* key = pj_strbuf(&ui->user);
	key[pj_strlen(&ui->user)] = '\0';
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
	pj_status_t status;
	pjsip_sip_uri * req_uri=pjsip_uri_get_uri(rdata->msg_info.msg->line.req.uri);
	char* key=pj_strbuf(&req_uri->user);
	key[pj_strlen(&req_uri->user)] = '\0';
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
	/*
	char *a = "cmn";
	char *b = "heio";
	pj_status_t status;
	pj_str_t str = pj_str("nihao");
	str.slen = 4;
	pj_str_t str2 = pj_str("cnma");
	status = pj_init();
	pj_caching_pool_init(&app.cp, NULL, 0);
	app.pool = pj_pool_create(&app.cp.factory, "sipecho", 512, 512, 0);
	pj_hash_table_t * ht = pj_hash_create(app.pool, 20);
	pj_hash_set(app.pool, ht, a, PJ_HASH_KEY_STRING, 0, &str);
	pj_hash_set(app.pool, ht, b, PJ_HASH_KEY_STRING, 0, &str2);
	char c[16];// = "cmnddddd";
	c[0] = 'c';
	c[1] = 'm';
	c[2] = 'n';
	c[3] = '\0';
	c[4] = 'n';
	c[5] = 'n';
	PJ_LOG(3, (THIS_FILE, pj_strbuf((pj_str_t*)pj_hash_get(ht, c, PJ_HASH_KEY_STRING, 0))));
	PJ_LOG(3, (THIS_FILE, pj_strbuf((pj_str_t*)pj_hash_get(ht, "heio", PJ_HASH_KEY_STRING, 0))));
	void *asd=pj_hash_get(ht, "hsl", PJ_HASH_KEY_STRING, 0);
	return 0;
	*/