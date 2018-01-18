#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include<memory>
#include<thread>
#include<map>
#include"udpproxy.h"
#include<fcntl.h>
namespace udpproxy
{
	using namespace std;
	void call(shared_ptr<proxy_control> pcon)
	{
		thread a(rtp_proxy<0>, pcon);
		a.detach();
		thread b(rtp_proxy<1>, pcon);
		b.detach();
		while (!pcon->shut);
		printf("shut\n");
	}
	void new_thread(string& key, port_table &ptable, uint16_t porta, uint16_t portb)
	{
		auto pcon = make_shared<proxy_control>(porta, portb);
		auto tcon = make_shared<thread>(call, pcon);
		auto ptpair = make_pair(tcon, pcon);
		ptable[key] = std::move(ptpair);
	}
	pair<uint16_t,uint16_t> kill_thread(string& key, port_table &ptable)
	{
		if (ptable.find(key) == ptable.end())
			return make_pair(0u, 0u);
		ptable[key].second->shut = 1;
		ptable[key].first->join();
		auto ret = make_pair(ptable[key].second->port[0], ptable[key].second->port[1]);
		close(ret.first);
		close(ret.second);
		ptable.erase(key);
		printf("kill_thread\n");
		return ret;
	}
	void set_nonblocking(int sockfd) {
		int flag = fcntl(sockfd, F_GETFL, 0);
		if (flag < 0) {
			perror("fcntl F_GETFL fail");
			return;
		}
		if (fcntl(sockfd, F_SETFL, flag | O_NONBLOCK) < 0) {
			perror("fcntl F_SETFL fail");
		}
	}
	void set_blocking(int sockfd) {
		int flag = fcntl(sockfd, F_GETFL, 0);
		if (flag < 0) {
			perror("fcntl F_GETFL fail");
			return;
		}
		if (fcntl(sockfd, F_SETFL, 0) < 0) {
			perror("fcntl F_SETFL fail");
		}
	}
}
