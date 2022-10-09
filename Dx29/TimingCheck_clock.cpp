
#include <iostream>
#include <Windows.h>
#include <time.h>
int main()
{
	clock_t start, finish;
	printf("Clock start\n");
	start = clock();
	
	for (int i = 0; i < INT_MAX; i++) {
	
	}

	printf("Clock Stop\n");
	finish = clock();
	printf("%lf\n",double (finish - start)/CLOCKS_PER_SEC);
	MessageBox(0,L"",L"",0);
}

