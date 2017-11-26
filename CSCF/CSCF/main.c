#include "pj.h"
#include "cscf.h"
int main(int argc, char *argv[])
{
	char a[10];
	init_stack();
	while (1)
	{
		scanf("%s",a);
		if (strcmp(a, "exit")==0)break;
	}
	destroy_stack();
	return 0;
}



