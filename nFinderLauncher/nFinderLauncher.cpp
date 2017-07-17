// nFinderLauncher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdio>
#include <stdio.h>
#include <windows.h>
#include "detours.h"

#pragma comment (lib, "detours.lib")

int _tmain(int argc, _TCHAR* argv[])
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	LPTSTR szExePath = NULL;
	CHAR   szDirPath[MAX_PATH];
	CHAR   szDllPath[MAX_PATH];
	BOOL   bStatus;

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);

	GetCurrentDirectoryA(MAX_PATH, szDirPath);

#ifdef UNICODE
	WCHAR szDirPathW[MAX_PATH];
	WCHAR szExePathW[MAX_PATH];

	GetCurrentDirectoryW(MAX_PATH, szDirPathW);
	lstrcpyW(szExePathW, szDirPathW);
	lstrcatW(szExePathW, _T("\\nFinder.exe"));
	szExePath = szExePathW;
#else
	snprintf_s(szExePath, MAX_PATH, "%s\\nFinder.exe", szDirPath);
#endif // UNICODE

	snprintf(szDllPath, MAX_PATH, "%s\\nFinderHook.dll", szDirPath);

	bStatus = DetourCreateProcessWithDll(
		NULL,			// application name 
		szExePath,		// full command line + arguments 
		NULL,			// process attributes
		NULL,			// thread attributes
		FALSE,			// inherit handles
		0,				// creation flags
		NULL,			// environment
		NULL,			// current directory
		&si,			// startup info
		&pi,			// process info
		szDllPath,		// path to dll to inject
		NULL);					// use standard CreateProcess API 

	if (bStatus) {
		_tprintf(_T("Launched nFinder PID %d!\n"), pi.dwProcessId);
	}
	else {
		_tprintf(_T("Failed to launch...\n"));
	}

	return 0;
}