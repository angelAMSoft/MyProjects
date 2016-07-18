#include "ManageThread.h"

CRITICAL_SECTION CriticalSectionThreads;

AbstractThread::AbstractThread()
{
	Thread = 0;
	status = NOTCREATE;
}

AbstractThread::AbstractThread(void* refThread)
{
	Thread = refThread;
	status = RUNNING;
	id = GetThreadId(refThread);
}

AbstractThread::~AbstractThread()
{
	Thread = 0;
	status = NOTCREATE;
}

unsigned int AbstractThread::GetId()
{
	return id;
}

void AbstractThread::SetStatus(StatusThread newStatus)
{
	status = newStatus;
}

void* AbstractThread::GetRef()
{
	return Thread;
}

StatusThread AbstractThread::GetStatus()
{
	return status;
}

void AbstractThread::AddRefThread(void* refThread)
{
	Thread = refThread;
	id = GetThreadId(refThread);
}

ArgumentsClass::ArgumentsClass()
{
	arg1 = NULL;
	arg2 = NULL;
	arg3 = NULL;
}

ArgumentsClass::ArgumentsClass(ARGUMENTS args)
{
	if(args.arg1 != NULL)
		arg1  = new std::wstring(args.arg1);
	else
		arg1 = NULL;
	if(args.arg2 != NULL)
		arg2  = new std::wstring(*args.arg2);
	else
		arg2 = NULL;
	if(args.arg3 != NULL)
		arg3  = args.arg3;
	else
		arg3 = NULL;
}

ArgumentsClass::ArgumentsClass(ArgumentsClass& other)
{
	std::wstring tmpStr;
	tmpStr = other.GetArg1();
	if(!tmpStr.empty())
		arg1 = new std::wstring(tmpStr);
	tmpStr.clear();
	tmpStr = other.GetArg2();
	if(!tmpStr.empty())
		arg2 = new std::wstring(other.GetArg2());
	int *tmpInt = other.GetArg3();
	if(tmpInt != NULL)
		arg3 = other.GetArg3();
}

ArgumentsClass::~ArgumentsClass()
{
	if(arg1 != NULL)
		delete arg1;
	if(arg2 != NULL)
		delete arg2;
	arg3 = NULL;
}

void ArgumentsClass::SetArg1(std::wstring& arg)
{
	*arg1 = arg;
}

void ArgumentsClass::SetArg2(std::wstring& arg)
{
	*arg2 = arg;
}

void ArgumentsClass::SetArg3(int* arg)
{
	arg3 = arg;
}

std::wstring  ArgumentsClass::GetArg1()
{
	std::wstring ArgStr;
	if(arg1 != NULL)
		ArgStr = *arg1;
	return ArgStr;
}

std::wstring  ArgumentsClass::GetArg2()
{
	std::wstring ArgStr;
	if(arg2 != NULL)
		ArgStr = *arg2;
	return ArgStr;
}

int* ArgumentsClass::GetArg3()
{
	int* ArgInt;
	if(arg3 != NULL)
		ArgInt = arg3;
	return ArgInt;
}

ManageThread::ManageThread()
{
	InitializeCriticalSection(&CriticalSectionThreads);
	ListThreads = new std::vector<AbstractThread>();
	ListArgs = new std::map<unsigned int, ArgumentsClass*>();
	StatusMonitor = RUNNING;
}


unsigned int ManageThread::AddThread(ARGUMENTS args, LPTHREAD_START_ROUTINE func)
{
	ArgumentsClass* newArg = new ArgumentsClass(args);
	HANDLE thread = CreateThread( 
									NULL,                   // default security attributes
									0,                      // use default stack size  
									func,       // thread function name
									newArg,          // argument to thread function 
									0,                      // use default creation flags 
									NULL);
	AbstractThread newThread(thread);
	DWORD threadId = newThread.GetId();
	EnterCriticalSection(&CriticalSectionThreads);
		ListThreads->push_back(newThread);
		ListArgs->insert(std::make_pair(threadId, newArg));
	LeaveCriticalSection(&CriticalSectionThreads);	
	return threadId;
}

