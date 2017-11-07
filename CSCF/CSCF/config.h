#pragma once
#include "pj.h"
struct network_config_t
{
	int				sip_af;
	int				sip_port;
	pj_bool_t		sip_tcp;
} network_config;

