#pragma once
#include "pj.h"
//保存app相关设置
struct network_config_t
{
	int				sip_af;
	int				sip_port;
	pj_bool_t		sip_tcp;
} network_config;

//enum contact_op
//{
//	NONE,	/* don't put Contact header	    */
//	EXACT,	/* return exact contact		    */
//	MODIFIED,	/* return modified Contact header   */
//};
struct registrar_cfg
{
	pj_bool_t	    respond;	    /* should it respond at all		*/
	unsigned	    status_code;    /* final response status code	*/
	pj_bool_t	    authenticate;   /* should we authenticate?		*/
	//enum contact_op contact_op;	    /* What should we do with Contact   */
	//unsigned	    expires_param;  /* non-zero to put in expires param	*/
	//unsigned	    expires;	    /* non-zero to put in Expires header*/
	pj_str_t	    more_contacts;  /* Additional Contact headers to put*/
}registrar_config;
extern pj_status_t init_config();
