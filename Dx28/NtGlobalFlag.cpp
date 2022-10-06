
#include <iostream>
#include <Windows.h>
DWORD isDebuger()
{
    __asm
    {
        mov eax, fs: [0x30]
        movzx eax, dword ptr[eax + 0x68]
    }
}

int main()
{
    if (isDebuger()) {
        printf("Is Debuger mode!");
        return 0;
    }
    MessageBoxA(0, "Hello world", "Dinlon", NULL);
}

