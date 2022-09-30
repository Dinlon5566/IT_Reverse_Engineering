// this code is from www.reversecore.com
// Author : reversecore@gmail.com
#include <iostream>
#include <Windows.h>

typedef struct _THREAD_PARAM
{
    FARPROC pFunc[2];               // LoadLibraryA(), GetProcAddress()
    char    szBuf[4][128];          // "user32.dll", "MessageBoxA", "www.reversecore.com", "ReverseCore"
} THREAD_PARAM, * PTHREAD_PARAM;


typedef HMODULE(WINAPI* PFLOADLIBRARYA)
(
    LPCSTR lpLibFileName
    );

typedef FARPROC(WINAPI* PFGETPROCADDRESS)
(
    HMODULE hModule,
    LPCSTR lpProcName
    );

typedef int (WINAPI* PFMESSAGEBOXA)
(
    HWND hWnd,
    LPCSTR lpText,
    LPCSTR lpCaption,
    UINT uType
    );

DWORD WINAPI ThreadProc(LPVOID lParam)
{
    PTHREAD_PARAM   pParam = (PTHREAD_PARAM)lParam;
    HMODULE         hMod = NULL;
    FARPROC         pFunc = NULL;

    // LoadLibrary()
    hMod = ((PFLOADLIBRARYA)pParam->pFunc[0])(pParam->szBuf[0]);    //  LoadLibraryA("user32.dll");
    if (!hMod)
        return 1;

    // GetProcAddress()
    pFunc = (FARPROC)((PFGETPROCADDRESS)pParam->pFunc[1])(hMod, pParam->szBuf[1]);  // GetProcAddress("MessageBoxA");
    if (!pFunc)
        return 1;

    // MessageBoxA()
    ((PFMESSAGEBOXA)pFunc)(NULL, pParam->szBuf[2], pParam->szBuf[3], MB_OK);  // MessageBoxA(NULL,"Hello","IT",NULL);

    return 0;
}

BOOL InjectCode(DWORD dwPID)
{
    HMODULE         hMod = NULL;
    THREAD_PARAM    param = { 0, };
    HANDLE          hProcess = NULL;
    HANDLE          hThread = NULL;
    LPVOID          pRemoteBuf[2] = { 0, };
    DWORD           dwSize = 0;

    hMod = GetModuleHandleA("kernel32.dll");
    if (hMod == NULL) {
        printf("GetModeuleHandle fail : error code [%d]\n",GetLastError());
    }

    // set THREAD_PARAM
    param.pFunc[0] = GetProcAddress(hMod, "LoadLibraryA");
    param.pFunc[1] = GetProcAddress(hMod, "GetProcAddress");
    strcpy_s(param.szBuf[0], "user32.dll");
    strcpy_s(param.szBuf[1], "MessageBoxA");
    strcpy_s(param.szBuf[2], "Hello \\OwO/");
    strcpy_s(param.szBuf[3], "IT-help");

    // Open Process
    if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS,   // dwDesiredAccess
        FALSE,                // bInheritHandle
        dwPID)))             // dwProcessId
    {
        printf("OpenProcess() fail : err_code = %d\n", GetLastError());
        return FALSE;
    }

    // Allocation for THREAD_PARAM
    dwSize = sizeof(THREAD_PARAM);
    if (!(pRemoteBuf[0] = VirtualAllocEx(hProcess,          // hProcess
        NULL,                 // lpAddress
        dwSize,               // dwSize
        MEM_COMMIT,           // flAllocationType
        PAGE_READWRITE)))    // flProtect
    {
        printf("VirtualAllocEx() fail : err_code = %d\n", GetLastError());
        return FALSE;
    }

    if (!WriteProcessMemory(hProcess,                       // hProcess
        pRemoteBuf[0],                  // lpBaseAddress
        (LPVOID)&param,                 // lpBuffer
        dwSize,                         // nSize
        NULL))                         // [out] lpNumberOfBytesWritten
    {
        printf("WriteProcessMemory() fail : err_code = %d\n", GetLastError());
        return FALSE;
    }

    // Allocation for ThreadProc()
    dwSize = (DWORD)InjectCode - (DWORD)ThreadProc;
    if (!(pRemoteBuf[1] = VirtualAllocEx(hProcess,          // hProcess
        NULL,                 // lpAddress
        dwSize,               // dwSize
        MEM_COMMIT,           // flAllocationType
        PAGE_EXECUTE_READWRITE)))    // flProtect
    {
        printf("VirtualAllocEx() fail : err_code = %d\n", GetLastError());
        return FALSE;
    }

    if (!WriteProcessMemory(hProcess,                       // hProcess
        pRemoteBuf[1],                  // lpBaseAddress
        (LPVOID)ThreadProc,             // lpBuffer
        dwSize,                         // nSize
        NULL))                         // [out] lpNumberOfBytesWritten
    {
        printf("WriteProcessMemory() fail : err_code = %d\n", GetLastError());
        return FALSE;
    }

    if (!(hThread = CreateRemoteThread(hProcess,            // hProcess
        NULL,                // lpThreadAttributes
        0,                   // dwStackSize
        (LPTHREAD_START_ROUTINE)pRemoteBuf[1],     // dwStackSize
        pRemoteBuf[0],       // lpParameter
        0,                   // dwCreationFlags
        NULL)))             // lpThreadId
    {
        printf("CreateRemoteThread() fail : err_code = %d\n", GetLastError());
        return FALSE;
    }

    // wait thread close
    WaitForSingleObject(hThread, INFINITE);

    // clode the Handle
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return TRUE;
}




int main(int argc,char* argv[])
{
	if (argc != 2) {
		printf("USEAGE : Injecter.exe <PID>\n");
		return 1;
	}
	DWORD pid = atol(argv[1]);
    InjectCode(pid);

	return 0;
}

