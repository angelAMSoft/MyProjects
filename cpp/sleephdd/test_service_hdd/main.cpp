#include "SleepHDD.h"
#include "Misc.h"
#include "LockVolume.h"
#include "MakeActiveDisk.h"
#include "SecurDescr.h"
#include "SleepHDDService.h"


void EndAllTask();
int runOperation(Operations command, POPERATION op, wchar_t* resultResponce);

DWORD WINAPI IPCThreadFunction( LPVOID lpParam );
HANDLE CreateIPCChannel(WCHAR* in, int lenght);
void CheckStartedWMI();

char servicePahtToLog[] = "C:\\Windows\\logs\\sleep_hdd.log";

LPTSTR lpszPipename;
LPTSTR lockMutex;
CRITICAL_SECTION* CriticalSection;
std::map<std::wstring, std::wstring>* ListMutex;
std::map<std::wstring, std::wstring>* ListTimers;
std::map<std::wstring, Operations>* ListCurrentBusyDisks;
ManageThread* ListThreads;

HANDLE  gThread;
HANDLE hPipe;
//BOOL ShutFlag = FALSE;
enum POSITIONDATA{
	POSITION_OPERATION,
	POSITION_ARGUMENTS
};

using namespace std;


int main(void)
{
	ListThreads = new ManageThread();
	lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe"); 
    lockMutex = TEXT("ANGEL_LOCKMUTEX");
	CriticalSection = new CRITICAL_SECTION;
	ListMutex = new	std::map<std::wstring, std::wstring>();
	ListTimers = new std::map<std::wstring, std::wstring>();
	ListCurrentBusyDisks = new std::map<std::wstring, Operations>();
	gThread = CreateIPCChannel(L"", 0);
	WaitForSingleObject(gThread, INFINITE);
	delete ListThreads;
	delete CriticalSection;
	delete ListMutex;
	delete ListTimers;
	delete ListCurrentBusyDisks;
	return 0;
}

void CheckStartedWMI()
{
	SC_HANDLE hServiceIOControlManager, hService;
	SERVICE_STATUS status;
	hServiceIOControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	hService = OpenService(hServiceIOControlManager, L"Winmgmt", GENERIC_READ | SERVICE_START);
	BOOL res = QueryServiceStatus(hService, &status);
	if(status.dwCurrentState == SERVICE_STOPPED)
		res = StartService(hService, 0, NULL);
	CloseServiceHandle(hService);
	CloseServiceHandle(hServiceIOControlManager);
}

HANDLE CreateIPCChannel(WCHAR* in, int lenght)
{
	DWORD   dwThreadIdArray;
    HANDLE  hThread; 
	SECURITY_ATTRIBUTES resultSID;
	{
		SecurDescr sSID;
		resultSID = sSID.CreateSID();
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
			&resultSID);
	}

	CheckStartedWMI();

	hThread = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            IPCThreadFunction,       // thread function name
            NULL,          // argument to thread function 
            0,                      // use default creation flags 
            &dwThreadIdArray);
	return hThread;
}



