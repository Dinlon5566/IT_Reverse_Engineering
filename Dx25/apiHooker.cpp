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

	// 將第一個 BYTE 改成 0xCC
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

	// 確認 Exception 為 WriteFile。
	if (debugRecord->ExceptionCode == EXCEPTION_BREAKPOINT &&
		debugRecord->ExceptionAddress==(DWORD64)pfWriteFile
		) {
		printf("--- Find WriteFile Called! ---\n");
		printf("APIAddress : %p\n",(DWORD64)pfWriteFile);
		
		// 把之前改過的 API 位置改回去
		WriteProcessMemory(debugInfo.hProcess, pfWriteFile, &chOrgByte, sizeof(BYTE), NULL);

		// 輸出 API 位置
		ReadProcessMemory(debugInfo.hProcess, pfWriteFile, arr, sizeof(arr), NULL);
		for (BYTE i : arr) {
			printf("%02X", i);
		}
		printf("\n");

		// 取得 Register
		context.ContextFlags = CONTEXT_FULL;
		GetThreadContext(debugInfo.hThread,&context);

		// 輸出 Register，方便查看
		printf("\nRegister data :\n");
		printf("RAX :%p\n", context.Rax);
		printf("RBX :%p\n", context.Rbx);
		printf("RCX :%p\n", context.Rcx);
		printf("RDX :%p\n", context.Rdx);
		printf("R8  :%p\n", context.R8);

		// 將傳遞值由寄存器撈上來
		ulpBufAddress = context.Rdx;
		ulpWriteLen = context.R8;

		// 宣告緩衝區空間與清空
		pBuf = (PBYTE)malloc(ulpWriteLen+1);
		memset(pBuf,0,ulpWriteLen+1);

		// 讀取文字位置的內容到緩衝區
		ReadProcessMemory(debugInfo.hProcess,(LPVOID)ulpBufAddress,pBuf,ulpWriteLen,NULL);
		printf("Org String :\n%s\n",pBuf);
		
		// 把字都變成大寫
		for (unsigned int i = 0; i < (unsigned int)ulpWriteLen; i++) {
			if ('a' <= pBuf[i] && pBuf[i] <= 'z') {
				pBuf[i] -= 0x20;
			}
		}

		printf("Aft String :\n%s\n", pBuf);
		
		// 將緩衝區內容存到原本放字串的位置
		WriteProcessMemory(debugInfo.hProcess,(LPVOID)ulpBufAddress,pBuf,ulpWriteLen,NULL);

		// 你沒用了
		free(pBuf);
		
		// 把執行位置調到 WriteFile 的位置
		context.Rip = (DWORD64)pfWriteFile;
		// 然後把 Context 的資料設置給 Thread
		SetThreadContext(debugInfo.hThread,&context);

		// 原程序可以繼續行動了
		ContinueDebugEvent(debugEvent->dwProcessId,debugEvent->dwThreadId,DBG_CONTINUE);
		
		// 把 WriteFile 的第一 BYTE 改回 0xCC
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
		printf("Debug Fail\n");
		printf("Error code : %d", GetLastError());
		return 1;
	}
	StayDebugEvent();
	return 0;

}