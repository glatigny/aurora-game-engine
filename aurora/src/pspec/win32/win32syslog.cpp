#include "win32syslog.h"
#include <windows.h>

/* ------------------------------------------------------------------------------------------------- */

const WCHAR g_syslogMainWnd[]		= L"_AuroraSyslog90_Aurora_MainWnd_";
const WCHAR g_syslogWindowName[]	= L"Aurora Syslog Viewer";

HWND hEdit					= NULL;
HWND g_sysloghwndMain		= NULL;
HINSTANCE g_syslogInst		= NULL;
unsigned long winsyslogcounter = 0;
char winsyslogbuffer[20000];

/* ------------------------------------------------------------------------------------------------- */

void win32Syslog(char* text)
{
	if( winsyslogcounter == 0 ) {
		memset(winsyslogbuffer, 0, 20000);
	}
	winsyslogcounter += (unsigned long)strlen(text) + 2;

	WCHAR wideText[65535];
	wideText[0] = '\0';

	if( winsyslogcounter >= 19999 ) {
		memmove(winsyslogbuffer, winsyslogbuffer+10000, 10000);
		MultiByteToWideChar( GetACP(), 0, winsyslogbuffer, -1, wideText, sizeof(wideText)/sizeof(wideText[0]) );
	//	SendMessage(hEdit, EM_SETSEL, (WPARAM)0, (LPARAM)10000);
	//	SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM) (LPCTSTR)L"");
	//	winsyslogcounter -= 10000;
		SendMessage(hEdit, WM_SETTEXT, (WPARAM)0, (LPARAM) (LPCTSTR)wideText);
		winsyslogcounter -= 10000;
	}

	MultiByteToWideChar( GetACP(), 0, text, -1, wideText, sizeof(wideText)/sizeof(wideText[0]) );
	strcat(winsyslogbuffer, "\r\n");
	strcat(winsyslogbuffer, text);
	
	WPARAM lsize = SendMessage(hEdit, WM_GETTEXTLENGTH, (WPARAM)0, (LPARAM)0);
	SendMessage(hEdit, EM_SETSEL, (WPARAM)lsize, (LPARAM)-1);

	SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM) (LPCTSTR)L"\r\n");
	SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM) (LPCTSTR)wideText);
	SendMessage(hEdit, EM_SETSEL, (WPARAM)-1, (LPARAM)-1);
}

/* ------------------------------------------------------------------------------------------------- */

int CreateSyslogWindow(HINSTANCE hInst)
{
	g_syslogInst = hInst;

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= 0;
	wcex.lpfnWndProc	= (WNDPROC) SyslogMainWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= g_syslogMainWnd;
	wcex.hIconSm		= NULL;

	if( !RegisterClassEx(&wcex) )
		return 1;

	int y = GetSystemMetrics(SM_CYFULLSCREEN);

	if( !CreateWindowEx(WS_EX_OVERLAPPEDWINDOW | WS_EX_TOPMOST, g_syslogMainWnd, g_syslogWindowName, 
		WS_CAPTION | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPED, 
						0, y - 120, 640, 120, NULL, NULL, g_syslogInst, NULL) )
		return 1;

	return 0;
}

/* ------------------------------------------------------------------------------------------------- */

LRESULT CALLBACK SyslogMainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:
			{
				g_sysloghwndMain = hwnd;
	            
				hEdit = CreateWindow( L"edit", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL, 0, 0, 0, 0, hwnd, NULL, g_syslogInst, NULL);
				HFONT l_font = CreateFontW(10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L"Lucida Console");
				SendMessage(hEdit, WM_SETFONT, (WPARAM)l_font, TRUE);

				ShowWindow(g_sysloghwndMain, 1);
				UpdateWindow(g_sysloghwndMain);
			}
			break;

		case WM_SIZE:
			MoveWindow(hEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
			break;

		case WM_CLOSE:
			SendMessage(g_sysloghwndMain, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
			break;

		case WM_COMMAND:
			break;

		default:
			return(DefWindowProc(hwnd, uMsg, wParam, lParam));
	}
	return 0;
}

DWORD WINAPI threadSyslog(LPVOID data)
{
	MSG msg;
	while (GetMessage(&msg, g_sysloghwndMain, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

void win32ShowError(const char* message) {
	MessageBoxA(g_sysloghwndMain, (LPCSTR)message, "Diagnosis", MB_OK | MB_ICONERROR | MB_APPLMODAL);
}
