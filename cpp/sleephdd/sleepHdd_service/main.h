#ifndef MAIN_H
#define MAIN_H

#include <Windows.h>
#include <string>
#include <fstream>
#include <cstdlib>
#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Advapi32.lib")

WCHAR serviceName[] = L"SleepHddService";

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID WINAPI ServiceCtrlHandler(DWORD dwControl);
SERVICE_STATUS service_status;
SERVICE_STATUS_HANDLE hServiceStatus;
//BOOL ShutFlag = FALSE;

#endif