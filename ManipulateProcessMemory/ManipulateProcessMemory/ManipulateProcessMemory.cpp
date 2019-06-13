/*********************************************************
* Copyright (C) VERTVER, 2019. All rights reserved.
* WindowsHacks - project with hacks for Windows
* MIT-License
**********************************************************
* Module Name: Manipulate with process memory
*********************************************************/
#include "pch.h"
#include <iostream>
#include <windows.h>

BOOL
IsProcessWithAdminPrivilege()
{
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	LPVOID pAdministratorsGroup = nullptr;
	BOOL bRet = FALSE;

	// init SID to control privileges
	AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pAdministratorsGroup);

	// ckeck membership
	CheckTokenMembership(nullptr, pAdministratorsGroup, &bRet);

	// clean pointer
	if (pAdministratorsGroup) { FreeSid(pAdministratorsGroup); pAdministratorsGroup = nullptr; }

	return bRet;
}

VOID
RunWithAdminPrivilege()
{
	if (!IsProcessWithAdminPrivilege())
	{
		WCHAR szPath[260] = { NULL };
		if (GetModuleFileNameW(nullptr, szPath, ARRAYSIZE(szPath)))
		{
			SHELLEXECUTEINFOW shellInfo = { sizeof(SHELLEXECUTEINFOW) };
			shellInfo.lpVerb = L"runas";
			shellInfo.lpFile = szPath;
			shellInfo.hwnd = nullptr;
			shellInfo.nShow = SW_NORMAL;

			if (ShellExecuteExW(&shellInfo)) { ExitProcess(GetCurrentProcessId()); }
		}
	}
}

int wmain()
{
	WCHAR PathToFile[260] = {};
	HANDLE hProcess = nullptr;
	DWORD ProcessId = 0;
	DWORD dwAccess = 0;
	LPWSTR lpCmdLine = GetCommandLineW();

	if (GetModuleFileNameW(GetModuleHandle(nullptr), PathToFile, sizeof(PathToFile)))
	{
		size_t size1 = 0;
		size_t size2 = 0;

		if ((size1 = wcslen(lpCmdLine)) > (size2 = wcslen(PathToFile) + 3))
		{
			ProcessId = *PULONGLONG((LPVOID)(lpCmdLine + (size2 * sizeof(WCHAR))));
		}

		Sleep(0);
	}

OpenPr:
	if (ProcessId)
	{
		hProcess = OpenProcess(dwAccess, FALSE, ProcessId);
		if (!hProcess || hProcess == INVALID_HANDLE_VALUE)
		{
			if (GetLastError() != ERROR_ACCESS_DENIED)
			{
				printf("Failed to open process with %u ID", ProcessId);
				_getwch();
				ExitProcess(0x7FFFFFFF);
			}
			else
			{
				if (!IsDebuggerPresent())
				{
					RunWithAdminPrivilege();
					printf("The process executing stopped by user\n");
					_getwch();
				}
				else
				{
					MessageBoxW(NULL, L"Please, restart your debugger or Visual Studio with admin privileges.", L"Warning", MB_OK | MB_ICONWARNING);
					ExitProcess(0x7FFFFFFF);
				}
			}
		}
	}
	else
	{
		printf("Enter process ID...\n");
		scanf_s("%u", &ProcessId);

		hProcess = OpenProcess(dwAccess, FALSE, ProcessId);
		if (!hProcess || hProcess == INVALID_HANDLE_VALUE)
		{
			if (GetLastError() != ERROR_ACCESS_DENIED)
			{
				printf("Failed to open process with %u ID", ProcessId);
				_getwch();
				ExitProcess(0x7FFFFFFF);
			}
			else
			{
				if (!IsDebuggerPresent())
				{
					RunWithAdminPrivilege();
					printf("The process executing stopped by user\n");
					_getwch();
				}
				else
				{
					MessageBoxW(NULL, L"Please, restart your debugger or Visual Studio with admin privileges.", L"Warning", MB_OK | MB_ICONWARNING);
					ExitProcess(0x7FFFFFFF);
				}
			}
		}
	}

DoThings:
	if (!hProcess)
	{
		goto OpenPr;
	}

	printf("\nWhat we can do now with this process? \n1 - Read process memory\n2 - Terminate process\n");
	wchar_t ch = _getwch();

	switch (ch)
	{
	case L'1':
	{
		size_t pData = 0;
		PVOID pBuf = nullptr;
		DWORD sizeToRead = 0;
		DWORD dws = 0;
		printf("Enter pointer to read data...\n");
		scanf_s("%u", &pData);
		printf("Enter size to read ...\n");
		scanf_s("%u", &sizeToRead);

		pBuf = HeapAlloc(GetProcessHeap(), 0, sizeToRead);

		if (!ReadProcessMemory(hProcess, (LPCVOID)pData, pBuf, sizeToRead, &dws))
		{
			printf("\nCan't read process memory");
		}

		DWORD dwFirstDWORD = *(LPDWORD)pBuf;

		printf("\nfirst dword: %u", dwFirstDWORD);
		_getwch();

		HeapFree(GetProcessHeap(), 0, pBuf);
	}
	break;
	case L'2':
	{
		size_t pData = 0;

		TerminateProcess(hProcess, 0);
		CloseHandle(hProcess);

		printf("Process with %u id was terminated\n", ProcessId);
		ProcessId = 0;
		hProcess = nullptr;
	}
	break;
	default:
		break;
	}

	printf("\nPress 'F' to exit, or any key to continue use");
	ch = _getwch();
	if (ch == L'F' || ch == L'f')
	{
		try
		{
			if (hProcess) CloseHandle(hProcess);
			hProcess = nullptr;
		}
		catch (...) {}
		
		ExitProcess(0);
	}
	
	goto DoThings;

	return 0;
}
