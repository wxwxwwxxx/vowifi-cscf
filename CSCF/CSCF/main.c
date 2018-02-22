#include "pj.h"
#include "cscf.h"
int main(int argc, char *argv[])
{
	char a[10];
	//初始化协议栈
	init_stack();
	while (1)
	{
		scanf("%s",a);
		//输入exit则退出主函数（一般不需要使用，直接kill该进程即可）
		if (strcmp(a, "exit")==0)break;
	}
	//销毁协议栈
	destroy_stack();
	return 0;
}



