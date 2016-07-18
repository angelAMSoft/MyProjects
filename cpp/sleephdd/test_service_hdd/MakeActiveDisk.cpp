#include "MakeActiveDisk.h"
#include "Misc.h"
#include "ProcessingQueue.h"
#include <PowrProf.h>


using std::map;
using std::wstring;

HANDLE CurrentActiveFile();


DWORD powerSettings()
{
	GUID* activeScheme = NULL;
	activeScheme = (GUID*)LocalAlloc(LPTR, sizeof(GUID));
	PowerGetActiveScheme(NULL, &activeScheme);
	DWORD result, status;
	status = PowerReadACValueIndex(NULL, activeScheme, &GUID_DISK_SUBGROUP, &GUID_DISK_POWERDOWN_TIMEOUT, &result);
	if(result == 0 ||result > 600){
		result = 360;
		PowerWriteACValueIndex(NULL, activeScheme, &GUID_DISK_SUBGROUP, &GUID_DISK_POWERDOWN_TIMEOUT, result);
	}
	LocalFree(activeScheme);
	activeScheme = NULL;
	return result;
}

int ProcessHDDIsInActive(std::wstring& disk)
{
	WCHAR letter[SIZELETTERWCHAR] = {0};
	CreateLetterFromVolume(disk.c_str(), letter);
	int result = 0;
	map<wstring, wstring>::iterator it = ListMutex->find(wstring(letter));
	map<wstring, wstring>::iterator itTimer;
	if (it != ListMutex->end()){
		HANDLE waitTimer = INVALID_HANDLE_VALUE;

		EnterCriticalSection(CriticalSection);	
			itTimer = ListTimers->find(wstring(letter));
	    LeaveCriticalSection(CriticalSection);	

		if(itTimer != ListTimers->end())
		{
			waitTimer = OpenWaitableTimer(TIMER_ALL_ACCESS, FALSE, itTimer->second.c_str());
			if(waitTimer != INVALID_HANDLE_VALUE)
				CancelWaitableTimer(waitTimer);
			ListTimers->erase(itTimer);
			CloseHandle(waitTimer);
		}
		ListMutex->erase(it);
		map<wstring, Operations>::iterator itOps = ListCurrentBusyDisks->find(wstring(letter));
		if(itOps != ListCurrentBusyDisks->end())
			ListCurrentBusyDisks->erase(itOps);
	}
	else
		result = 1;
	return result;
}

