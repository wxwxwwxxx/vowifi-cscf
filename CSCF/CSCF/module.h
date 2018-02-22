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
typedef struct  {
	pj_str_t user;
	pj_str_t host;
	int port;
	pj_int32_t expires;
	int valid;
}IPC_userinfo;
extern IPC_userinfo ui;
extern pj_status_t register_module();
//路由模块
extern pjsip_module module_route;
extern pj_status_t set_chart();
extern pj_status_t get_chart(const char* key);
extern char* pj_strdup4(pj_pool_t* pool, const pj_str_t * pjstr);
//RTP转发模块
extern pjsip_module module_rtpproxy;
extern void set_m_rtpproxy(long mtype, pj_str_t* str);
extern pj_uint16_t get_port(long mtype, pj_str_t* str);
typedef struct 
{
	//msgtype定义：
	//1:申请端口0
	//2:申请端口1（由于是转发，需要两个端口）
	//3:释放端口
	long msgtype;
	char msgtext[100];
	//用于返回端口
	pj_uint16_t port;
}msg_rtpproxy;//
extern msg_rtpproxy m_rtpproxy;
