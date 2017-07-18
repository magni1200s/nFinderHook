/*
nFinderLauncher.cpp : nFinder launcher with DLL injection 

Copyright(C) 2017 magni1200s

This program is free software; you can redistribute it and / or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301, USA.
*/


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

	// Initialize si&pi; fill with 0
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);

	// Get cuurent directory path
	// Charset of path to nFinderHook.dll must be ANSI even if the system is running with UNICODE charset
	// So GetCurrentDirectoryA shall be called
	GetCurrentDirectoryA(MAX_PATH, szDirPath);

	// Set full path for nFinder.exe
#ifdef UNICODE
	WCHAR szDirPathW[MAX_PATH];
	WCHAR szExePathW[MAX_PATH];

	// Path to nFinder.exe must be 
	GetCurrentDirectoryW(MAX_PATH, szDirPathW);
	lstrcpyW(szExePathW, szDirPathW);
	lstrcatW(szExePathW, _T("\\nFinder.exe"));
	szExePath = szExePathW;
#else
	snprintf_s(szExePath, MAX_PATH, "%s\\nFinder.exe", szDirPath);
#endif // UNICODE

	// Set full path for nFinderHook.dll
	snprintf(szDllPath, MAX_PATH, "%s\\nFinderHook.dll", szDirPath);

	// Spawn a DLL injected process
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
		NULL);			// use standard CreateProcess API 

	// Show the result of DetourCreateProcessWithDll
	if (bStatus) {
		_tprintf(_T("Launched nFinder PID %d!\n"), pi.dwProcessId);
	}
	else {
		_tprintf(_T("Failed to launch...\n"));
	}

	return 0;
}