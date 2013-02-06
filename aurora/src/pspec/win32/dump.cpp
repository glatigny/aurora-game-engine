#include "dump.h"
#include <windows.h>
#include <dbghelp.h>

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
	CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
	);

LONG WINAPI TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo );
	
void CreateDumpOnCrash() {
	SetUnhandledExceptionFilter( TopLevelFilter );
}

LONG TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo ) {
	LONG retval = EXCEPTION_CONTINUE_SEARCH;

	HMODULE dbHlpDll = NULL;
	char modulePath[MAX_PATH];
	char strTmp[MAX_PATH];

	memset(modulePath, 0, sizeof(char) * MAX_PATH);
	if( GetModuleFileName(NULL, strTmp, MAX_PATH) )
	{
		char *slashPos = strrchr(strTmp, '\\');
		if (slashPos)
		{
			strncpy(modulePath, strTmp, (size_t)(slashPos - strTmp) );
		}
	}

	// TODO: try to load the library in the current directory first
	dbHlpDll = LoadLibraryA("dbghelp.dll");

	char* resultText = NULL;

	if( dbHlpDll )
	{
		MINIDUMPWRITEDUMP writeDump = (MINIDUMPWRITEDUMP)GetProcAddressA( dbHlpDll, "MiniDumpWriteDump" );
		if( writeDump )
		{
			char dumpPath[MAX_PATH];

			// work out a good place for the dump file
			if( modulePath[0] != '\0' )
				memcpy(dumpPath, modulePath, sizeof(char) * MAX_PATH);
			else if (!GetTempPath( MAX_PATH, dumpPath )
				memset(dumpPath, 0, sizeof(char) * MAX_PATH);

			strcat( dumpPath, "aed_dump.dmp" );

			// ask the user if they want to save a dump file
			if( MessageBoxA( NULL, "Something bad happened in your program, would you like to save a diagnostic file ?", "Aurora Engine Debugger", MB_YESNO) == IDYES )
			{
				// create the file
				HANDLE dumpFile = CreateFile(dumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				if( dumpFile != INVALID_HANDLE_VALUE )
				{
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

					ExInfo.ThreadId = GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;

					// write the dump
					if( writeDump(GetCurrentProcess(), GetCurrentProcessId(), dumpFile, MiniDumpNormal, &ExInfo, NULL, NULL) )
					{
						sprintf(strTmp, "Saved dump file to '%s'", dumpPath);
						resultText = strTmp;
						retval = EXCEPTION_EXECUTE_HANDLER;
					}
					else
					{
						sprintf(strTmp, "Failed to save dump file to '%s' (error %d)", dumpPath, GetLastError());
						resultText = strTmp;
					}
					CloseHandle(dumpFile);
				}
				else
				{
					sprintf(strTmp, "Failed to create dump file '%s' (error %d)", dumpPath, GetLastError());
					resultText = strTmp;
				}
			}
		}
		else
		{
			resultText = "dbghelp.dll is too old";
		}
	}
	else
	{
		resultText = "dbghelp.dll not found";
	}

	if( resultText )
		MessageBoxA( NULL, resultText, "Aurora Engine Debugger", MB_OK );

	return retval;
}