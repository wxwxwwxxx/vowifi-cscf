#pragma once
#include "pj.h"
extern pj_bool_t logging_on_rx_msg(pjsip_rx_data *rdata);
extern pj_status_t logging_on_tx_msg(pjsip_tx_data *tdata);
extern pjsip_module module_log;