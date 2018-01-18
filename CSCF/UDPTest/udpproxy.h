#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include<pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include<arpa/inet.h>
#include<memory>
#include<map>
namespace udpproxy
{
	using namespace std;
	extern void set_blocking(int sockfd);
	extern void set_nonblocking(int sockfd);
	struct proxy_control
	{
		unique_ptr<sockaddr_in> client_addr[2];
		int sock[2];
		uint16_t port[2];// , portb;
		bool shut;
		proxy_control(uint16_t pa, uint16_t pb) :port{ pa,pb }
		{
			
			client_addr[0].reset(new sockaddr_in());
			client_addr[1].reset(new sockaddr_in());
			shut = 0;
			sock[0] = bindsocket(pa);
			sock[1] = bindsocket(pb);
		}
		~proxy_control()
		{
			printf("pcdelete\n");
		}
	private:
		int bindsocket(uint16_t port)
		{
			struct sockaddr_in addr;
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
			int sock;

			if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
			{
				perror("socket");
				exit(1);
			}
			int enable = 1;
			if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
			{
				perror("setsockopt");
				exit(1);
			}
			if (bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
			{
				perror("bind");
				exit(1);
			}
			return sock;
		}

	};
	template <unsigned self, unsigned oppo = 1 - self>
	void rtp_proxy(shared_ptr<proxy_control> pcon)
	{
		char buff[512];
		int n=-1;
		socklen_t len = sizeof(sockaddr_in);
		set_nonblocking(pcon->sock[self]);
		while (n == -1)
		{
			n = recvfrom(pcon->sock[self], buff, 511, 0, (sockaddr *)pcon->client_addr[self].get(), &len);
			if (pcon->shut == 1)
			{
				printf("proxy_shut_without_connect:%d %d\n", self, pcon->port[self]);
				return;
			}
		}
		while (pcon->client_addr[oppo]->sin_addr.s_addr == 0)
		{
			n = recvfrom(pcon->sock[self], buff, 511, 0, (sockaddr *)pcon->client_addr[self].get(), &len);
			if (pcon->shut == 1) 
			{
				printf("proxy_shut_without_connect:%d %d\n", self, pcon->port[self]);
				return;
			}
		}
		set_blocking(pcon->sock[self]);
		printf("connect,%d %d\n", self, pcon->port[self]);
		while (!pcon->shut)
		{
			n = recvfrom(pcon->sock[self], buff, 511, 0, (sockaddr *)pcon->client_addr[self].get(), &len);
			n = sendto(pcon->sock[oppo], buff, n, 0, (sockaddr *)pcon->client_addr[oppo].get(), sizeof(sockaddr_in));
		}
		printf("proxy_shut:%d %d\n", self,pcon->port[self]);
	}
	using thread_control = decltype(make_pair(shared_ptr<thread>(), shared_ptr<proxy_control>()));
	using port_table = map<string, thread_control>;
	extern void call(shared_ptr<proxy_control> pcon);
	extern void new_thread(string& key, port_table &ptable, uint16_t porta, uint16_t portb);
	extern pair<uint16_t, uint16_t> kill_thread(string& key, port_table &ptable);


}