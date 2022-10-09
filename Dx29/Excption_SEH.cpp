#include <iostream>
#include "windows.h"

void debugerFound() {
	MessageBox(0, L"Debuger Found", L"Excption_SEH", 0);
}
int main()
{
	printf("Start");
	__asm {
		push handler
		push DWORD PTR fs : [0]
		mov DWORD PTR fs:[0], esp

		int 0x03

		call debugerFound
		mov eax,0xFFFFFFFF

		jmp eax

	handler:
		mov eax, dword ptr ss : [esp + 0xc]
		mov ebx, normal_code
		mov dword ptr ds : [eax + 0xb8] , ebx
		xor eax, eax
		retn

	normal_code :
		pop dword ptr fs : [0]
		add esp, 4

	}
	MessageBox(0, L"Save", L"Excption_SEH", 0);
}
