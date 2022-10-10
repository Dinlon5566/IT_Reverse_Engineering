#include <iostream>
#include <Windows.h>
void foundDebugger() {
    MessageBox(0, L"Found Debugger", L"TF", 0);
}
int main()
{
    __asm {
        // 載入 SEH
        push handler
        push DWORD ptr fs : [0]
        mov DWORD ptr fs : [0] , esp
        xor eax,eax
        // 觸發例外
        int 0x2d

        // 如果執行這行會使 EAX 變成 1
        mov EAX, 0x1
 
        // 確認 EAX 的值
        Continue:
        cmp EAX,0x0
        je normal_code

        // 發現 Debuger
        call foundDebugger
        mov eax,0xFFFFFFFF
        jmp eax 
        
     handler :
        // 處裡例外
        mov eax, dword ptr ss : [esp + 0xc]
        mov dword ptr ds : [eax + 0xb8] , offset Continue
        xor eax,eax  // 設定 EAX=0
        retn

    normal_code :
        // 移除 SEH
        pop dword ptr fs : [0]
        add esp, 4
       
    }
    printf("SAVE\n");
    return 0;
}

