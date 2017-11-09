#include "module.h"
#include "cscf.h"
#include "pj.h"
pjsip_module module_registrar =
{
	NULL, NULL,			    /* prev, next.		*/
	{ "module_registrar", 16 },	    /* Name.			*/
	-1,				    /* Id			*/
	PJSIP_MOD_PRIORITY_APPLICATION, /* Priority			*/
	NULL,			    /* load()			*/
	NULL,			    /* start()			*/
	NULL,			    /* stop()			*/
	NULL,			    /* unload()			*/
	NULL,		    /* on_rx_request()		*/
	NULL,			    /* on_rx_response()		*/
	NULL,			    /* on_tx_request.		*/
	NULL,			    /* on_tx_response()		*/
	NULL,			    /* on_tsx_state()		*/
};