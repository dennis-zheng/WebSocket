#include <stdio.h>
#include <windows.h>
#include "serverManager.h"

int main()
{
	ServerManager serverManager;
	serverManager.init();
	int index = 0;
	while (true)
	{
		printf("main index=%d\n", index++);
		Sleep(10*1000);
	}
	return 0;
}
