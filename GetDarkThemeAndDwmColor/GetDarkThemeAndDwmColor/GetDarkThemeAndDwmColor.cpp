/*********************************************************
* Copyright (C) VERTVER, 2019. All rights reserved.
* WindowsHacks - project with hacks for Windows
* MIT-License
**********************************************************
* Module Name: Get info of system colors
*********************************************************/
#include "pch.h"
#include <iostream>

enum WINDOWS_VERSIONS
{
	WIN_7_SERVER2008R2 = 1,
	WIN_8_SERVER2012,
	WIN_81_SERVER2012R2,
	WIN_10_SERVER2016,
	WINDOWS_FUTURE
};

DWORD
GetWindowsVersion()
{
	static DWORD WinVer = 0;

	if (!WinVer)
	{
		DWORD dwVersion = 0;
		DWORD dwMajorVersion = 0;
		DWORD dwMinorVersion = 0;
		DWORD dwBuild = 0;

		typedef DWORD(WINAPI *LPFN_GETVERSION)(VOID);
		static LPFN_GETVERSION fnGetVersion = nullptr;

		if (!fnGetVersion)
		{
			fnGetVersion = (LPFN_GETVERSION)GetProcAddress(GetModuleHandleA("kernel32"), "GetVersion");
		}

		if (fnGetVersion)
		{
			dwVersion = fnGetVersion();
			dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
			dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

			if (dwVersion < 0x80000000) { dwBuild = (DWORD)(HIWORD(dwVersion)); }

			switch (dwMajorVersion)
			{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				break;
			case 6:
				switch (dwMinorVersion)
				{
				case 1:	 WinVer = WIN_7_SERVER2008R2; break;
				case 2:	 WinVer = WIN_8_SERVER2012; break;
				case 3:	 WinVer = WIN_81_SERVER2012R2; break;
				default: WinVer = WINDOWS_FUTURE; break;
				}
			case 10:
				switch (dwMinorVersion)
				{
				case 0:	 WinVer = WIN_10_SERVER2016; break;
				default: WinVer = WINDOWS_FUTURE; break;
				}
				break;
			default:
				WinVer = WINDOWS_FUTURE;
				break;
			}
		}
	}

	return WinVer;
}


typedef HRESULT(__stdcall *PDwmGetColorizationColor)(DWORD *pcrColorization, BOOL *pfOpaqueBlend);
#define RELEASEKEY(Key) if (Key) RegCloseKey(Key);

DWORD
GetDWMCOLOR() {
	static PDwmGetColorizationColor pProc = nullptr;
	DWORD color = 0;

	if (!pProc)
	{
		// don't use DWM api with linking
		HMODULE hModule = nullptr;
		if (!(hModule = GetModuleHandleW(L"Dwmapi.dll")))
		{
			hModule = LoadLibraryW(L"Dwmapi.dll");
			if (!hModule) return 0;
		}

		pProc = (PDwmGetColorizationColor)GetProcAddress(hModule, "DwmGetColorizationColor");

	}

	BOOL opaq = 0;
	pProc(&color, &opaq);

	return color;
}

bool
IsDarkTheme()
{
	HKEY hKey = nullptr;
	DWORD Value1 = 0;
	DWORD Value2 = 0;
	DWORD size = 0;

	// open theme regedit key 
	if (!RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey))
	{
		// if we can't open first key - try second
		if (RegQueryValueExA(hKey, "AppsUseLightTheme", nullptr, nullptr, (LPBYTE)&Value1, &size)) 
		{
			if (RegQueryValueExA(hKey, "SystemUsesLightTheme", nullptr, nullptr, (LPBYTE)&Value2, &size))
			{
				RELEASEKEY(hKey);
				return false;
			}
		}

		RELEASEKEY(hKey);
		return !(Value1 || Value2);
	}

	return false;
}

int main()
{
	if (!GetWindowsVersion()) 
	{
		printf("This program can only work on Windows 7 or greater\n");
		return -1;
	}

	printf("Dark mode is %s", (IsDarkTheme() ? "enabled" : "disabled"));
	printf("\nYour system color: %#08x", GetDWMCOLOR());
	printf("\nPress any key to close console window");

	_getwch();
	ExitProcess(0);

	return 0;
}
