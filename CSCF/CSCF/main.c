#include "pj.h"
#include "cscf.h"
int main(int argc, char *argv[])
{
	char a[10];
	//��ʼ��Э��ջ
	init_stack();
	while (1)
	{
		scanf("%s",a);
		//����exit���˳���������һ�㲻��Ҫʹ�ã�ֱ��kill�ý��̼��ɣ�
		if (strcmp(a, "exit")==0)break;
	}
	//����Э��ջ
	destroy_stack();
	return 0;
}



