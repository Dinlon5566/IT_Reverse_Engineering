#include <iostream>
#include <Windows.h>
void foundDebugger() {
    MessageBox(0, L"Found Debugger", L"TF", 0);
}
int main()
{
    printf("Start\n");
    __asm {
        // 設定 SEH
        push handler
        push DWORD ptr fs : [0]
        mov DWORD ptr fs : [0] , esp

        // 由於無法直接設置 TF，所以把 EFLAGS 抓出來後把第 9 bit 設為 1 再存進 EFLAGS
        pushfd
        // 0x100=100000000b
        or dword ptr ss : [esp] , 0x100
        popfd

        // 這邊會觸發 TF 引發例外拋出。
        nop

        // 若是例外被處裡或是使用單步模式，就會跑來這邊。
        call foundDebugger
        mov eax, 0xFFFFFFFF
        jmp eax

        // 例外處裡器
    handler :
        mov eax, dword ptr ss : [esp + 0xc]
        mov dword ptr ds : [eax + 0xb8] , offset normal_code
        xor eax, eax
        retn

        // 移除 SEH
        normal_code :
        pop dword ptr fs : [0]
        add esp, 4

    }
    MessageBox(0, L"SAVE", L"TF", 0);
}