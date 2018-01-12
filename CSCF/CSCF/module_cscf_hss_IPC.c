#include "module.h"
#include "cscf.h"
#include "pj.h"
/*send.c*/
#include <stdio.h>   
#include <sys/types.h>   
#include <sys/ipc.h>   
#include <sys/msg.h>   
#include <errno.h>   
#include <stdlib.h>
#include<string.h>
#include<unistd.h>

struct msg_cscf
{
	long msgtype;
	char msgtext[100];
	int port;
	int expire;

} m_cscf;
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
pj_status_t set_chart()
{
	int ret;
	m_cscf.msgtype = 1;
	m_cscf.port = ui.port;
	for (int i = 0; i < ui.user.slen; i++)
	{
		m_cscf.msgtext[i] = ui.user.ptr[i];
	}
	m_cscf.msgtext[ui.user.slen] = ';';
	for (int i = 0; i < ui.host.slen; i++)
	{
		m_cscf.msgtext[i+1+ ui.user.slen] = ui.host.ptr[i];
	}
	m_cscf.msgtext[ui.user.slen+ui.host.slen+1] = '\0';

	m_cscf.expire = ui.expires;
	ret=msgsnd(app.send_id, &m_cscf, sizeof(struct msg_cscf), 0);
	assert(ret!=-1);
	return PJ_SUCCESS;
}
//key ÊÇC·ç¸ñ×Ö·û´®
pj_status_t  get_chart(const char* key)
{
	int ret;
	m_cscf.expire = 0;
	m_cscf.msgtype = 2;
	m_cscf.port = 0;
	strcpy(m_cscf.msgtext,key);
	ret=msgsnd(app.send_id, &m_cscf, sizeof(struct msg_cscf), 0);
	assert(ret != -1);
	ret=msgrcv(app.recv_id, &m_cscf, sizeof(struct msg_cscf), 0, 0);
	assert(ret != -1);
	if (m_cscf.expire == -1)return PJ_TRUE;
	ui.user = pj_str(key);
	ui.host = pj_str(m_cscf.msgtext);
	ui.port = m_cscf.port;
	ui.expires = 0;
	return PJ_SUCCESS;
}
pj_bool_t route_on_rx_msg(pjsip_rx_data *rdata)
{
//	PJ_LOG(3, ("IPC", "%s", pj_strdup4(app.pool, &rdata->msg_info.msg->line.req.method.name)));
	pjsip_msg_type_e msgtype= rdata->msg_info.msg->type;
//	if (msgtype==PJSIP_RESPONSE_MSG)
//	{
//		return PJ_FALSE;
//	}
	pj_status_t status;
	pjsip_sip_uri * req_uri = pjsip_uri_get_uri(rdata->msg_info.msg->line.req.uri);
	char* key = pj_strdup4(app.pool, &req_uri->user);
	status = get_chart(key);
	if (status != PJ_SUCCESS)
	{
		status = pjsip_endpt_respond(app.sip_endpt, NULL, rdata, 404, NULL, NULL, NULL, NULL);
		return PJ_TRUE;
	}
	else
	{
		pj_strcpy(&req_uri->host, &ui.host);
		req_uri->port = ui.port;
	}
	return PJ_FALSE;
}

pjsip_module module_route =
{
	NULL, NULL,				/* prev, next.		*/
	{ "module_route", 12 },		/* Name.		*/
	-1,					/* Id			*/
	PJSIP_MOD_PRIORITY_UA_PROXY_LAYER- 2,/* Priority	        */
	NULL,				/* load()		*/
	NULL,				/* start()		*/
	NULL,				/* stop()		*/
	NULL,				/* unload()		*/
	&route_on_rx_msg,			/* on_rx_request()	*/
	NULL,//&route_on_rx_msg,			/* on_rx_response()	*/
	NULL,
	NULL,
	NULL,				/* on_tsx_state()	*/

};