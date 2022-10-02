#include <stdio.h>
#include <Windows.h>

LPVOID pfWriteFile = NULL;
CREATE_PROCESS_DEBUG_INFO debugInfo;
BYTE chINT3 = 0xCC, chOrgByte = 0;

BOOL doCreateEvent(LPDEBUG_EVENT debugEvent) {
	printf("--- CreateEvent START ---\n");

	//取得函式庫與得到 WriteFile API 的位置
	HMODULE module = GetModuleHandleA("kernel32.dll");
	pfWriteFile = GetProcAddress(module, "WriteFile");

	printf("kernel32.dll -> %p\n", module);
	printf("\\->WriteFile -> %p\n", pfWriteFile);

	// 將 debugEvent 的 CreateProcessInfo 寫到 debugInfo，u.CreateProcessInfo 的結構為 CREATE_PROCESS_DEBUG_INFO。
	// https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-debug_event
	memcpy(&debugInfo, &debugEvent->u.CreateProcessInfo, sizeof(CREATE_PROCESS_DEBUG_INFO));

	
	// 讀入目標程序的 pfWriteFile 位置。
	BYTE arr[10];
	ReadProcessMemory(debugInfo.hProcess, pfWriteFile, arr , sizeof(BYTE)*10, NULL);
	// 用來還原 API 位置
	chOrgByte = arr[0]; 


	printf("API Address : ");
	for (BYTE i : arr) {
		printf("%02X", i);
	}

	WriteProcessMemory(debugInfo.hProcess, pfWriteFile, &chINT3, sizeof(BYTE), NULL);

	ReadProcessMemory(debugInfo.hProcess, pfWriteFile, arr, sizeof(arr), NULL);
	printf("\n           -> ");
	for (BYTE i : arr) {
		printf("%02X", i);
	}

	printf("\n--- CreateEvent DONE ---\n");
	return 1;
}

BOOL doExceptionEvent(LPDEBUG_EVENT debugEvent) {

	CONTEXT context;
	PBYTE pBuf = NULL;
	ULONG_PTR ulpWriteLen, ulpBufAddress;
	PEXCEPTION_RECORD64 debugRecord = (PEXCEPTION_RECORD64)&debugEvent->u.Exception.ExceptionRecord;
	BYTE arr[10];

	if (debugRecord->ExceptionCode == EXCEPTION_BREAKPOINT &&
		debugRecord->ExceptionAddress==(DWORD64)pfWriteFile
		) {
		printf("--- Find WriteFile Called! ---\n");
		printf("APIAddress : %p\n",(DWORD64)pfWriteFile);
		
		WriteProcessMemory(debugInfo.hProcess, pfWriteFile, &chOrgByte, sizeof(BYTE), NULL);

		ReadProcessMemory(debugInfo.hProcess, pfWriteFile, arr, sizeof(arr), NULL);
		for (BYTE i : arr) {
			printf("%02X", i);
		}
		printf("\n");

		context.ContextFlags = CONTEXT_FULL;
		GetThreadContext(debugInfo.hThread,&context);

		LPOVERLAPPED arg5_lpoverlapped = NULL;
		ReadProcessMemory(debugInfo.hProcess, (LPVOID)(context.Rsp + 0x28), &arg5_lpoverlapped, sizeof(DWORD), NULL);
		
		printf("\nRegister data :\n");
		printf("RAX :%p\n", context.Rax);
		printf("RBX :%p\n", context.Rbx);
		printf("RCX :%p\n", context.Rcx);
		printf("RDX :%p\n", context.Rdx);
		printf("R8  :%p\n", context.R8);
		printf("R9  :%p\n", context.R9);
		printf("arg5:%p\n\n", arg5_lpoverlapped);

		ulpBufAddress = context.Rdx;
		ulpWriteLen = context.R8;

		pBuf = (PBYTE)malloc(ulpWriteLen+1);
		memset(pBuf,0,ulpWriteLen+1);

		ReadProcessMemory(debugInfo.hProcess,(LPVOID)ulpBufAddress,pBuf,ulpWriteLen,NULL);
		printf("Org String :\n%s\n",pBuf);
		
		for (unsigned int i = 0; i < (unsigned int)ulpWriteLen; i++) {
			if ('a' <= pBuf[i] && pBuf[i] <= 'z') {
				pBuf[i] -= 0x20;
			}
		}

		printf("Aft String :\n%s\n", pBuf);
		
		WriteProcessMemory(debugInfo.hProcess,(LPVOID)ulpBufAddress,pBuf,ulpWriteLen,NULL);
		free(pBuf);
		
		context.Rip = (DWORD64)pfWriteFile;
		SetThreadContext(debugInfo.hThread,&context);

		ContinueDebugEvent(debugEvent->dwProcessId,debugEvent->dwThreadId,DBG_CONTINUE);
		
		
		WriteProcessMemory(debugInfo.hProcess, pfWriteFile, &chINT3, sizeof(BYTE),NULL);
		
		printf("-- - Write Change end-- - \n");
	}
	return 1;
}

BOOL StayDebugEvent() {

	DEBUG_EVENT debugEvent;
	DWORD dwStat;
	while (WaitForDebugEvent(&debugEvent, INFINITE)) {
		dwStat = DBG_CONTINUE;
		if (debugEvent.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT) {
			doCreateEvent(&debugEvent);
		}
		else if (debugEvent.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
		{
			doExceptionEvent(&debugEvent);
		}
		else if (debugEvent.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT) {
			printf("Process %d is down!\n", debugEvent.dwProcessId);
			break;
		}

		ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, dwStat);
	}

	return 1;
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("USEAGE : apiHooker.exe <PID> \n");
		system("tasklist | findstr notepad");
		return 1;
	}

	DWORD dwPID = atol(argv[1]);
	if (!dwPID) {
		printf("PID error!");
		return 0;
	}
	printf("--- PID : %d ---\n", dwPID);
	if (!DebugActiveProcess(dwPID)) {
		printf("Debug Fail");
		printf("Error code : %d", GetLastError());
		return 1;
	}
	StayDebugEvent();
	return 0;

}