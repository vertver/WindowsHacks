/*********************************************************
* Copyright (C) VERTVER, 2019. All rights reserved.
* WindowsHacks - project with hacks for Windows
* MIT-License
**********************************************************
* Module Name: Getting Windows version
*********************************************************/

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <versionhelpers.h>
#pragma comment(lib, "Version.lib")

typedef struct
{
	char szBuildName[260];
	uint16_t MajorVersion;			// 6 for Windows Seven
	uint16_t MinorVersion;			// 1 for Windows Seven
	uint16_t BuildVersion;			// ex. 18362
	uint16_t PostBuildVersion;		// ex. 112
	uint16_t MarketVersion;			// ex. 1903
} BASE_OS_VERSION_INFO;

bool
GetSystemVersion(BASE_OS_VERSION_INFO* pVersion)
{
	WCHAR szPath[260] = {};
	BYTE StringInfo[1024] = {};

	if (IsWindows7OrGreater())
	{
		pVersion->MajorVersion = 6;
		pVersion->MinorVersion = 1;
	}
	else
	{
		return false;
	}

	if (IsWindows8OrGreater())
	{
		pVersion->MajorVersion = 6;
		pVersion->MinorVersion = 2;
	}

	// it's not just working before 8.1, because assholes in Microsoft can't 
	// make basic function for check Windows version
	if (IsWindows8Point1OrGreater())
	{
		pVersion->MajorVersion = 6;
		pVersion->MinorVersion = 3;
	}

	if (IsWindows10OrGreater())
	{
		pVersion->MajorVersion = 10;
		pVersion->MinorVersion = 0;
	}

	// get system directory for check dll version
	if (GetSystemDirectoryW(szPath, sizeof(szPath)))
	{
		// use more updated library from system directory
		wcscat_s(szPath, L"\\ntoskrnl.exe");

		VS_FIXEDFILEINFO * pvi;
		DWORD sz = sizeof(VS_FIXEDFILEINFO);

		// get Windows version from ntdll library
		if (GetFileVersionInfoW(szPath, 0, sizeof(StringInfo), StringInfo))
		{
			if (VerQueryValueW(&StringInfo[0], L"\\", (LPVOID*)&pvi, (unsigned int*)&sz))
			{
				pVersion->MajorVersion = HIWORD(pvi->dwProductVersionMS);
				pVersion->MinorVersion = LOWORD(pvi->dwProductVersionMS);
				pVersion->BuildVersion = HIWORD(pvi->dwProductVersionLS);
				pVersion->PostBuildVersion = LOWORD(pvi->dwProductVersionLS);

				switch (pVersion->BuildVersion)
				{
				case 7600: pVersion->MarketVersion = 'S0'; break;
				case 7601: pVersion->MarketVersion = 'S1'; break;
				case 9200: pVersion->MarketVersion = 'S0'; break;
				case 9600: pVersion->MarketVersion = 'S1'; break;
				case 10240: pVersion->MarketVersion = 1507; break;
				case 10586: pVersion->MarketVersion = 1511; break;
				case 14393: pVersion->MarketVersion = 1607; break;
				case 15063: pVersion->MarketVersion = 1703; break;
				case 16299: pVersion->MarketVersion = 1709; break;
				case 17134: pVersion->MarketVersion = 1803; break;
				case 17763: pVersion->MarketVersion = 1809; break;
				case 18362: pVersion->MarketVersion = 1903; break;
				default: pVersion->MarketVersion = pVersion->BuildVersion; break;
				}			
			}
		}
	}

	LPCSTR WindowsVer = nullptr;
	switch (pVersion->MajorVersion)
	{
	case 10:
		WindowsVer = "10";
		break;
	case 6:
		if (pVersion->MinorVersion == 1)
		{
			WindowsVer = "7";
			break;
		}
		if (pVersion->MinorVersion == 2)
		{
			WindowsVer = "8";
			break;
		}
		if (pVersion->MinorVersion == 3)
		{
			WindowsVer = "8.1";
			break;
		}
	default:
		break;
	}

	bool isServicePacked = pVersion->MarketVersion == 'S1' || pVersion->MarketVersion == 'S0';

	if (!isServicePacked)
	{
		snprintf(
			pVersion->szBuildName,
			sizeof(pVersion->szBuildName),
			"Microsoft Windows %s %i (%s %i.%i)",
			WindowsVer,
			pVersion->MarketVersion,
			pVersion->BuildVersion == pVersion->MarketVersion ? "Insider build" : "Build",
			pVersion->BuildVersion,
			pVersion->PostBuildVersion
		);
	}
	else
	{
		snprintf(
			pVersion->szBuildName,
			sizeof(pVersion->szBuildName),
			"Microsoft Windows %s %s(%s %i.%i)",
			WindowsVer,
			pVersion->MarketVersion == 'S1' ? "Service Pack 1 " : "",
			pVersion->BuildVersion == pVersion->MarketVersion ? "Insider build" : "Build",
			pVersion->BuildVersion,
			pVersion->PostBuildVersion
		);
	}

	return true;
}


int main()
{
	BASE_OS_VERSION_INFO baseInfo = {};

	if (GetSystemVersion(&baseInfo))
	{
		printf("Windows OS information: \n%s", baseInfo.szBuildName);
	}

	printf("\nPress any key to close console window");

	_getwch();
	ExitProcess(0);

	return 0;
}
