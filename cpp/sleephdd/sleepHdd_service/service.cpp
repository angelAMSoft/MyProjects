#include "service.h"

using namespace std;

HANDLE CreateIPCChannel(WCHAR* in, int lenght)
{
	PARGUMENTS pDataArray = (PARGUMENTS)malloc(sizeof(ARGUMENTS));
	DWORD   dwThreadIdArray;
    HANDLE  hThread; 
	pDataArray->in = in;
	pDataArray->lenght = lenght;
	hPipe = CreateNamedPipe( 
        lpszPipename,             // pipe name 
        PIPE_ACCESS_DUPLEX,       // read/write access 
        PIPE_TYPE_MESSAGE |       // message type pipe 
        PIPE_READMODE_MESSAGE |   // message-read mode 
        PIPE_WAIT,                // blocking mode 
        PIPE_UNLIMITED_INSTANCES, // max. instances  
        BUFSIZE,                  // output buffer size 
        BUFSIZE,                  // input buffer size 
        0,                        // client time-out 
        NULL);
	hThread = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            IPCThreadFunction,       // thread function name
            pDataArray,          // argument to thread function 
            0,                      // use default creation flags 
            &dwThreadIdArray);
	return hThread;
}

DWORD WINAPI IPCThreadFunction( LPVOID lpParam )
{
	BOOL fConnected, fSuccess;
	TCHAR Data[BUFSIZE] = {0}; 
	DWORD cbBytesRead = 0, cbWritten = 0; 
	PARGUMENTS args = (PARGUMENTS)lpParam;
	int lenghtReply = 0;

	fConnected = ConnectNamedPipe(hPipe, NULL);
	if (fConnected) {
		while(!ShutFlag)
		{
			fSuccess = ReadFile( 
				hPipe,        // handle to pipe 
				Data,    // buffer to receive data 
				BUFSIZE*sizeof(TCHAR), // size of buffer 
				&cbBytesRead, // number of bytes read 
				NULL);        // not overlapped I/O
			if(lstrcmp(L"Lock", Data) == 0)
			{
				ZeroMemory(Data, sizeof(WCHAR) * BUFSIZE);
			    fSuccess = ReadFile( 
				hPipe,        // handle to pipe 
				Data,    // buffer to receive data 
				BUFSIZE*sizeof(TCHAR), // size of buffer 
				&cbBytesRead, // number of bytes read 
				NULL);        // not overlapped I/O
				vector<wstring> disks;
				ParsingData(Data, cbBytesRead, disks);
				ZeroMemory(Data, sizeof(WCHAR) * BUFSIZE);
				for(vector<wstring>::iterator iter = disks.begin(); iter != disks.end(); ++iter)
				{
					int result = LockVolume(*iter);
					if (!result)
					{
						swprintf(Data,L"Volume %s lock successfully!", iter->c_str());
					}
					else
					{
						swprintf(Data,L"Volume %s not lock with error %d!", iter->c_str(), result);
					}
					fSuccess = WriteFile( 
						hPipe,        // handle to pipe 
						Data,      // buffer to write from 
						BUFSIZE*sizeof(TCHAR), // number of bytes to write 
						&cbWritten,   // number of bytes written 
						NULL);        // not overlapped I/O 
					ZeroMemory(Data, sizeof(WCHAR) * BUFSIZE);
					FlushFileBuffers(hPipe);
				}
			}
		}
	}
	return 0;
}

void ParsingData(const WCHAR *data, int size, vector<wstring> &out)
{
	wstring tmpStr;
	int lenght = size / sizeof(WCHAR);
	for(int i = 0; i < lenght; ++i)
	{
		if(data[i] != L';')
		{
			tmpStr += data[i];
		}
		else
		{
			out.push_back(tmpStr);
			tmpStr.clear();
		}
	}

}

