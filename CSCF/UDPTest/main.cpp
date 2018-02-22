#include <sys/types.h>
//#include <sys/socket.h>
//#include<pthread.h>
//#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
//#include<arpa/inet.h>
#include<memory>
#include<thread>
#include<string>
#include <sys/ipc.h>   
#include <sys/msg.h>   
#include <errno.h>   
#include<deque>
#include<assert.h>
#include"udpproxy.h"
#define RTP_SEND 1984
#define RTP_RECV 1983
using namespace std;
using namespace udpproxy;
struct msg_rtpproxy
{
	//msgtype功能设计：
	//1:申请端口0
	//2:申请端口1
	//3:释放端口
	long msgtype;
	char msgtext[100];
	//端口为0时，表示端口申请失败
	unsigned int port;
} m_rtpproxy;

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	int rtp_send_id = msgget(RTP_SEND, IPC_CREAT);
	int rtp_recv_id = msgget(RTP_RECV, IPC_CREAT);
	int ret;
	port_table p_table;
	deque<uint16_t> port_pool;
	for (uint16_t i = 20001; i <= 21000; i++)
	{
		port_pool.push_back(i);
	}
	while (1)
	{
		ret = msgrcv(rtp_recv_id, &m_rtpproxy, sizeof(struct msg_rtpproxy), 0, 0);
		assert(ret != -1);
		//主叫rtp转发端口，同时新建rtp转发进程
		if (m_rtpproxy.msgtype == 1)
		{
			string key(m_rtpproxy.msgtext);
			if (p_table.find(key) != p_table.end())
			{
				m_rtpproxy.port =p_table[key].second->port[0];
				ret = msgsnd(rtp_send_id, &m_rtpproxy, sizeof(struct msg_rtpproxy), 0);
				assert(ret != -1);
				continue;
			}
			//端口不够时的处理
			if (port_pool.size() < 2)
			{
				m_rtpproxy.port = 0;
				ret = msgsnd(rtp_send_id, &m_rtpproxy, sizeof(struct msg_rtpproxy), 0);
				assert(ret != -1);
				continue;
			}
			uint16_t porta, portb;
			porta = port_pool.front();
			port_pool.pop_front();
			portb= port_pool.front();
			port_pool.pop_front();
			new_thread(key, p_table, porta, portb);
			m_rtpproxy.port = porta;
			ret = msgsnd(rtp_send_id, &m_rtpproxy, sizeof(struct msg_rtpproxy), 0);
			assert(ret != -1);
		}
		//被叫的rtp转发端口
		else if (m_rtpproxy.msgtype == 2)
		{
			string key(m_rtpproxy.msgtext);
			//printf(key.c_str());
			if (p_table.find(key) == p_table.end())
			{
				m_rtpproxy.port =0;
				ret = msgsnd(rtp_send_id, &m_rtpproxy, sizeof(struct msg_rtpproxy), 0);
				assert(ret != -1);
				continue;
			}
			m_rtpproxy.port = p_table[key].second->port[1];
			ret = msgsnd(rtp_send_id, &m_rtpproxy, sizeof(struct msg_rtpproxy), 0);
			assert(ret != -1);
		}
		//释放端口
		else if (m_rtpproxy.msgtype == 3)
		{
			string key(m_rtpproxy.msgtext);
			auto port=kill_thread(key, p_table);
			if (port.first + port.second != 0)
			{
				port_pool.push_back(port.first);
				port_pool.push_back(port.second);
			}
		}
	}
	return 0;
}