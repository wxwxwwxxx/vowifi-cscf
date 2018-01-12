#include "module.h"
#include "cscf.h"
#include "pj.h"
//utf8
pj_bool_t change_sdp(pjsip_rx_data *rdata)
{
//	PJ_LOG(3, ("SDP", "%s", pj_strdup4(app.pool, &rdata->msg_info.msg->line.req.method.name)));
	pjmedia_sdp_session *sdp;
	pj_status_t status;
	pjsip_msg_type_e msgtype = rdata->msg_info.msg->type;

	if(rdata->msg_info.msg->body==NULL)
		return PJ_FALSE;

	unsigned int len= rdata->msg_info.msg->body->len;
	void* sdp_buf = pj_pool_alloc(app.pool, len + 50);
	status = pjmedia_sdp_parse(app.pool, (char*)rdata->msg_info.msg->body->data, len, (&sdp));
	sdp->conn->addr = pj_str(PJ_SERVER_ADDRESS);
	if (msgtype == PJSIP_REQUEST_MSG)
	{
		sdp->media[0]->desc.port = 20001;
		PJ_LOG(3, (THIS_FILE, "REQUEST SDP"));
	}
	else
	{
		sdp->media[0]->desc.port = 20002;
		PJ_LOG(3, (THIS_FILE, "RESPONSE SDP"));
	}
	int len_buf=pjmedia_sdp_print(sdp, sdp_buf, len + 50);
	rdata->msg_info.msg->body->data = sdp_buf;
	rdata->msg_info.msg->body->len = len_buf;
	return PJ_FALSE;
}
pjsip_module module_rtpproxy =
{
	NULL, NULL,				/* prev, next.		*/
	{ "module_rtpproxy", 15 },		/* Name.		*/
	-1,					/* Id			*/
	PJSIP_MOD_PRIORITY_TRANSPORT_LAYER - 1,/* Priority	        */
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