
#include <iostream>
#include <Windows.h>
int main()
{
    if (IsDebuggerPresent()) {
        printf("Is Debuger mode!");
        return 0;
    }
    MessageBoxA(0,"Hello world","Dinlon",NULL);
}


