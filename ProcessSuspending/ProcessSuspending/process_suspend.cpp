/*********************************************************
* Copyright (C) VERTVER, 2019. All rights reserved.
* WindowsHacks - project with hacks for Windows
* MIT-License
**********************************************************
* Module Name: suspending and resuming processes
*********************************************************/
#include <windows.h>
#include <iostream>

typedef LONG(NTAPI *ProcFunc)(HANDLE);

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	static ProcFunc pFuncR = nullptr;
	static ProcFunc pFuncS = nullptr;
	DWORD dwProcessId = 0;

	// get procs of funcs
	if (!pFuncS) { pFuncS = (ProcFunc)GetProcAddress(GetModuleHandleW(L"ntdll"), "NtSuspendProcess"); }
	if (!pFuncR) { pFuncR = (ProcFunc)GetProcAddress(GetModuleHandleW(L"ntdll"), "NtResumeProcess"); }

	if (pFuncS && pFuncR)
	{
		printf("Enter process Id\n");
		scanf_s("%u", &dwProcessId);

		if (dwProcessId)
		{
			printf("Suspending process...\n");
			
			HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
			if (!hProcess)
			{
				printf("OpenProcess ended with fail %u\nPress any key to continue\n", GetLastError());
				_getwch();
				return GetLastError();
			}

			// suspend process
			if (SUCCEEDED(pFuncS(hProcess)))
			{
				printf("NtSuspendProcess - success\n");
				Sleep(2000);
				printf("Resuming process...\n");

				if (SUCCEEDED(pFuncR(hProcess)))
				{
					printf("NtResumeProcess - success\nPress any key to continue\n");
				}
				else
				{
					printf("NtResumeProcess ended with fail %u\nPress any key to continue\n", GetLastError());
				}

				_getwch();
			}
		}
	}
}