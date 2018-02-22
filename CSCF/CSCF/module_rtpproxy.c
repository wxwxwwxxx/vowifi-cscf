#include "module.h"
#include "cscf.h"
#include "pj.h"
//utf8
#include <stdio.h>   
#include <sys/types.h>   
#include <sys/ipc.h>   
#include <sys/msg.h>   
#include <errno.h>   
#include <stdlib.h>
#include<string.h>
#include<unistd.h>
//rtp转发模块
msg_rtpproxy m_rtpproxy;
pj_bool_t change_sdp(pjsip_rx_data *rdata)
{
	pjmedia_sdp_session *sdp;
	pj_status_t status;
	pjsip_msg_type_e msgtype = rdata->msg_info.msg->type;
	//是挂断消息时，释放端口
	if (rdata->msg_info.msg->line.req.method.id==PJSIP_BYE_METHOD
		|| rdata->msg_info.msg->line.req.method.id==PJSIP_CANCEL_METHOD
		||pj_strcmp2( &rdata->msg_info.msg->line.req.method.name,"Decline")==0)
	{
		PJ_LOG(3, ("BYE", "%s", pj_strdup4(app.pool, &rdata->msg_info.cid->id)));
		get_port(3, &rdata->msg_info.cid->id);
		return PJ_FALSE;
	}
	if(rdata->msg_info.msg->body==NULL)
		return PJ_FALSE;
	if(pj_strcmp2(&rdata->msg_info.msg->body->content_type.subtype, "sdp") != 0)
		return PJ_FALSE;

	unsigned int len= rdata->msg_info.msg->body->len;
	void* sdp_buf = pj_pool_alloc(app.pool, len + 50);
	status = pjmedia_sdp_parse(app.pool, (char*)rdata->msg_info.msg->body->data, len, (&sdp));
	sdp->conn->addr = pj_str(PJ_SERVER_ADDRESS);
	//根据是响应还是请求来获得端口号，并修改SDP
	if (msgtype == PJSIP_REQUEST_MSG)
	{
		pj_uint16_t port = get_port(1, &rdata->msg_info.cid->id);
		if (port == 0)
		{
			status = pjsip_endpt_respond(app.sip_endpt, NULL, rdata, 486, NULL, NULL, NULL, NULL);
			return PJ_TRUE;
		}
		sdp->media[0]->desc.port = port;
		PJ_LOG(3, (THIS_FILE, "REQUEST SDP,%u", port));
	}
	else
	{
		pj_uint16_t port=get_port(2, &rdata->msg_info.cid->id);
		if (port == 0)
		{
			return PJ_FALSE;
		}
		sdp->media[0]->desc.port = port;
		PJ_LOG(3, (THIS_FILE, "RESPONSE SDP,%u", port));
	}
	int len_buf=pjmedia_sdp_print(sdp, sdp_buf, len + 50);
	rdata->msg_info.msg->body->data = sdp_buf;
	rdata->msg_info.msg->body->len = len_buf;
	return PJ_FALSE;
}
//根据cid设置消息队列
void set_m_rtpproxy(long mtype,pj_str_t* str)
{
	m_rtpproxy.msgtype = mtype;
	for (int i = 0; i < str->slen; i++)
	{
		m_rtpproxy.msgtext[i] = str->ptr[i];
	}
	m_rtpproxy.msgtext[str->slen] = '\0';
	m_rtpproxy.port = 0;
}
//获得rtp转发端口号
pj_uint16_t get_port(long mtype, pj_str_t* str)
{
	int ret;
	set_m_rtpproxy(mtype, str);
	ret = msgsnd(app.rtp_send_id, &m_rtpproxy, sizeof(msg_rtpproxy), 0);
	assert(ret != -1);
	if (mtype == 3)return 0;
	ret = msgrcv(app.rtp_recv_id, &m_rtpproxy, sizeof(msg_rtpproxy), 0, 0);
	assert(ret != -1);
	return m_rtpproxy.port;
}

pjsip_module module_rtpproxy =
{
	NULL, NULL,				/* prev, next.		*/
	{ "module_rtpproxy", 15 },		/* Name.		*/
	-1,					/* Id			*/
	PJSIP_MOD_PRIORITY_TRANSPORT_LAYER - 1,/* Priority	        *///优先级在传输层之前时会有bug
	NULL,				/* load()		*/
	NULL,				/* start()		*/
	NULL,				/* stop()		*/
	NULL,				/* unload()		*/
	&change_sdp,			/* on_rx_request()	*/
	&change_sdp,			/* on_rx_response()	*/
	NULL,			/* on_tx_request.	*/
	NULL,			/* on_tx_response()	*/
	NULL,				/* on_tsx_state()	*/

};