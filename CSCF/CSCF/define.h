#pragma once
//宏定义的设置

#define THIS_FILE   "VOWIFI_CSCF"
//#define PJ_SERVER_TEST 
#ifdef PJ_SERVER_TEST
	#define PJ_SERVER_ADDRESS "59.110.217.175"
	#define PJ_SERVER_PORT 9170;
#else
	#define PJ_SERVER_PORT 5060;
#endif // DEBUG

