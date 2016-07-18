#ifndef MANAGETHREAD_H
#define MANAGETHREAD_H
#include <vector>
#include <Windows.h>
#include <string>
#include <map>

enum StatusThread{
	RUNNING,
	COMPLETED,
	NOTCREATE
};


typedef struct Arguments {
    WCHAR* arg1;
	std::wstring *arg2;
    int *arg3;
} ARGUMENTS, *PARGUMENTS;

class ArgumentsClass{
public:
	ArgumentsClass();
	ArgumentsClass(ARGUMENTS args);
	ArgumentsClass(ArgumentsClass& other);
	~ArgumentsClass();
	void SetArg1(std::wstring& arg);
	void SetArg2(std::wstring& arg);
	void SetArg3(int* arg);
	std::wstring GetArg1();
	std::wstring GetArg2();
	int* GetArg3();

private:
	std::wstring* arg1;
    std::wstring* arg2;
    int *		  arg3;
};

class AbstractThread{
public:
	AbstractThread(void* refThread);
	AbstractThread();
	~AbstractThread();
	void AddRefThread(void* refThread);
	void* GetRef();
	void SetStatus(StatusThread newStatus);
	unsigned int GetId();
	StatusThread GetStatus();
private:
	void* Thread;
	StatusThread status;
	unsigned int id;
};

class ManageThread{
public:
	ManageThread();
	~ManageThread();
	unsigned int AddThread(ARGUMENTS args, LPTHREAD_START_ROUTINE func);
	void EndThread(unsigned int id);
	void EndAllThreads();
	void DeleteThreads(std::vector<unsigned long> &ids);
	void Monitor();
	void ManageThread::FindCompletedThreads();
	void SetStatusMonitor(StatusThread newStatus);
	StatusThread GetStatusMonitor();

private:
	std::vector<AbstractThread>* ListThreads;
	std::map<unsigned int, ArgumentsClass*>* ListArgs;
	StatusThread StatusMonitor;
};
#endif //MANAGETHREAD_H
