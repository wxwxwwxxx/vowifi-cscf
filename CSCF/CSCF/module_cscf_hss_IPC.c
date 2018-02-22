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
//用于将注册信息发送至HSS
struct msg_cscf
{
	long msgtype;
	char msgtext[100];
	int port;
	int expire;

} m_cscf;
//utf8
//将pj_str的内容复制至char*
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
//设置路由表，将用户信息复制至消息队列并发送
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
//访问路由表，获取用户信息并保存至ui结构体
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
	//当expire为-1时，代表查找失败，不存在该用户
	if (m_cscf.expire == -1)return PJ_TRUE;
	ui.user = pj_str(key);
	ui.host = pj_str(m_cscf.msgtext);
	ui.port = m_cscf.port;
	ui.expires = 0;
	return PJ_SUCCESS;
}
//模块的响应函数，用于将sip消息的目的地址替换为被叫者的真实地址
pj_bool_t route_on_rx_msg(pjsip_rx_data *rdata)
{

	pjsip_msg_type_e msgtype= rdata->msg_info.msg->type;

	pj_status_t status;
	pjsip_sip_uri * req_uri = pjsip_uri_get_uri(rdata->msg_info.msg->line.req.uri);
	char* key = pj_strdup4(app.pool, &req_uri->user);
	status = get_chart(key);
	if (status != PJ_SUCCESS)
	{
		//路由表未查找到时，暂时返回404，接入能力云平台时可能需要修改这里
		status = pjsip_endpt_respond(app.sip_endpt, NULL, rdata, 404, NULL, NULL, NULL, NULL);
		PJ_LOG(3, ("BYE", "%s", pj_strdup4(app.pool, &rdata->msg_info.cid->id)));
		get_port(3, &rdata->msg_info.cid->id);
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
	PJSIP_MOD_PRIORITY_UA_PROXY_LAYER-2,/* Priority	        */
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