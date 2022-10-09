#include <iostream>
#include <Windows.h>


int main() {

	UINT64 T1 = __rdtsc();
	for (int i = 0; i < 50; i++) {}
	UINT64 T2 = __rdtsc();

	UINT64 delta = T2 - T1;
	printf("Time = %lld\n",delta);
	if (delta > 0xFFFF) {
		printf("OUT!\n");
		system("pause");
		return 0;
	}
	printf("SAVE~ : )\n");
	system("pause");
	return 0;
}