DWORD WINAPI IPCThreadFunction( LPVOID lpParam )
{
	BOOL fConnected, fSuccess;
	//TCHAR Request[BUFSIZE] = {0}; 
	TCHAR Response[BUFSIZE] = {0};
	//WCHAR buffer[SMALLBUF] = {0};
	//WCHAR letter[SIZELETTERWCHAR] = {0};
	DWORD cbBytesRead = 0, cbWritten = 0; 
	//int lenghtReply = 0;
	//bool IsCommand;
	//bool commandVerify;
	Operations recieveCommand;
	//commandVerify = false;
	DisconnectNamedPipe(hPipe);
	ListThreads->Monitor();
	SleepHDDService *managerService = new SleepHDDService();
	
	//ListThreads->EndAllThreads();
	char* buff = new char[256];
	DWORD err;
	ZeroMemory(buff, 256);
	while(managerService->GetStatus() == HDDSERVICE_RUNNING){
		fConnected = ConnectNamedPipe(hPipe, NULL);
		if(managerService->GetStatus() == HDDSERVICE_RUNNING) continue;
		while(managerService->GetStatus() == HDDSERVICE_RUNNING && ReadFile(hPipe, buff, 256, &cbBytesRead, NULL)){
			char** query = GetCommandAndArgs(buff, cbBytesRead);
			ZeroMemory(buff, 256);
			WCHAR* cmd = CharToWide(query[POSITION_OPERATION]);
			WCHAR* disks = CharToWide(query[POSITION_ARGUMENTS]);

			delete query[0];
			delete query[1];
			delete query;

			err = GetLastError();
			if(lstrcmp(L"Command:Lock", cmd) == 0){
				recieveCommand = LOCK;
			}
			else if (lstrcmp(L"Command:Active", cmd) == 0){
				recieveCommand = ACTIVE;
			}
			else if (lstrcmp(L"Command:Unlock", cmd) == 0){
				recieveCommand = UNLOCK;
			}
			else if (lstrcmp(L"Command:InActive", cmd) == 0){
				recieveCommand = INACTIVE;
			}
			else if(lstrcmp(L"Command:Query", cmd) == 0){
				recieveCommand = QUERY;
			}
			POPERATION op = new OPERATION;
			op->Data = disks;
			op->size = lstrlen(disks);
			runOperation(recieveCommand, op, Response);
			delete op;
			op = NULL;
			fSuccess = WriteFile( 
					hPipe,        // handle to pipe 
					Response,      // buffer to write from 
					BUFSIZE*sizeof(TCHAR), // number of bytes to write 
					&cbWritten,   // number of bytes written 
					NULL);        // not overlapped I/O 	
		}
		DisconnectNamedPipe(hPipe);
	}
	delete buff;
	EndAllTask();
	ListThreads->EndAllThreads();

	/*
	while(!ShutFlag)
	{
		IsCommand = false;
		fConnected = ConnectNamedPipe(hPipe, NULL);
		while(ReadFile( 
			hPipe,        // handle to pipe 
			Request,    // buffer to receive data 
			BUFSIZE*sizeof(TCHAR), // size of buffer 
			&cbBytesRead, // number of bytes read 
			NULL))
		{
			if(!IsCommand && (lstrcmp(L"Command:Lock", Request) == 0)){
				IsCommand = true;
				recieveCommand = LOCK;
			}
			else if (!IsCommand && (lstrcmp(L"Command:Active", Request) == 0)){
				IsCommand = true;
				recieveCommand = ACTIVE;
			}
			else if (!IsCommand && (lstrcmp(L"Command:Unlock", Request) == 0)){
				IsCommand = true;
				recieveCommand = UNLOCK;
			}
			else if (!IsCommand && (lstrcmp(L"Command:InActive", Request) == 0)){
				IsCommand = true;
				recieveCommand = INACTIVE;
			}
			else if(!IsCommand && (lstrcmp(L"Command:Query", Request) == 0)){
				IsCommand = true;
				recieveCommand = QUERY;
			}
			if(!commandVerify && IsCommand){
				fSuccess = WriteFile( 
					hPipe,        // handle to pipe 
					Request,      // buffer to write from 
					BUFSIZE*sizeof(TCHAR), // number of bytes to write 
					&cbWritten,   // number of bytes written 
					NULL);        // not overlapped I/O 
				commandVerify = true;
				FlushFileBuffers(hPipe);
				DisconnectNamedPipe(hPipe);
				ZeroMemory(Request, sizeof(WCHAR) * BUFSIZE);
			    continue;
			}
			if(commandVerify){
				CreateLetterFromVolume(Request, letter);
				swprintf(buffer,L"Angel_Volume_%s_lock_mutex", letter);
				
				//mapValType
				ListMutex->insert(mapValType(wstring(letter), wstring(buffer)));
				ZeroMemory(buffer, sizeof(WCHAR) * SMALLBUF);
				ZeroMemory(letter, sizeof(WCHAR) * SIZELETTERWCHAR);
				POPERATION op = new OPERATION;
				op->Data = Request;
				op->size = cbBytesRead;
				runOperation(recieveCommand, op, Response);
				delete op;
				op = NULL;
				fSuccess = WriteFile( 
					hPipe,        // handle to pipe 
					Response,      // buffer to write from 
					BUFSIZE*sizeof(TCHAR), // number of bytes to write 
					&cbWritten,   // number of bytes written 
					NULL);        // not overlapped I/O 
				DisconnectNamedPipe(hPipe);
				commandVerify = false;
				IsCommand = false;
			}
		}
	}
	EndAllTask();
	ListThreads->EndAllThreads();
	*/
	return 0;
}

void EndAllTask()
{
	wstring ListActiveDisks, ListLockedDisks;
	for(map<wstring, Operations>::iterator it = ListCurrentBusyDisks->begin(); it != ListCurrentBusyDisks->end(); it++)
	{
		if(it->second == LOCK){
			ListLockedDisks += it->first;
			ListLockedDisks += ':;';
		}
		else if(it->second == ACTIVE){
			ListActiveDisks += it->first;
			ListActiveDisks += ':;';
		}
	}
	POPERATION op = new OPERATION;
	if(!ListActiveDisks.empty()){
		op->Data = (WCHAR*)ListActiveDisks.c_str();
		op->size = ListActiveDisks.size() * sizeof(WCHAR);
		runOperation(ENDALLACTIVETASK, op, NULL);
	}
	if(!ListLockedDisks.empty()){
		op->Data = (WCHAR*)ListLockedDisks.c_str();
		op->size = ListLockedDisks.size() * sizeof(WCHAR);
		runOperation(ENDALLLOCKEDTASK, op, NULL);
	}
	delete op;
}

