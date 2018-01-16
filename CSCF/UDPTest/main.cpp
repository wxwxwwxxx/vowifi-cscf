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
#include<thread>
#include"udpproxy.h"
//#include"threadcontrol.cpp"
using namespace std;
using namespace udpproxy;
int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	
	auto pcon = make_shared<proxy_control>(20001,20002);
	auto tcon = make_shared<thread>(call, pcon);
	auto ptpair = make_pair(tcon, pcon);
	//this_thread::sleep_for(chrono::seconds(60));
	//ptpair.second->shut = 1;
	//ptpair.first->join();
	while (1);
	return 0;
}