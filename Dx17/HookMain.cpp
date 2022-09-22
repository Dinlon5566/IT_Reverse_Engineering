#include "stdio.h"
#include "conio.h"
#include "windows.h"

#define	DEF_DLL_NAME		"KeyHook.dll"
#define	DEF_HOOKSTART		"HookStart"
#define	DEF_HOOKSTOP		"HookStop"

typedef void (*PFN_HOOKSTART)();
typedef void (*PFN_HOOKSTOP)();

void main()
{
	HMODULE			hDll = NULL;
	PFN_HOOKSTART	HookStart = NULL;
	PFN_HOOKSTOP	HookStop = NULL;
	char			ch = 0;

	hDll = LoadLibraryA(DEF_DLL_NAME);
	if (hDll == NULL)
	{
		printf("LoadLibrary(%s) failed!!! [%d]", DEF_DLL_NAME, GetLastError());
		return;
	}

	HookStart = (PFN_HOOKSTART)GetProcAddress(hDll, DEF_HOOKSTART);
	HookStop = (PFN_HOOKSTOP)GetProcAddress(hDll, DEF_HOOKSTOP);

	HookStart();

	printf("press 'q' to quit!\n");
	while (_getch() != 'q');

	HookStop();

	FreeLibrary(hDll);
}