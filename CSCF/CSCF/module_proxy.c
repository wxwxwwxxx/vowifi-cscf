#include "module.h"
#include "cscf.h"
#include "pj.h"
//utf8
/* Callback to be called to handle new incoming requests. */
static pj_bool_t is_uri_local(const pjsip_sip_uri *uri)
{
	unsigned i;
	for (i = 0; i<app.name_cnt; ++i) {
		if ((uri->port == app.name[i].port ||
			(uri->port == 0 && app.name[i].port == 5060)) &&
			pj_stricmp(&uri->host, &app.name[i].host) == 0)
		{
			/* Match */
			return PJ_TRUE;
		}
	}

	/* Doesn't match */
	return PJ_FALSE;
}
static void proxy_postprocess(pjsip_tx_data *tdata)
{
	/* Optionally record-route */
	if (app.record_route) {
		char uribuf[128];
		pj_str_t uri;
		const pj_str_t H_RR = { "Record-Route", 12 };
		pjsip_generic_string_hdr *rr;

		pj_ansi_snprintf(uribuf, sizeof(uribuf), "<sip:%.*s:%d;lr>",
			(int)app.name[0].host.slen,
			app.name[0].host.ptr,
			app.name[0].port);
		uri = pj_str(uribuf);
		rr = pjsip_generic_string_hdr_create(tdata->pool,
			&H_RR, &uri);
		pjsip_msg_insert_first_hdr(tdata->msg, (pjsip_hdr*)rr);
	}
}
static  pj_status_t proxy_calculate_target(pjsip_rx_data *rdata,pjsip_tx_data *tdata)
{
	pjsip_sip_uri *target;

	/* RFC 3261 Section 16.5 Determining Request Targets */

	target = (pjsip_sip_uri*)tdata->msg->line.req.uri;

	/* If the Request-URI of the request contains an maddr parameter, the
	* Request-URI MUST be placed into the target set as the only target
	* URI, and the proxy MUST proceed to Section 16.6.
	*/
	if (target->maddr_param.slen) {
		proxy_postprocess(tdata);
		return PJ_SUCCESS;
	}


	/* If the domain of the Request-URI indicates a domain this element is
	* not responsible for, the Request-URI MUST be placed into the target
	* set as the only target, and the element MUST proceed to the task of
	* Request Forwarding (Section 16.6).
	*/
	if (!is_uri_local(target)) {
		proxy_postprocess(tdata);
		return PJ_SUCCESS;
	}

	/* If the target set for the request has not been predetermined as
	* described above, this implies that the element is responsible for the
	* domain in the Request-URI, and the element MAY use whatever mechanism
	* it desires to determine where to send the request.
	*/

	/* We're not interested to receive request destined to us, so
	* respond with 404/Not Found (only if request is not ACK!).
	*/
	if (rdata->msg_info.msg->line.req.method.id != PJSIP_ACK_METHOD) {
		pjsip_endpt_respond_stateless(app.sip_endpt, rdata,
			PJSIP_SC_NOT_FOUND, NULL,
			NULL, NULL);
	}

	/* Delete the request since we're not forwarding it */
	pjsip_tx_data_dec_ref(tdata);

	return PJSIP_ERRNO_FROM_SIP_STATUS(PJSIP_SC_NOT_FOUND);
}
static void app_perror(const char *msg, pj_status_t status)
{
	char errmsg[PJ_ERR_MSG_SIZE];

	pj_strerror(status, errmsg, sizeof(errmsg));
	PJ_LOG(1, (THIS_FILE, "%s: %s", msg, errmsg));
}
static pj_status_t proxy_process_routing(pjsip_tx_data *tdata)
{
	pjsip_sip_uri *target;
	pjsip_route_hdr *hroute;

	/* RFC 3261 Section 16.4 Route Information Preprocessing */

	target = (pjsip_sip_uri*)tdata->msg->line.req.uri;

	/* The proxy MUST inspect the Request-URI of the request.  If the
	* Request-URI of the request contains a value this proxy previously
	* placed into a Record-Route header field (see Section 16.6 item 4),
	* the proxy MUST replace the Request-URI in the request with the last
	* value from the Route header field, and remove that value from the
	* Route header field.  The proxy MUST then proceed as if it received
	* this modified request.
	*/
	if (is_uri_local(target)) {
		pjsip_route_hdr *r;
		pjsip_sip_uri *uri;

		/* Find the first Route header */
		r = hroute = (pjsip_route_hdr*)
			pjsip_msg_find_hdr(tdata->msg, PJSIP_H_ROUTE, NULL);
		if (r == NULL) {
			/* No Route header. This request is destined for this proxy. */
			return PJ_SUCCESS;
		}

		/* Find the last Route header */
		while ((r = (pjsip_route_hdr*)pjsip_msg_find_hdr(tdata->msg,
			PJSIP_H_ROUTE,
			r->next)) != NULL)
		{
			hroute = r;
		}

		/* If the last Route header doesn't have ";lr" parameter, then
		* this is a strict-routed request indeed, and we follow the steps
		* in processing strict-route requests above.
		*
		* But if it does contain ";lr" parameter, skip the strict-route
		* processing.
		*/
		uri = (pjsip_sip_uri*)
			pjsip_uri_get_uri(&hroute->name_addr);
		if (uri->lr_param == 0) {
			/* Yes this is strict route, so:
			* - replace req URI with the URI in Route header,
			* - remove the Route header,
			* - proceed as if it received this modified request.
			*/
			tdata->msg->line.req.uri = hroute->name_addr.uri;
			target = (pjsip_sip_uri*)tdata->msg->line.req.uri;
			pj_list_erase(hroute);
		}
	}

	/* If the Request-URI contains a maddr parameter, the proxy MUST check
	* to see if its value is in the set of addresses or domains the proxy
	* is configured to be responsible for.  If the Request-URI has a maddr
	* parameter with a value the proxy is responsible for, and the request
	* was received using the port and transport indicated (explicitly or by
	* default) in the Request-URI, the proxy MUST strip the maddr and any
	* non-default port or transport parameter and continue processing as if
	* those values had not been present in the request.
	*/
	if (target->maddr_param.slen != 0) {
		pjsip_sip_uri maddr_uri;

		maddr_uri.host = target->maddr_param;
		maddr_uri.port = app.port;

		if (is_uri_local(&maddr_uri)) {
			target->maddr_param.slen = 0;
			target->port = 0;
			target->transport_param.slen = 0;
		}
	}

	/* If the first value in the Route header field indicates this proxy,
	* the proxy MUST remove that value from the request.
	*/
	hroute = (pjsip_route_hdr*)
		pjsip_msg_find_hdr(tdata->msg, PJSIP_H_ROUTE, NULL);
	if (hroute && is_uri_local((pjsip_sip_uri*)hroute->name_addr.uri)) {
		pj_list_erase(hroute);
	}

	return PJ_SUCCESS;
}
static pj_status_t proxy_verify_request(pjsip_rx_data *rdata)
{
	const pj_str_t STR_PROXY_REQUIRE = { "Proxy-Require", 13 };

	/* RFC 3261 Section 16.3 Request Validation */

	/* Before an element can proxy a request, it MUST verify the message's
	* validity.  A valid message must pass the following checks:
	*
	* 1. Reasonable Syntax
	* 2. URI scheme
	* 3. Max-Forwards
	* 4. (Optional) Loop Detection
	* 5. Proxy-Require
	* 6. Proxy-Authorization
	*/

	/* 1. Reasonable Syntax.
	* This would have been checked by transport layer.
	*/

	/* 2. URI scheme.
	* We only want to support "sip:"/"sips:" URI scheme for this simple proxy.
	*/
	if (!PJSIP_URI_SCHEME_IS_SIP(rdata->msg_info.msg->line.req.uri) &&
		!PJSIP_URI_SCHEME_IS_SIPS(rdata->msg_info.msg->line.req.uri))
	{
		pjsip_endpt_respond_stateless(app.sip_endpt, rdata,
			PJSIP_SC_UNSUPPORTED_URI_SCHEME, NULL,
			NULL, NULL);
		return PJSIP_ERRNO_FROM_SIP_STATUS(PJSIP_SC_UNSUPPORTED_URI_SCHEME);
	}

	/* 3. Max-Forwards.
	* Send error if Max-Forwards is 1 or lower.
	*/
	if (rdata->msg_info.max_fwd && rdata->msg_info.max_fwd->ivalue <= 1) {
		pjsip_endpt_respond_stateless(app.sip_endpt, rdata,
			PJSIP_SC_TOO_MANY_HOPS, NULL,
			NULL, NULL);
		return PJSIP_ERRNO_FROM_SIP_STATUS(PJSIP_SC_TOO_MANY_HOPS);
	}

	/* 4. (Optional) Loop Detection.
	* Nah, we don't do that with this simple proxy.
	*/

	/* 5. Proxy-Require */
	if (pjsip_msg_find_hdr_by_name(rdata->msg_info.msg, &STR_PROXY_REQUIRE,
		NULL) != NULL)
	{
		pjsip_endpt_respond_stateless(app.sip_endpt, rdata,
			PJSIP_SC_BAD_EXTENSION, NULL,
			NULL, NULL);
		return PJSIP_ERRNO_FROM_SIP_STATUS(PJSIP_SC_BAD_EXTENSION);
	}

	/* 6. Proxy-Authorization.
	* Nah, we don't require any authorization with this sample.
	*/

	return PJ_SUCCESS;
}