int runOperation(Operations command, POPERATION op, wchar_t* resultResponce)
{
	int result = 0xFF;
	WCHAR buffer[SMALLBUF] = {0};
	HANDLE hx;
	switch(command){
		case LOCK:
			{
				vector<wstring> disks;
				ParsingData(op->Data, op->size, disks);
				wstring responces;
				wstring tmpStr;
				int DiskCounts = disks.size();
				int counter = 0;
				for(vector<wstring>::iterator iter = disks.begin(); iter != disks.end(); ++iter, counter++)
				{

					hx = CreateSemaphore(0,0,1,lockMutex);
					tmpStr = *iter;
					ARGUMENTS args;
					args.arg1 = NULL;
					args.arg2 = &tmpStr;
					args.arg3 = &result;
					ListThreads->AddThread(args, LockVolume);
					WaitForSingleObject(hx, INFINITE);
					CloseHandle(hx);
					if (!result)
					{
						swprintf(buffer,L"Volume %s lock successfully!", iter->c_str());
					}
					else
					{
						swprintf(buffer,L"Volume %s not lock with error %d!", iter->c_str(), result);
					}
					responces += buffer;
					responces.append(L";");
					ZeroMemory(buffer, SMALLBUF * sizeof(WCHAR));
				}
				lstrcpy(resultResponce, responces.c_str());
			}
			break;
		case UNLOCK:
			{
				vector<wstring> disks;	
				ParsingData(op->Data, op->size, disks);
				wstring responces;
				for(vector<wstring>::iterator iter = disks.begin() ; iter != disks.end(); ++iter)
				{
					result = UnlockVolume(*iter);
					if(!result){
						swprintf(buffer,L"Volume %s Unlock successfully!", iter->c_str());
						responces += buffer;
						responces.append(L";");
						ZeroMemory(buffer, SMALLBUF * sizeof(WCHAR));

					}
					else{
						swprintf(buffer,L"Volume %s not unlock with error %d!", iter->c_str(), result);
						responces += buffer;
						responces.append(L";");
						ZeroMemory(buffer, SMALLBUF * sizeof(WCHAR));
					}
				}
				lstrcpy(resultResponce, responces.c_str());
				GetLogicalDrives();
			}
			break;
		case ACTIVE:
			{
				vector<wstring> disks;	
				ParsingData(op->Data, op->size, disks);
				int DiskCounts = disks.size();
				wstring responces;
				HANDLE hx;
				wstring tmpStr;
				int counter = 0;
				InitializeCriticalSection(CriticalSection);

				for(vector<wstring>::iterator iter = disks.begin() ; iter != disks.end(); ++iter, counter++)
				{
					hx = CreateSemaphore(0,0,1,lockMutex);
					tmpStr = *iter;
					ARGUMENTS args;
					args.arg1 = NULL;
					args.arg2 = &tmpStr;
					args.arg3 = &result;
					ListThreads->AddThread(args, ProcessHDDIsActive);

					WaitForSingleObject(hx, INFINITE);
					CloseHandle(hx);
					if (!result)
					{
						swprintf(buffer,L"Volume %s Process Active successfully!", iter->c_str());
					}
					else
					{
						swprintf(buffer,L"Volume %s Process Active fail with error %d!", iter->c_str(), result);
					}
					responces += buffer;
					responces.append(L";");
					ZeroMemory(buffer, SMALLBUF * sizeof(WCHAR));
				}
				lstrcpy(resultResponce, responces.c_str());
			}
			break;
		case INACTIVE:
			{
				vector<wstring> disks;	
				ParsingData(op->Data, op->size, disks);
				wstring responces;
				int DiskCounts = disks.size();
				int result = 0;
				for(vector<wstring>::iterator iter = disks.begin() ; iter != disks.end(); ++iter)
				{
					result = ProcessHDDIsInActive(*iter);
					swprintf(buffer,L"Volume %s InActive successfully!", iter->c_str());
					responces += buffer;
					responces.append(L";");
					ZeroMemory(buffer, SMALLBUF * sizeof(WCHAR));
				}
				lstrcpy(resultResponce, responces.c_str());
			}
			break;
		case QUERY:
			{
				wstring responces;
				for(map<wstring, Operations>::iterator it = ListCurrentBusyDisks->begin(); it != ListCurrentBusyDisks->end(); it++)
				{
					if(it->second == LOCK){
						swprintf(buffer,L"Volume %s - Locked", it->first.c_str());
						responces += buffer;
						responces.append(L";");
					}
					else if (it->second == ACTIVE){
						swprintf(buffer,L"Volume %s - Active", it->first.c_str());
						responces += buffer;
						responces.append(L";");
					}
				}
				if(ListCurrentBusyDisks->empty())
					responces += L"No active operations!";
				lstrcpy(resultResponce, responces.c_str());
			}
			break;
		case ENDALLACTIVETASK:
			{
				vector<wstring> disks;	
				ParsingData(op->Data, op->size, disks);
				for(vector<wstring>::iterator iter = disks.begin() ; iter != disks.end(); ++iter)
				{
					ProcessHDDIsInActive(*iter);
				}
			}
			break;
		case ENDALLLOCKEDTASK:
			{
				vector<wstring> disks;	
				ParsingData(op->Data, op->size, disks);
				for(vector<wstring>::iterator iter = disks.begin() ; iter != disks.end(); ++iter)
				{
					UnlockVolume(*iter);
				}
			}
			break;
	}
	return result;
}
