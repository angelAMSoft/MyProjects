#include "LockVolume.h"
#include "Misc.h"
using std::wstring;
using std::map;

DWORD WINAPI  LockVolume(LPVOID lpParam)
{
	HANDLE hDevice; DWORD lpBytesReturned;
	int result = 0;
	WCHAR letter[SIZELETTERWCHAR] = {0};
	ArgumentsClass* arg = (ArgumentsClass*)lpParam;
	std::wstring volume = arg->GetArg2();
	int *out = arg->GetArg3();
	HANDLE hx, hx2;
	CreateLetterFromVolume(volume.c_str(), letter);
	map<wstring, Operations>::iterator itOps = ListCurrentBusyDisks->find(wstring(letter));
	bool alreadyLocked = false;
	WCHAR buffer[32] = {0};
	hx2 = OpenSemaphore(SEMAPHORE_ALL_ACCESS ,0,lockMutex);
	if(itOps != ListCurrentBusyDisks->end())
		if(itOps->second == LOCK){
			alreadyLocked = true;
			*out = 1;
			ReleaseSemaphore(hx2, 1, NULL);
		}

	if(!alreadyLocked){
		hDevice = CreateFile(volume.c_str(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,0,0);
		if (hDevice == INVALID_HANDLE_VALUE)
			result = GetLastError();
		else
		{
			if(!DeviceIoControl(hDevice,FSCTL_LOCK_VOLUME,NULL,0, NULL,0,&lpBytesReturned,0))
			{
				result = GetLastError();
			}
			if(!result)
				ListCurrentBusyDisks->insert(make_pair(wstring(letter), LOCK)); 
		}
		*out = result;
		ReleaseSemaphore(hx2, 1, NULL);
		swprintf(buffer,L"Angel_Volume_%s_lock_mutex", letter);
		ListMutex->insert(mapValType(wstring(letter), wstring(buffer)));
		if(!result){
			hx = CreateSemaphore(0,0,1,buffer);
			WaitForSingleObject(hx,INFINITE);
		}
	}
	DWORD curThreadId = GetCurrentThreadId();
	ListThreads->EndThread(curThreadId);
	return 0;





	/*
	map<wstring, wstring>::iterator iter = ListMutex->find(wstring(letter));
	if (iter != ListMutex->end())
		hx = OpenSemaphore(SEMAPHORE_ALL_ACCESS ,0,iter->second.c_str());
	hx2 = OpenSemaphore(SEMAPHORE_ALL_ACCESS ,0,lockMutex);
	CreateVolumePath(volume);
	ReleaseSemaphore(hx, 1, NULL);
	if (hx == 0)
	{
		hDevice = CreateFile(volume.c_str(),GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,0,0);
		if (hDevice == INVALID_HANDLE_VALUE)
			result = GetLastError();
		else
		{
			if(!DeviceIoControl(hDevice,FSCTL_LOCK_VOLUME,NULL,0, NULL,0,&lpBytesReturned,0))
			{
				result = GetLastError();
			}
			if(!result)
				ListCurrentBusyDisks->insert(make_pair(wstring(letter), LOCK)); 
		}
		*out = result;
		ReleaseSemaphore(hx2, 1, NULL);
		if(!result){
			hx = CreateSemaphore(0,0,1,iter->second.c_str());
			WaitForSingleObject(hx,INFINITE);
		}
	}
	DWORD curThreadId = GetCurrentThreadId();
	ListThreads->EndThread(curThreadId);
	return 0;
	*/
}

int UnlockVolume(std::wstring& disk)
{
	HANDLE hx;
	WCHAR letter[SIZELETTERWCHAR] = {0};
	int counter = 0;
	CreateLetterFromVolume(disk.c_str(), letter);
	map<wstring, wstring>::iterator it = ListMutex->find(wstring(letter));
	if (it != ListMutex->end()){
		hx = OpenSemaphore(SEMAPHORE_ALL_ACCESS ,0,it->second.c_str());
		ReleaseSemaphore(hx, 1, NULL);
		ListMutex->erase(it);
	}
	map<wstring, Operations>::iterator itOps = ListCurrentBusyDisks->find(wstring(letter));
	if(itOps != ListCurrentBusyDisks->end())
		ListCurrentBusyDisks->erase(itOps);
	CloseHandle(hx);
	return 0;
}