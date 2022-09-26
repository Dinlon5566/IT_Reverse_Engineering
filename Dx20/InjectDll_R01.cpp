
#include <iostream>
#include <Windows.h>
#include "loaderX64.h"
#define WIN32_LEAN_AND_MEAN

typedef LONG(NTAPI* RTLCREATEUSERTHREAD)(HANDLE, PSECURITY_DESCRIPTOR, BOOLEAN, ULONG, SIZE_T, SIZE_T, PTHREAD_START_ROUTINE, PVOID, PHANDLE, LPVOID);

BOOL ReadFileData(WCHAR* filename, BYTE** buff, DWORD* size)
{
	if (filename == NULL || buff == NULL || size == NULL)
		return FALSE;

	HANDLE hFile = CreateFileW(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;

	// get size of file
	LARGE_INTEGER liSize;
	if (!GetFileSizeEx(hFile, &liSize)) {
		CloseHandle(hFile);
		return FALSE;
	}
	if (liSize.HighPart > 0) {
		CloseHandle(hFile);
		return FALSE;
	}

	// read entire file into memory
	*buff = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, liSize.LowPart);
	if (*buff == NULL) {
		CloseHandle(hFile);
		return FALSE;
	}

	BYTE* buffPtr = *buff;
	DWORD numLeft = liSize.LowPart;
	DWORD numRead = 0;

	while (numLeft > 0) {
		if (!ReadFile(hFile, buffPtr, numLeft, &numRead, NULL)) {
			CloseHandle(hFile);
			HeapFree(GetProcessHeap(), 0, *buff);
			return FALSE;
		}
		numLeft -= numRead;
		buffPtr += numRead;
	}
	*size = liSize.LowPart;
	CloseHandle(hFile);
	return TRUE;
}

int wmain(int argc, WCHAR* argv[])
{	
	if (argc !=3) {
		wprintf(L"usage: InjectDll.exe <X64DLL> [X64PID]\n");
		return 1;
	}

	// read in the dll
	BYTE* image = NULL;
	DWORD imageSize = 0;
	if (!ReadFileData(argv[1], &image, &imageSize)) {
		wprintf(L"Failed to read image file: %s\n", argv[1]);
		return 0;
	}

	//turn pid->Dword
	WCHAR* wcNull = NULL;
	DWORD pid= wcstoul(argv[2],&wcNull, 10);
	if (*wcNull!=L'\x00') {
		wprintf(L"PID format Error: [%s]\n", argv[2]);
		return 0;
	}

	//get process handle
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProc == NULL) {
		wprintf(L"Failed to open process for injection: %lu\n", pid);
		return 1;
	}

	//get loader
	BYTE* loader = (BYTE*)loader_x64;
	size_t loaderSize =  sizeof(loader_x64);

	//set and creat remote image size
	const size_t remoteShellcodeSize = loaderSize + imageSize;
	void* remoteShellcode = VirtualAllocEx(hProc, 0, remoteShellcodeSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (remoteShellcode == NULL) {
		wprintf(L"Failed to allocate remote memory\n");
		return 1;
	}

	wprintf(L"Writing %lu bytes into process at %p\n", (DWORD)remoteShellcodeSize, remoteShellcode);

	// write shellcode and image to process
	SIZE_T numWritten = 0;
	if (!WriteProcessMemory(hProc, remoteShellcode, loader, loaderSize, &numWritten) ||
		!WriteProcessMemory(hProc, (BYTE*)remoteShellcode + loaderSize, image, imageSize, &numWritten)) {
		wprintf(L"Failed to write remote process memory\n");
		return 1;
	}

	DWORD tid = 0;
	HANDLE hThread = NULL;

	RTLCREATEUSERTHREAD fpRtlCreateUserThread = (RTLCREATEUSERTHREAD)GetProcAddress(GetModuleHandleA("ntdll"), "RtlCreateUserThread");
	
	if (fpRtlCreateUserThread == NULL) {
		wprintf(L"Failed to resolve ntdll!RtlCreateUserThread\n");
		return 1;
	}

	wprintf(L"Calling RtlCreateUserThread\n");
	fpRtlCreateUserThread(hProc, NULL, FALSE, 0, 0, 0, (LPTHREAD_START_ROUTINE)remoteShellcode, NULL, &hThread, NULL);
	wprintf(L"--- Success ---\n");

	return 1;
}
