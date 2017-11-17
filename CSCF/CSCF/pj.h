#pragma once
//包含PJSIP头文件与lib模块
#include "define.h"
#include <stdio.h>
#include <pjsip.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjsip_ua.h>
#include <pjsip_simple.h>
#include <pjlib-util.h>
#include <pjlib.h>

//常用宏定义工具

#define CHECK_STATUS()	do { if (status != PJ_SUCCESS) return status; } while (0)
//程序相关头文件


//win32下的lib处理
#ifdef PJ_WIN32

#ifdef _DEBUG

#pragma comment(lib,"pjlib-x86_64-x64-vc14-debug.lib")
#pragma comment(lib,"pjlib-util-x86_64-x64-vc14-debug.lib")
#pragma comment(lib,"pjmedia-audiodev-x86_64-x64-vc14-debug.lib")
#pragma comment(lib,"pjmedia-codec-x86_64-x64-vc14-debug.lib")
#pragma comment(lib,"pjmedia-x86_64-x64-vc14-debug.lib")
#pragma comment(lib,"pjnath-x86_64-x64-vc14-debug.lib")
#pragma comment(lib,"pjsip-core-x86_64-x64-vc14-debug.lib")
#pragma comment(lib,"pjsip-simple-x86_64-x64-vc14-debug.lib")
#pragma comment(lib,"pjsip-ua-x86_64-x64-vc14-debug.lib")
#pragma comment(lib,"pjsua-lib-x86_64-x64-vc14-debug.lib")
#pragma comment(lib,"pjsua2-lib-x86_64-x64-vc14-Debug.lib")
#pragma comment(lib,"pjmedia-videodev-x86_64-x64-vc14-Debug.lib")
#endif
#endif