pj_bool_t proxy_on_rx_request(pjsip_rx_data *rdata)
{
	pjsip_transaction *uas_tsx, *uac_tsx;
	struct uac_data *uac_data;
	struct uas_data *uas_data;
	pjsip_tx_data *tdata;
	pj_status_t status;

	if (rdata->msg_info.msg->line.req.method.id != PJSIP_CANCEL_METHOD) {

		/* Verify incoming request */
		status = proxy_verify_request(rdata);
		if (status != PJ_SUCCESS) {
			app_perror("RX invalid request", status);
			return PJ_TRUE;
		}

		/*
		* Request looks sane, next clone the request to create transmit data.
		*/
		status = pjsip_endpt_create_request_fwd(app.sip_endpt, rdata, NULL,
			NULL, 0, &tdata);
		if (status != PJ_SUCCESS) {
			pjsip_endpt_respond_stateless(app.sip_endpt, rdata,
				PJSIP_SC_INTERNAL_SERVER_ERROR,
				NULL, NULL, NULL);
			return PJ_TRUE;
		}
#ifdef PJ_SERVER_TEST 
		tdata->via_addr.host = pj_str(PJ_SERVER_ADDRESS);
		tdata->via_addr.port = PJ_SERVER_PORT;
#endif // PJ_SERVER_TEST 

		/* Process routing */
		status = proxy_process_routing(tdata);
		if (status != PJ_SUCCESS) {
			app_perror("Error processing route", status);
			return PJ_TRUE;
		}



		/* Calculate target */
		status = proxy_calculate_target(rdata, tdata);
		if (status != PJ_SUCCESS) {
			app_perror("Error calculating target", status);
			return PJ_TRUE;
		}

		/* Everything is set to forward the request. */

		/* If this is an ACK request, forward statelessly.
		* This happens if the proxy records route and this ACK
		* is sent for 2xx response. An ACK that is sent for non-2xx
		* final response will be absorbed by transaction layer, and
		* it will not be received by on_rx_request() callback.
		*/
		if (tdata->msg->line.req.method.id == PJSIP_ACK_METHOD) {
			status = pjsip_endpt_send_request_stateless(app.sip_endpt, tdata,
				NULL, NULL);
			if (status != PJ_SUCCESS) {
				app_perror("Error forwarding request", status);
				return PJ_TRUE;
			}

			return PJ_TRUE;
		}

		/* Create UAC transaction for forwarding the request.
		* Set our module as the transaction user to receive further
		* events from this transaction.
		*/
		status = pjsip_tsx_create_uac(&mod_tu, tdata, &uac_tsx);
		if (status != PJ_SUCCESS) {
			pjsip_tx_data_dec_ref(tdata);
			pjsip_endpt_respond_stateless(app.sip_endpt, rdata,
				PJSIP_SC_INTERNAL_SERVER_ERROR,
				NULL, NULL, NULL);
			return PJ_TRUE;
		}

		/* Create UAS transaction to handle incoming request */
		status = pjsip_tsx_create_uas(&mod_tu, rdata, &uas_tsx);
		if (status != PJ_SUCCESS) {
			pjsip_tx_data_dec_ref(tdata);
			pjsip_endpt_respond_stateless(app.sip_endpt, rdata,
				PJSIP_SC_INTERNAL_SERVER_ERROR,
				NULL, NULL, NULL);
			pjsip_tsx_terminate(uac_tsx, PJSIP_SC_INTERNAL_SERVER_ERROR);
			return PJ_TRUE;
		}

		/* Feed the request to the UAS transaction to drive it's state
		* out of NULL state.
		*/
		pjsip_tsx_recv_msg(uas_tsx, rdata);

		/* Attach a data to the UAC transaction, to be used to find the
		* UAS transaction when we receive response in the UAC side.
		*/
		uac_data = (struct uac_data*)
			pj_pool_alloc(uac_tsx->pool, sizeof(struct uac_data));
		uac_data->uas_tsx = uas_tsx;
		uac_tsx->mod_data[mod_tu.id] = (void*)uac_data;

		/* Attach data to the UAS transaction, to find the UAC transaction
		* when cancelling INVITE request.
		*/
		uas_data = (struct uas_data*)
			pj_pool_alloc(uas_tsx->pool, sizeof(struct uas_data));
		uas_data->uac_tsx = uac_tsx;
		uas_tsx->mod_data[mod_tu.id] = (void*)uas_data;

		/* Everything is setup, forward the request */
		status = pjsip_tsx_send_msg(uac_tsx, tdata);
		if (status != PJ_SUCCESS) {
			pjsip_tx_data *err_res;

			/* Fail to send request, for some reason */

			/* Destroy transmit data */
			pjsip_tx_data_dec_ref(tdata);

			/* I think UAC transaction should have been destroyed when
			* it fails to send request, so no need to destroy it.
			pjsip_tsx_terminate(uac_tsx, PJSIP_SC_INTERNAL_SERVER_ERROR);
			*/

			/* Send 500/Internal Server Error to UAS transaction */
			pjsip_endpt_create_response(app.sip_endpt, rdata,
				500, NULL, &err_res);
			pjsip_tsx_send_msg(uas_tsx, err_res);

			return PJ_TRUE;
		}

		/* Send 100/Trying if this is an INVITE */
		if (rdata->msg_info.msg->line.req.method.id == PJSIP_INVITE_METHOD) {
			pjsip_tx_data *res100;

			pjsip_endpt_create_response(app.sip_endpt, rdata, 100, NULL,
				&res100);
			pjsip_tsx_send_msg(uas_tsx, res100);
		}

	}
	else {
		/* This is CANCEL request */
		pjsip_transaction *invite_uas;
		struct uas_data *uas_data2;
		pj_str_t key;

		/* Find the UAS INVITE transaction */
		pjsip_tsx_create_key(rdata->tp_info.pool, &key, PJSIP_UAS_ROLE,
			pjsip_get_invite_method(), rdata);
		invite_uas = pjsip_tsx_layer_find_tsx(&key, PJ_TRUE);
		if (!invite_uas) {
			/* Invite transaction not found, respond CANCEL with 481 */
			pjsip_endpt_respond_stateless(app.sip_endpt, rdata, 481, NULL,
				NULL, NULL);
			return PJ_TRUE;
		}

		/* Respond 200 OK to CANCEL */
		pjsip_endpt_respond(app.sip_endpt, NULL, rdata, 200, NULL, NULL,
			NULL, NULL);

		/* Send CANCEL to cancel the UAC transaction.
		* The UAS INVITE transaction will get final response when
		* we receive final response from the UAC INVITE transaction.
		*/
		uas_data2 = (struct uas_data*) invite_uas->mod_data[mod_tu.id];
		if (uas_data2->uac_tsx && uas_data2->uac_tsx->status_code < 200) {
			pjsip_tx_data *cancel;

			pj_grp_lock_acquire(uas_data2->uac_tsx->grp_lock);

			pjsip_endpt_create_cancel(app.sip_endpt, uas_data2->uac_tsx->last_tx,
				&cancel);
			pjsip_endpt_send_request(app.sip_endpt, cancel, -1, NULL, NULL);

			pj_grp_lock_release(uas_data2->uac_tsx->grp_lock);
		}

		/* Unlock UAS tsx because it is locked in find_tsx() */
		pj_grp_lock_release(invite_uas->grp_lock);
	}

	return PJ_TRUE;
}


