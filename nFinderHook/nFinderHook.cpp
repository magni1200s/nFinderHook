/*
nFinderHook.cpp : DLL hook for send() and HttpSendRequest() with Detour

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

// Preparation for send() hook
// Target dll is ws2_32.dll
int (WINAPI *dsend)(SOCKET, const char*, int, int) = send;
int WINAPI mysend(SOCKET s, const char* buf, int len, int flags);

// Preparation for HttpSendRequest() hook
// Target dll is wininet.dll
int (WINAPI *dHttpSendRequestW)(HINTERNET ,LPCTSTR ,DWORD ,LPVOID ,DWORD) = HttpSendRequestW;
int WINAPI myHttpSendRequestW(HINTERNET, LPCTSTR, DWORD, LPVOID, DWORD);

INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		// Attach custom send() to ws2_32.dll
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)dsend, mysend);
		DetourTransactionCommit();

		// Attach custom HttpSendRequest() to wininet.dll
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

// mysend() : Add the flag to request a Adobe Flash video into Cookie header in HTTP request headers
//
// nFinder will send a HTTP request to retrieve target video as follows.
//
// GET /watch/sm00000000?watch_harmful=1 HTTP/1.1
// User - Agent : Mozilla / 5.0 (Windows NT 6.1; WOW64; rv:17.0) Gecko / 20100101 Firefox / 17.0
// Referer : http ://www.nicovideo.jp/
// Host : www.nicovideo.jp
// Cookie : nicosid = <id>; user_session = <session id>; nicohistory = <history> ; nicorepo_filter = all
//
// NICO NICO servers recieve above request, the servers will return video in HTML5 format by default.
// Unfortunatelly, nFinder supports only Flash video .If you need Flash video, you have to add the flag "watch_flash=1" into Cookie header in HTTP request headers.
// So mysend() intercepts send(), and insert the flag into Cookie heaer.
//
int WINAPI mysend(SOCKET s, const char* buf, int len, int flags) {

	// Request header size is normally less than 1KB, but do nothing if buffer size exceeds 8KB for safety
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

// myHttpSendRequestW() : Modify content-type to request author's commnet in HTTP request headers
//
// nFinder will send a HTTP POST request with request body to author's commnet as follows.
//
// POST /api/ HTTP/1.1
// Accept: */*
// Accept-Language: ja-JP
// x-flash-version: 26,0,0,137
// Content-Type: application/x-www-form-urlencoded
// Content-Length: 74
// Accept-Encoding: gzip, deflate
// User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 6.2; WOW64; Trident/7.0; .NET4.0C; .NET4.0E; .NET CLR 2.0.50727; .NET CLR 3.0.30729; .NET CLR 3.5.30729; Creative AutoUpdate v1.41.07)
// Host: nmsg.nicovideo.jp
// Connection: Keep-Alive
// Pragma: no-cache
// Cookie: nicosid=<id>; user_session=<sessionid>; nicorepo_filter=all; nicohistory=<history>; __utma=<value>; user_session_secure=<value>; optimizelyEndUserId=<value>; nicolivehistory=<value>; optimizelyBuckets=<value>; optimizelySegments=<value>; _ga=<value>; _gid=<value>
//
// <thread res_from="-250" fork="1" version="20090904" thread="1498727560" />
//
//
// NICO NICO servers recieve above request, the servers will return 400 error since they do not accpet "application/x-www-form-urlencoded".
// So myHttpSendRequestW() intercepts HttpSendRequestW(), and modify content type to "text/html".
// NOTE: **fork="1"** in request body is the key to retrieve author's comment. If the key does not exitst, that request is to retrieve viewer's comment.
// NOTE2: This POST request is generated by SPUTNIK.swf, not nFinder.exe. So you cannot trace the code wiht .NET debugger like dnSpy. API monitor e.g. WinAPIOverride http://jacquelin.potier.free.fr/winapioverride32/ is useful.
//
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