#include "module.h"
#include "cscf.h"
#include "pj.h"
//utf8
//log模块，直接打印收到的消息即可
pj_bool_t logging_on_rx_msg(pjsip_rx_data *rdata)
{
	
	if (!app.enable_msg_logging)
		return PJ_FALSE;

	PJ_LOG(3, (THIS_FILE, "RX %d bytes %s from %s %s:%d:\n"
		"%.*s\n"
		"--end msg--",
		rdata->msg_info.len,
		pjsip_rx_data_get_info(rdata),
		rdata->tp_info.transport->type_name,
		rdata->pkt_info.src_name,
		rdata->pkt_info.src_port,
		(int)rdata->msg_info.len,
		rdata->msg_info.msg_buf));
	return PJ_FALSE;
}
pj_status_t logging_on_tx_msg(pjsip_tx_data *tdata)
{
	if (!app.enable_msg_logging)
		return PJ_SUCCESS;

	PJ_LOG(3, (THIS_FILE, "TX %d bytes %s to %s %s:%d:\n"
		"%.*s\n"
		"--end msg--",
		(tdata->buf.cur - tdata->buf.start),
		pjsip_tx_data_get_info(tdata),
		tdata->tp_info.transport->type_name,
		tdata->tp_info.dst_name,
		tdata->tp_info.dst_port,
		(int)(tdata->buf.cur - tdata->buf.start),
		tdata->buf.start));
	return PJ_SUCCESS;
}
pjsip_module module_log =
{
	NULL, NULL,				/* prev, next.		*/
	{ "module_log", 10 },		/* Name.		*/
	-1,					/* Id			*/
	PJSIP_MOD_PRIORITY_TRANSPORT_LAYER - 2,/* Priority	        */
	NULL,				/* load()		*/
	NULL,				/* start()		*/
	NULL,				/* stop()		*/
	NULL,				/* unload()		*/
	&logging_on_rx_msg,			/* on_rx_request()	*/
	&logging_on_rx_msg,			/* on_rx_response()	*/
	&logging_on_tx_msg,			/* on_tx_request.	*/
	&logging_on_tx_msg,			/* on_tx_response()	*/
	NULL,				/* on_tsx_state()	*/

};