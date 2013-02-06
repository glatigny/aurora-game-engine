#ifndef PSPEC_WIN32_SYSLOG_H
#define PSPEC_WIN32_SYSLOG_H

#ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0500
#endif

#ifndef _WIN32_IE
#  define _WIN32_IE 0x0600
#endif

#ifdef LOG_WINDOW_SUPPORT

int CreateSyslogWindow(HINSTANCE hInst);
LRESULT CALLBACK SyslogMainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI threadSyslog(LPVOID data);

#endif

#endif /* PSPEC_WIN32_SYSLOG_H */