/* Callback to be called to handle incoming response outside
* any transactions. This happens for example when 2xx/OK
* for INVITE is received and transaction will be destroyed
* immediately, so we need to forward the subsequent 2xx/OK
* retransmission statelessly.
*/
pj_bool_t proxy_on_rx_response(pjsip_rx_data *rdata)
{
	pjsip_tx_data *tdata;
	pjsip_response_addr res_addr;
	pjsip_via_hdr *hvia;
	pj_status_t status;

	/* Create response to be forwarded upstream (Via will be stripped here) */
	status = pjsip_endpt_create_response_fwd(app.sip_endpt, rdata, 0, &tdata);
	if (status != PJ_SUCCESS) {
		app_perror("Error creating response", status);
		return PJ_TRUE;
	}

	/* Get topmost Via header */
	hvia = (pjsip_via_hdr*)pjsip_msg_find_hdr(tdata->msg, PJSIP_H_VIA, NULL);
	if (hvia == NULL) {
		/* Invalid response! Just drop it */
		pjsip_tx_data_dec_ref(tdata);
		return PJ_TRUE;
	}

	/* Calculate the address to forward the response */
	pj_bzero(&res_addr, sizeof(res_addr));
	res_addr.dst_host.type = PJSIP_TRANSPORT_UDP;
	res_addr.dst_host.flag =
		pjsip_transport_get_flag_from_type(PJSIP_TRANSPORT_UDP);

	/* Destination address is Via's received param */
	res_addr.dst_host.addr.host = hvia->recvd_param;
	if (res_addr.dst_host.addr.host.slen == 0) {
		/* Someone has messed up our Via header! */
		res_addr.dst_host.addr.host = hvia->sent_by.host;
	}

	/* Destination port is the rport */
	if (hvia->rport_param != 0 && hvia->rport_param != -1)
		res_addr.dst_host.addr.port = hvia->rport_param;

	if (res_addr.dst_host.addr.port == 0) {
		/* Ugh, original sender didn't put rport!
		* At best, can only send the response to the port in Via.
		*/
		res_addr.dst_host.addr.port = hvia->sent_by.port;
	}

	/* Forward response */
	status = pjsip_endpt_send_response(app.sip_endpt, &res_addr, tdata,
		NULL, NULL);
	if (status != PJ_SUCCESS) {
		app_perror("Error forwarding response", status);
		return PJ_TRUE;
	}

	return PJ_TRUE;
}

