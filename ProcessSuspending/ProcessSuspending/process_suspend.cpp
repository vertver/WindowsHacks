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
			if (!hProcess || hProcess == INVALID_HANDLE_VALUE)
			{
				if (GetLastError() != ERROR_ACCESS_DENIED)
				{
					printf("OpenProcess ended with fail %u\nPress any key to continue\n", GetLastError());
					_getwch();
					return GetLastError();
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
			else
			{
				printf("NtSuspendProcess - failed.\n");
				_getwch();
			}
		}
	}

	return 0;
}