// nFinderHook.cpp
//

#include "stdafx.h"
#include "detours.h"
#include <stdio.h>
#include <cstdio>
#include <tchar.h>
#include <WinInet.h>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "detours.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "ws2_32.lib")

#define BUFFSIZE 8192

/* send */
int (WINAPI *dsend)(SOCKET, const char*, int, int) = send;
int WINAPI mysend(SOCKET s, const char* buf, int len, int flags);

/* HttpSendRequest */
int (WINAPI *dHttpSendRequestW)(HINTERNET ,LPCTSTR ,DWORD ,LPVOID ,DWORD) = HttpSendRequestW;
int WINAPI myHttpSendRequestW(HINTERNET, LPCTSTR, DWORD, LPVOID, DWORD);

INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)dsend, mysend);
		DetourTransactionCommit();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)dHttpSendRequestW, myHttpSendRequestW);
		DetourTransactionCommit();

	case DLL_PROCESS_DETACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}

int WINAPI mysend(SOCKET s, const char* buf, int len, int flags) {

	if (len < BUFFSIZE)
	{
		const char *p1 = NULL;

		if (((p1 = strstr(buf, "GET /watch/sm")) != NULL) || ((p1 = strstr(buf, "GET http://www.nicovideo.jp/watch/sm")) != NULL))
		{
			char buf2[BUFFSIZE];
			int len2 = 0;

			if (((p1 = strstr(p1, "Cookie:")) != NULL) && (!strstr(p1, "watch_flash=1")))
			{
				p1 = strstr(p1, "\r\n");

				strncpy_s(buf2, sizeof(buf2), buf, p1 - buf);
				strcat_s(buf2, sizeof(buf2), "; watch_flash=1");
				strcat_s(buf2, sizeof(buf2), p1);

				len2 = strlen(buf2);

				return dsend(s, buf2, len2, flags);
			}
		}
	}

	return dsend(s, buf, len, flags);
}

int WINAPI myHttpSendRequestW(HINTERNET hRequest, LPCTSTR lpszHeaders, DWORD dwHeadersLength, LPVOID lpOptional, DWORD dwOptionalLength)
{
	if (dwOptionalLength > 0)
	{
		char buf[BUFFSIZE];

		strcpy_s(buf, sizeof(buf), (char *)lpOptional);
		if (strstr(buf, "fork=\"1\""))
		{
			TCHAR* szHeaders = _T("Content-Type: text/xml");

			return dHttpSendRequestW(hRequest, szHeaders, _tcslen(szHeaders), lpOptional, dwOptionalLength);
		}
	}

	return dHttpSendRequestW(hRequest, lpszHeaders, dwHeadersLength, lpOptional, dwOptionalLength);
}

extern "C" __declspec(dllexport) void dummy(void) {
	return;
}