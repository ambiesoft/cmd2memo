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
	DWORD dwRead = 0;
	for( ;ReadFile(GetStdHandle(STD_INPUT_HANDLE), szBuffer, sizeof(szBuffer)-1, &dwRead, NULL) && dwRead != 0;)
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
