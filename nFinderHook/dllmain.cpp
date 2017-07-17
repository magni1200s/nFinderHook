// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "detours.h"
#include <stdio.h>
#include <cstdio>
#include <ws2tcpip.h>
#include <windows.h>

#pragma comment(lib, "detours.lib")
#pragma comment(lib, "ws2_32.lib")

/* send */
int (WINAPI *dsend)(SOCKET, const char*, int, int) = send;
int WINAPI mysend(SOCKET s, const char* buf, int len, int flags);

/* recv */
int (WINAPI *drecv)(SOCKET, char*, int, int) = recv;
int WINAPI myrecv(SOCKET s, char* buf, int len, int flags);

/* connect */
//int (WINAPI *dconnect)(SOCKET, const struct sockaddr*, int) = connect;
//int WINAPI myconnect(SOCKET s, const struct sockaddr *name, int namelen);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//		DetourTransactionBegin();
		//		DetourUpdateThread(GetCurrentThread());
		//		DetourAttach(&(PVOID&)dconnect, myconnect);
		//		DetourTransactionCommit();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)dsend, mysend);
		DetourTransactionCommit();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)drecv, myrecv);
		DetourTransactionCommit();
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}