DWORD WINAPI ProcessHDDIsActive(LPVOID lpParam)
{
	WCHAR letter[SIZELETTERWCHAR] = {0};
	ArgumentsClass* arg = (ArgumentsClass*)lpParam;
	std::wstring volume = arg->GetArg2();
	int *out = arg->GetArg3();
	WCHAR buffer[SMALLBUF] = {0};
    HANDLE hx = 0, hx2;
	HANDLE waitTimer = INVALID_HANDLE_VALUE;
	CreateLetterFromVolume(volume.c_str(), letter);
	HANDLE fileActive = INVALID_HANDLE_VALUE;
	wstring PathToFile;
	DWORD err = 0;
    CreateLetterFromVolume(volume.c_str(), letter);
	swprintf(buffer,L"ANGEL_WAITABLE_TIMER_%s", letter);
	wstring timername(buffer);
	ListTimers->insert(mapValType(wstring(letter), wstring(buffer)));
	ZeroMemory(buffer, sizeof(WCHAR) * SMALLBUF);
	map<wstring, wstring>::iterator iter = ListMutex->find(wstring(letter));

	hx2 = OpenSemaphore(SEMAPHORE_ALL_ACCESS ,0,lockMutex);
	if (hx2 != 0)
	{
		ProcessingQueue* keeper = new ProcessingQueue(volume.c_str());
		if (keeper == NULL){
			err = GetLastError();
			*out = err;
			ReleaseSemaphore(hx2, 1, NULL);
		}
		
		if(keeper){
			WAIT_PARAMS idleParams;
			idleParams.CountPasses = 5;
			idleParams.timeWaitAfterPass = 20;
			idleParams.timePass = 3;
			idleParams.CurrentHddSleepTimeout = powerSettings();
			keeper->SetParams(idleParams);
			keeper->PrepareQueue();
			*out = 0;
			ReleaseSemaphore(hx2, 1, NULL);
			waitTimer = CreateWaitableTimer(NULL, TRUE,timername.c_str());
			__int64 TimeInterval = -5 * _SECOND;

			unsigned int partT = (idleParams.timeWaitAfterPass + idleParams.timePass) * idleParams.CountPasses * 0.1;

			long lPeriod = (idleParams.CurrentHddSleepTimeout / 2 - partT) * 1000;
			//long lPeriod = 20 * 1000; //* 1000

			ListCurrentBusyDisks->insert(make_pair(wstring(letter), ACTIVE));
			SetWaitableTimer(waitTimer, (LARGE_INTEGER*)&TimeInterval, lPeriod, TimerAPCProc, keeper, FALSE);
			map<wstring, wstring>::iterator itTimer = ListTimers->find(wstring(letter));
			while(itTimer != ListTimers->end()){
				SleepEx(INFINITE, TRUE);
				EnterCriticalSection(CriticalSection);
				itTimer = ListTimers->find(wstring(letter));
				LeaveCriticalSection(CriticalSection);	
			}
		}
	}
	DeleteCriticalSection(CriticalSection);
	DWORD curThreadId = GetCurrentThreadId();
	ListThreads->EndThread(curThreadId);
	return 0;


	//	if(IsFileToHDDExists(volume.c_str(), PathToFile))
	//	{
	//		fileActive = CreateFile(PathToFile.c_str(), 
	//					   GENERIC_READ,          // open for reading
	//					   FILE_SHARE_READ,       // share for reading
	//					   NULL,                  // default security
	//					   OPEN_EXISTING,         // existing file only
	//					   FILE_ATTRIBUTE_NORMAL, // normal file
	//					   NULL);
	//	}else{
	//		fileActive = CreateFile(PathToFile.c_str(),         // open for reading
	//					   GENERIC_READ | GENERIC_WRITE,
	//					   NULL,				// share for reading
	//					   NULL,                  // default security
	//					   CREATE_NEW,         // existing file only
	//					   FILE_ATTRIBUTE_NORMAL, // normal file
	//					   NULL);
	//		WCHAR data[] = L"TestData";
	//		DWORD dwwriten = 0;
	//		WriteFile(fileActive, data, sizeof(data), &dwwriten, NULL);
	//	}
	//	if (fileActive == INVALID_HANDLE_VALUE) 
	//	{ 
	//		err = GetLastError();
	//		*out = err;
	//	}else{
	//		DWORD dwBytesRead = 0;
	//		if( FALSE == ReadFile(fileActive, buffer, SMALLBUF, &dwBytesRead, NULL) )
	//		{
	//			err = GetLastError();
	//			*out = err;
	//			CloseHandle(fileActive);
	//		}else{
	//			CloseHandle(fileActive);
	//			*out = 0;
	//			ReleaseSemaphore(hx2, 1, NULL);
	//			waitTimer = CreateWaitableTimer(NULL, TRUE,timername.c_str());
	//			__int64 TimeInterval = -5 * _SECOND;
	//			long lPeriod = 20 * 1000;
	//			ARGUMENTS args;
	//			args.arg1 = (WCHAR*)PathToFile.c_str();
	//			args.arg2 = NULL;
	//			args.arg3 = NULL;
	//			ListCurrentBusyDisks->insert(make_pair(wstring(letter), ACTIVE));
	//			SetWaitableTimer(waitTimer, (LARGE_INTEGER*)&TimeInterval, lPeriod, TimerAPCProc, &args, FALSE);
	//			map<wstring, wstring>::iterator itTimer = ListTimers->find(wstring(letter));
	//			while(itTimer != ListTimers->end()){
	//				SleepEx(INFINITE, TRUE);
	//				EnterCriticalSection(CriticalSection);
	//				itTimer = ListTimers->find(wstring(letter));
	//				LeaveCriticalSection(CriticalSection);	
	//			}
	//		}
	//		ReleaseSemaphore(hx2, 1, NULL);
	//	}
	//	ReleaseSemaphore(hx2, 1, NULL);
	//}
	//DeleteCriticalSection(CriticalSection);
	//DWORD curThreadId = GetCurrentThreadId();
	//ListThreads->EndThread(curThreadId);
	//return 0;
}


VOID CALLBACK TimerAPCProc(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	//PARGUMENTS arg = (PARGUMENTS)lpArg;
	//wchar_t* path = arg->arg1;
	//WCHAR buffer[SMALLBUF] = {0};
	//DWORD dwBytesRead = 0;
	//HANDLE fileActive = CreateFile(path, GENERIC_READ,          // open for reading
	//					   FILE_SHARE_READ,       // share for reading
	//					   NULL,                  // default security
	//					   OPEN_EXISTING,         // existing file only
	//					   FILE_ATTRIBUTE_NORMAL, // normal file
	//					   NULL);
	//if(fileActive != INVALID_HANDLE_VALUE)
	//	ReadFile(fileActive, buffer, SMALLBUF, &dwBytesRead, NULL);
	//CloseHandle(fileActive);
	ProcessingQueue* keeper = (ProcessingQueue*)lpArg;
	keeper->ActiveDisk();
}