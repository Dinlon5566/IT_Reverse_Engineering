#include <iostream>
using namespace std;
int main()
{
	printf("STOP\n");
	__asm {
		int 0x03
	}
	return 0;
	
}