ManageThread::~ManageThread()
{
	delete ListThreads;
	ListThreads = 0;
	delete ListArgs;
	ListArgs = 0;
	DeleteCriticalSection(&CriticalSectionThreads);
}

void ManageThread::DeleteThreads(std::vector<unsigned long> &ids)
{
	EnterCriticalSection(&CriticalSectionThreads);
	for(std::vector<unsigned long>::iterator itID = ids.begin(); itID != ids.end(); itID++){
		for(std::vector<AbstractThread>::iterator it = ListThreads->begin(); it != ListThreads->end(); it++)
		{
			if (it->GetId() == *itID){
				void* Handle = it->GetRef();
				CloseHandle(Handle);
				Handle = NULL;
				ListThreads->erase(it);
				break;
			}
		}
	}
	LeaveCriticalSection(&CriticalSectionThreads);	
}

void ManageThread::EndThread(unsigned int id)
{
	EnterCriticalSection(&CriticalSectionThreads);	
	for(std::vector<AbstractThread>::iterator it = ListThreads->begin(); it != ListThreads->end(); it++)
	{
		if (it->GetId() == id){
			it->SetStatus(COMPLETED);
		}
	}
	LeaveCriticalSection(&CriticalSectionThreads);	
}

void ManageThread::EndAllThreads()
{
	EnterCriticalSection(&CriticalSectionThreads);	
	for(std::vector<AbstractThread>::iterator it = ListThreads->begin(); it != ListThreads->end(); it++)
	{
			it->SetStatus(COMPLETED);
	}
	LeaveCriticalSection(&CriticalSectionThreads);
	FindCompletedThreads();
	SetStatusMonitor(COMPLETED);
}

void ManageThread::FindCompletedThreads()
{
	std::vector<DWORD> allowedToDeleteThreads;
	EnterCriticalSection(&CriticalSectionThreads);
	for(std::vector<AbstractThread>::iterator it = ListThreads->begin(); it != ListThreads->end(); it++)
	{
		if(it->GetStatus() == COMPLETED){
			DWORD thId = it->GetId();	
			std::map<unsigned int, ArgumentsClass*>::iterator SearchedArgs = ListArgs->find(thId);
			if(SearchedArgs != ListArgs->end())
			{
				delete SearchedArgs->second;
				ListArgs->erase(SearchedArgs);
			}
			allowedToDeleteThreads.push_back(thId);		
		}
	}
	LeaveCriticalSection(&CriticalSectionThreads);
	if(!allowedToDeleteThreads.empty())
		DeleteThreads(allowedToDeleteThreads);
}

void ManageThread::SetStatusMonitor(StatusThread newStatus)
{
	EnterCriticalSection(&CriticalSectionThreads);
	StatusMonitor = newStatus;
	LeaveCriticalSection(&CriticalSectionThreads);
}

StatusThread ManageThread::GetStatusMonitor()
{
	StatusThread curStatus;
	EnterCriticalSection(&CriticalSectionThreads);
		curStatus = StatusMonitor;
	LeaveCriticalSection(&CriticalSectionThreads);
	return curStatus;
}

DWORD WINAPI MonitorThread(LPVOID lpArg)
{
	PARGUMENTS arg = (PARGUMENTS)lpArg;
	ManageThread* manageTh = (ManageThread*)arg->arg3;
	while(manageTh->GetStatusMonitor() == RUNNING)
	{
		SleepEx(60 * 1000, FALSE);
		manageTh->FindCompletedThreads();
	}
  return 0;
}


void ManageThread::Monitor()
{
	//const int _SECOND = 10000000;
	ARGUMENTS args;
	args.arg1 = NULL;
	args.arg2 = NULL;
	args.arg3 = (int*)this;

    //HANDLE waitTimer = ::CreateWaitableTimer(NULL, TRUE,timername.c_str());
	//__int64 TimeInterval = -5 * _SECOND;
	//long lPeriod = 1800 * 1000;
	//::SetWaitableTimer(waitTimer, (LARGE_INTEGER*)&TimeInterval, lPeriod, TimerAPCProc, args, FALSE);

	DWORD threadId = AddThread(args, MonitorThread);
}


