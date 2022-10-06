#include <iostream>
#include <Windows.h>
BOOL isCRDP() {
    BOOL dbg;
    if (CheckRemoteDebuggerPresent(GetCurrentProcess(), &dbg) && dbg) {
        return 1;
    }
    return 0;

}
int main()
{
    if (isCRDP()) {
        printf("Is Debuger mode!");
        return 0;
    }
    MessageBoxA(0, "Hello world", "Dinlon", NULL);
}