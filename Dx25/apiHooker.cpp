#include <stdio.h>
#include <Windows.h>

LPVOID pfWriteFile = NULL;
CREATE_PROCESS_DEBUG_INFO debugInfo;
BYTE chINT3 = 0xCC, chOrgByte = 0;

BOOL doCreateEvent(LPDEBUG_EVENT debugEvent) {
	printf("--- CreateEvent START ---\n");
	HMODULE module = GetModuleHandleA("kernel32.dll");
	pfWriteFile = GetProcAddress(module, "WriteFile");

	printf("kernel32.dll -> %p\n", module);
	printf("\\->WriteFile -> %p\n", pfWriteFile);

	memcpy(&debugInfo, &debugEvent->u.CreateProcessInfo, sizeof(CREATE_PROCESS_DEBUG_INFO));

	ReadProcessMemory(debugInfo.hProcess, pfWriteFile, &chOrgByte, sizeof(BYTE), NULL);

	printf("API Address : %p", chOrgByte);

	WriteProcessMemory(debugInfo.hProcess, pfWriteFile, &chINT3, sizeof(BYTE), NULL);

	BYTE arr[10];
	ReadProcessMemory(debugInfo.hProcess, pfWriteFile, arr, sizeof(arr), NULL);

	printf(" -> ");
	for (BYTE i : arr) {
		printf("%02X", i);
	}
	printf("\n--- CreateEvent DONE ---\n");
	return 1;
}

BOOL doExceptionEvent(LPDEBUG_EVENT debugEvent) {



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