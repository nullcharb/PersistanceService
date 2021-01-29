#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "log.h"


char cServiceName[32] = "asktest";

SERVICE_STATUS ss;

SERVICE_STATUS_HANDLE hStatus = NULL;

FILE* setupLogging();

/*
 *
 */
BOOL ServiceHandler( DWORD dwControl )
{
	if( dwControl == SERVICE_CONTROL_STOP || dwControl == SERVICE_CONTROL_SHUTDOWN )
	{
		ss.dwWin32ExitCode = 0;
		ss.dwCurrentState  = SERVICE_STOPPED;
	}
	return SetServiceStatus( hStatus, &ss );
}

/*
 *
 */
VOID ServiceMain( DWORD dwNumServicesArgs, LPSTR * lpServiceArgVectors )
{
	CONTEXT Context;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	LPVOID lpPayload = NULL;

	ZeroMemory( &ss, sizeof(SERVICE_STATUS) );
	ZeroMemory( &si, sizeof(STARTUPINFO) );
	ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );

	si.cb = sizeof(STARTUPINFO);

	ss.dwServiceType = SERVICE_WIN32_SHARE_PROCESS;

	ss.dwCurrentState = SERVICE_START_PENDING;

	ss.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_SHUTDOWN;

	hStatus = RegisterServiceCtrlHandler( (LPCSTR)&cServiceName, (LPHANDLER_FUNCTION)ServiceHandler );
  
	if ( hStatus )
	{
		ss.dwCurrentState = SERVICE_RUNNING;

		SetServiceStatus( hStatus, &ss );

		FILE* logFptr = setupLogging();
		
		while (true)
		{
			char szFile[MAX_PATH] = { 0 };

			DWORD p_id = GetCurrentProcessId();
			GetModuleFileName(NULL, szFile, MAX_PATH);

			log_trace("Process ID: %d.\n", p_id);
			log_trace("GetModuleFileName: %s\n", szFile);

			Sleep(10000);
		}

		ServiceHandler( SERVICE_CONTROL_STOP );

		fclose(logFptr);
		
		ExitProcess( 0 );
	}
}


FILE* setupLogging()
{
	char logPath[MAX_PATH];

	GetTempPath(sizeof(logPath), logPath);

	char* logFileName = "PersistanceLog.log";

	if (logPath[lstrlen(logPath) - 1] != '\\') lstrcat(logPath, "\\");

	lstrcat(logPath, logFileName);

	FILE* logFptr;

	logFptr = fopen(logPath, "a+");

	log_add_fp(logFptr, LOG_TRACE);

	return logFptr;
}



/*
 *
 */
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	SERVICE_TABLE_ENTRY st[] = 
    { 
        { (LPSTR)&cServiceName, (LPSERVICE_MAIN_FUNCTIONA)&ServiceMain }, 
        { NULL, NULL } 
    };
	return StartServiceCtrlDispatcher( (SERVICE_TABLE_ENTRY *)&st );
}
