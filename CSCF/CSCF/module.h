#pragma once
/*各个模块公用该头文件
*添加新模块时请按以下步骤：
*1、建立模块的c文件
*2、在该头文件中声明模块及模块的函数
*3、在register_module函数中添加模块的注册
*/
#include "pj.h"
//注册模块
extern pjsip_module module_registrar;
//log模块
extern pj_bool_t logging_on_rx_msg(pjsip_rx_data *rdata);
extern pj_status_t logging_on_tx_msg(pjsip_tx_data *tdata);
extern pjsip_module module_log;
//proxy模块

struct uac_data
{
	pjsip_transaction	*uas_tsx;
	pj_timer_entry	 timer;
};


struct uas_data
{
	pjsip_transaction	*uac_tsx;
};
extern pj_bool_t proxy_on_rx_request(pjsip_rx_data * rdata);

extern pj_bool_t proxy_on_rx_response(pjsip_rx_data * rdata);

extern void tu_on_tsx_state(pjsip_transaction * tsx, pjsip_event * event);

extern pjsip_module module_proxy;
extern pjsip_module mod_tu;
//模块注册函数
extern pj_status_t register_module();