#include <windows.h>
#include <tchar.h> 
#include <cstdio>
#include <iostream>
//#include <string>
//#include <stack>

#include "SleepConsole.h"
#include "PerformanceQuery.h"

#pragma comment(lib, "User32.lib")
using namespace std;

#define BUFSIZE 4096

void DisplayErrorBox(LPTSTR lpszFunction);

//bool OpenIPCChannel(LPTSTR datarecv, int lenght);
HINSTANCE gInstance;
const int minLenghtPass = 5;
const int maxLenghtPass = 40;
#define BUFFER_LENGTH 8192
#define MAXCMDLINELENGHT 4
LPTSTR bufferData;
//#define BUFSIZE 512
LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");
LPSTR asciiNP = "\\\\.\\pipe\\mynamedpipe";


int main(void)
{
	//PrintWaitCommandMessage();
	//char cmd[] = "Command:InActive";
	//char arg[] = "H:\\";
	//int lenght;
	//char* res = CreateBufferToSend(cmd, arg, &lenght);
	//char* newbuf = new char[lenght];
	//ZeroMemory(newbuf, lenght);
	//CopyMemory(newbuf, res, lenght);
	//OpenIPCChannel(NULL, 0);

	SleepConsole console;
	console.PrintHelpCommandMessage();
	console.ConsoleSleepHdd();
	//MEMORYSTATUSEX stat;
	//stat.dwLength = sizeof(MEMORYSTATUSEX);
	//GlobalMemoryStatusEx(&stat);
	//int res = queryPerf();
	return 0;
}



/*
bool OpenIPCChannel(LPTSTR datarecv, int lenght)
{
	//TCHAR chBuf[BUFSIZE] = L"H:\\;"; 
	//WCHAR cmd[BUFSIZE] = L"Command:InActive";
	DWORD cbRead = 0, cbWritten = 0; 
	WCHAR responce[BUFSIZE] = {0};
    BOOL fSuccess;
	DWORD error = 1;

	char cmd[] = "Command:InActive";
	char arg[] = "H:\\";
	//int lenght;
	//char* res = CreateBufferToSend(cmd, arg, &lenght);
	//while (!WaitNamedPipe(lpszPipename, NMPWAIT_WAIT_FOREVER) && error != 0){
	error = GetLastError();
	}
	//fSuccess = CallNamedPipe(lpszPipename, cmd, lstrlen(cmd) * sizeof(WCHAR),
	//	responce, BUFSIZE*sizeof(TCHAR), &cbRead, NMPWAIT_USE_DEFAULT_WAIT);

	fSuccess = CallNamedPipeA(asciiNP, res, lenght,
							responce, BUFSIZE*sizeof(TCHAR), &cbRead, NMPWAIT_USE_DEFAULT_WAIT);
	WaitNamedPipe(lpszPipename, NMPWAIT_WAIT_FOREVER);

	if(lstrcmp(responce, cmd) != 0)
		 return false;		
	fSuccess = CallNamedPipe(lpszPipename, chBuf, lstrlen(chBuf) * sizeof(WCHAR),
							responce, BUFSIZE*sizeof(TCHAR), &cbRead, NMPWAIT_USE_DEFAULT_WAIT);
	return true;
}
*/