pjsip_module module_proxy =
{
	NULL, NULL,			    /* prev, next.		*/
	{ "module_proxy", 12 },	    /* Name.			*/
	-1,					/* Id			*/
	PJSIP_MOD_PRIORITY_UA_PROXY_LAYER,	/* Priority		*/
	NULL,				/* load()		*/
	NULL,				/* start()		*/
	NULL,				/* stop()		*/
	NULL,				/* unload()		*/
	&proxy_on_rx_request,		/* on_rx_request()	*/
	&proxy_on_rx_response,		/* on_rx_response()	*/
	NULL,				/* on_tx_request.	*/
	NULL,				/* on_tx_response()	*/
	NULL,				/* on_tsx_state()	*/
};
void tu_on_tsx_state(pjsip_transaction *tsx, pjsip_event *event)
{
	struct uac_data *uac_data;
	pj_status_t status;

	if (tsx->role == PJSIP_ROLE_UAS) {
		if (tsx->state == PJSIP_TSX_STATE_TERMINATED) {
			struct uas_data *uas_data;

			uas_data = (struct uas_data*) tsx->mod_data[mod_tu.id];
			if (uas_data->uac_tsx) {
				uac_data = (struct uac_data*)
					uas_data->uac_tsx->mod_data[mod_tu.id];
				uac_data->uas_tsx = NULL;
			}

		}
		return;
	}

	/* Get the data that we attached to the UAC transaction previously */
	uac_data = (struct uac_data*) tsx->mod_data[mod_tu.id];


	/* Handle incoming response */
	if (event->body.tsx_state.type == PJSIP_EVENT_RX_MSG) {

		pjsip_rx_data *rdata;
		pjsip_response_addr res_addr;
		pjsip_via_hdr *hvia;
		pjsip_tx_data *tdata;

		rdata = event->body.tsx_state.src.rdata;

		/* Do not forward 100 response for INVITE (we already responded
		* INVITE with 100)
		*/
		if (tsx->method.id == PJSIP_INVITE_METHOD &&
			rdata->msg_info.msg->line.status.code == 100)
		{
			return;
		}

		/* Create response to be forwarded upstream
		* (Via will be stripped here)
		*/
		status = pjsip_endpt_create_response_fwd(app.sip_endpt, rdata, 0,
			&tdata);
		if (status != PJ_SUCCESS) {
			app_perror("Error creating response", status);
			return;
		}

		/* Get topmost Via header of the new response */
		hvia = (pjsip_via_hdr*)pjsip_msg_find_hdr(tdata->msg, PJSIP_H_VIA,
			NULL);
		if (hvia == NULL) {
			/* Invalid response! Just drop it */
			pjsip_tx_data_dec_ref(tdata);
			return;
		}

		/* Calculate the address to forward the response */
		pj_bzero(&res_addr, sizeof(res_addr));
		res_addr.dst_host.type = PJSIP_TRANSPORT_UDP;
		res_addr.dst_host.flag =
			pjsip_transport_get_flag_from_type(PJSIP_TRANSPORT_UDP);

		/* Destination address is Via's received param */
		res_addr.dst_host.addr.host = hvia->recvd_param;
		if (res_addr.dst_host.addr.host.slen == 0) {
			/* Someone has messed up our Via header! */
			res_addr.dst_host.addr.host = hvia->sent_by.host;
		}

		/* Destination port is the rport */
		if (hvia->rport_param != 0 && hvia->rport_param != -1)
			res_addr.dst_host.addr.port = hvia->rport_param;

		if (res_addr.dst_host.addr.port == 0) {
			/* Ugh, original sender didn't put rport!
			* At best, can only send the response to the port in Via.
			*/
			res_addr.dst_host.addr.port = hvia->sent_by.port;
		}

		/* Forward response with the UAS transaction */
		pjsip_tsx_send_msg(uac_data->uas_tsx, tdata);

	}

	/* If UAC transaction is terminated, terminate the UAS as well.
	* This could happen because of:
	*	- timeout on the UAC side
	*  - receipt of 2xx response to INVITE
	*/
	if (tsx->state == PJSIP_TSX_STATE_TERMINATED && uac_data &&
		uac_data->uas_tsx)
	{

		pjsip_transaction *uas_tsx;
		struct uas_data *uas_data;

		uas_tsx = uac_data->uas_tsx;
		uas_data = (struct uas_data*) uas_tsx->mod_data[mod_tu.id];
		uas_data->uac_tsx = NULL;

		if (event->body.tsx_state.type == PJSIP_EVENT_TIMER) {

			/* Send 408/Timeout if this is an INVITE transaction, since
			* we must have sent provisional response before. For non
			* INVITE transaction, just destroy it.
			*/
			if (tsx->method.id == PJSIP_INVITE_METHOD) {

				pjsip_tx_data *tdata = uas_tsx->last_tx;

				tdata->msg->line.status.code = PJSIP_SC_REQUEST_TIMEOUT;
				tdata->msg->line.status.reason = pj_str("Request timed out");
				tdata->msg->body = NULL;

				pjsip_tx_data_add_ref(tdata);
				pjsip_tx_data_invalidate_msg(tdata);

				pjsip_tsx_send_msg(uas_tsx, tdata);

			}
			else {
				/* For non-INVITE, just destroy the UAS transaction */
				pjsip_tsx_terminate(uas_tsx, PJSIP_SC_REQUEST_TIMEOUT);
			}

		}
		else if (event->body.tsx_state.type == PJSIP_EVENT_RX_MSG) {

			if (uas_tsx->state < PJSIP_TSX_STATE_TERMINATED) {
				pjsip_msg *msg;
				int code;

				msg = event->body.tsx_state.src.rdata->msg_info.msg;
				code = msg->line.status.code;

				uac_data->uas_tsx = NULL;
				pjsip_tsx_terminate(uas_tsx, code);
			}
		}
	}
}
pjsip_module mod_tu =
{
	NULL, NULL,				/* prev, next.		*/
	{ "mod-transaction-user", 20 },	/* Name.		*/
	-1,					/* Id			*/
	PJSIP_MOD_PRIORITY_APPLICATION,	/* Priority		*/
	NULL,				/* load()		*/
	NULL,				/* start()		*/
	NULL,				/* stop()		*/
	NULL,				/* unload()		*/
	NULL,				/* on_rx_request()	*/
	NULL,				/* on_rx_response()	*/
	NULL,				/* on_tx_request.	*/
	NULL,				/* on_tx_response()	*/
	&tu_on_tsx_state,			/* on_tsx_state()	*/
};
