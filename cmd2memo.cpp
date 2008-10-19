/***
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of the project nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
***/

#include "stdafx.h"
using namespace std;

#ifndef _countof
	#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

struct PROCESSWND {
	DWORD	dwProcessID;
	HWND	hWnd;
};

BOOL CALLBACK GetProcessWndProc(HWND hWnd, LPARAM lParam)
{
	DWORD	dwProcessID;

	GetWindowThreadProcessId(hWnd,&dwProcessID);
	if (dwProcessID == ((PROCESSWND*)lParam)->dwProcessID) {
		((PROCESSWND*)lParam)->hWnd = hWnd;
		return FALSE;
	}

	return TRUE;
}

HWND GetProcessWindow(DWORD dwProcessID)
{
	struct PROCESSWND stProcessWnd;

	stProcessWnd.dwProcessID = dwProcessID;
	stProcessWnd.hWnd = NULL;
	EnumWindows((WNDENUMPROC)GetProcessWndProc,(LPARAM)&stProcessWnd);

	return stProcessWnd.hWnd;
}

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
	TCHAR szClassName[1024]; szClassName[0]=0;
	if(GetClassName(hWnd, szClassName, _countof(szClassName)))
	{
		if(lstrcmpi(szClassName, _T("Edit"))==0)
		{
			*(HWND*)lParam = hWnd;
			return FALSE;
		}
	}
	return TRUE;
}

HWND GetPadEditWindow(DWORD dwProcessID)
{
	HWND hWndPad = GetProcessWindow(dwProcessID);
	if(hWndPad==NULL)
		return NULL;

	HWND hWndEdit = NULL;
	EnumChildWindows(hWndPad, EnumChildProc, (LPARAM)&hWndEdit);

	return hWndEdit;
}

#define BBBUUUFFF_SIZE 256
int _tmain(int argc, TCHAR* argv[])
{
	TCHAR szBuffer[BBBUUUFFF_SIZE];
	string all;

	for( DWORD dwRead = 0;
		ReadFile(GetStdHandle(STD_INPUT_HANDLE), szBuffer, sizeof(szBuffer)-1, &dwRead, NULL) 
			&& dwRead != 0
			;)
	{
		szBuffer[dwRead]=0;
		all += szBuffer;
	}

	PROCESS_INFORMATION ps;
	STARTUPINFO sp = {sizeof(STARTUPINFO)};
	if(!CreateProcess(NULL,
                      "notepad.exe",
					  NULL,
					  NULL,
					  FALSE,
					  0,
					  NULL,
					  NULL,
					  &sp,
					  &ps)) 
	{
		return -1;
	}

	if( 0 != WaitForInputIdle(ps.hProcess, 80000) )
		return -2;

	HWND hWndPadEdit = GetPadEditWindow(ps.dwProcessId);
	if(hWndPadEdit==NULL)
		return -3;

	SendMessage(hWndPadEdit, WM_SETTEXT, 0, (LPARAM)(LPCTSTR)all.c_str());

	CloseHandle(ps.hProcess);
	CloseHandle(ps.hThread);

	return 0;
}
