#pragma once
//包含PJSIP头文件与lib模块
#include "define.h"
#include <stdio.h>
//#include <tchar.h>
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

#pragma comment(lib,"pjlib-i386-win32-vc14-debug.lib")
#pragma comment(lib,"pjlib-util-i386-win32-vc14-debug.lib")
#pragma comment(lib,"pjmedia-audiodev-i386-win32-vc14-debug.lib")
#pragma comment(lib,"pjmedia-codec-i386-win32-vc14-debug.lib")
#pragma comment(lib,"pjmedia-i386-win32-vc14-debug.lib")
#pragma comment(lib,"pjnath-i386-win32-vc14-debug.lib")
#pragma comment(lib,"pjsip-core-i386-win32-vc14-debug.lib")
#pragma comment(lib,"pjsip-simple-i386-win32-vc14-debug.lib")
#pragma comment(lib,"pjsip-ua-i386-win32-vc14-debug.lib")
#pragma comment(lib,"pjsua-lib-i386-win32-vc14-debug.lib")
#pragma comment(lib,"libresample-i386-win32-vc14-debug.lib")
#pragma comment(lib,"libspeex-i386-win32-vc14-debug.lib")
#pragma comment(lib,"libsrtp-i386-win32-vc14-debug.lib")
#pragma comment(lib,"libg7221codec-i386-win32-vc14-debug.lib")
#pragma comment(lib,"libgsmcodec-i386-win32-vc14-debug.lib")
#pragma comment(lib,"libilbccodec-i386-win32-vc14-debug.lib")
#pragma comment(lib,"pjsua2-lib-i386-Win32-vc14-Debug.lib")

#endif

#ifdef NDEBUG

#pragma comment(lib,"pjlib-i386-win32-vc14-release.lib")
#pragma comment(lib,"pjlib-util-i386-win32-vc14-release.lib")
#pragma comment(lib,"pjmedia-audiodev-i386-win32-vc14-release.lib")
#pragma comment(lib,"pjmedia-codec-i386-win32-vc14-release.lib")
#pragma comment(lib,"pjmedia-i386-win32-vc14-release.lib")
#pragma comment(lib,"pjnath-i386-win32-vc14-release.lib")
#pragma comment(lib,"pjsip-core-i386-win32-vc14-release.lib")
#pragma comment(lib,"pjsip-simple-i386-win32-vc14-release.lib")
#pragma comment(lib,"pjsip-ua-i386-win32-vc14-release.lib")
#pragma comment(lib,"pjsua-lib-i386-win32-vc14-release.lib")
#pragma comment(lib,"libresample-i386-win32-vc14-release.lib")
#pragma comment(lib,"libspeex-i386-win32-vc14-release.lib")
#pragma comment(lib,"libsrtp-i386-win32-vc14-release.lib")
#pragma comment(lib,"libg7221codec-i386-win32-vc14-release.lib")
#pragma comment(lib,"libgsmcodec-i386-win32-vc14-release.lib")
#pragma comment(lib,"libilbccodec-i386-win32-vc14-release.lib")
#pragma comment(lib,"libmilenage-i386-win32-vc14-release.lib")
#pragma comment(lib,"libportaudio-i386-win32-vc14-release.lib")

#endif
#endif