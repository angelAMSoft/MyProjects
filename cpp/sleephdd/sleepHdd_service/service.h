#ifndef SERVICE_H
#define SERVICE_H

#include <Windows.h>
#include <vector>
#include <string>

int LockVolume(const std::wstring &volume);
void ProcessHDDIsActive(void);
void MakeHDDInactive(void);
DWORD WINAPI IPCThreadFunction( LPVOID lpParam );
HANDLE CreateIPCChannel(WCHAR* in, int lenght);
void ParsingData(const WCHAR *data, int size, std::vector<std::wstring> &out);

#define BUFSIZE 4096
char servicePahtToLog[] = "C:\\Windows\\logs\\sleep_hdd.log";
LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe"); 
HANDLE  hThread;
HANDLE hPipe;
extern BOOL ShutFlag;

typedef struct Arguments {
    WCHAR* in;
    int lenght;
} ARGUMENTS, *PARGUMENTS;


#endif