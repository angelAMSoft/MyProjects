#ifndef PROCESSING_QUEUE_H
#define PROCESSING_QUEUE_H

#include <stack>
#include <string>
#include <vector>
#include <Windows.h>

enum STATUS{
	PROCESSING_SUCCESS,
	PROCESSING_ERROR,
	PROCESSING_QUEUE_FULL,
	PROCESSING_NOFILES,
	PROCESSING_NODIRS,
	PROCESSING_READY_TO_ACTIVE
};

typedef struct{
	unsigned int timePass;
	unsigned int timeWaitAfterPass;
	unsigned int CurrentHddSleepTimeout;
	unsigned int CountPasses;
}WAIT_PARAMS;

typedef std::vector<std::wstring> LISTDIRTYPE;


class ProcessingQueue{
public:
	ProcessingQueue(const wchar_t* root, unsigned int max = 10);
	~ProcessingQueue();
	void PrepareQueue();
	bool ActiveDisk();
	void SetParams(WAIT_PARAMS newParams);

private:
	//STATUS CreateDirList(const wchar_t *root);
	//STATUS CreateDirList();
	void CreateDirList();
	void ChangeDir(const wchar_t *path, bool child = true);
	bool IsAlreadyTraverse(const wchar_t *path);
	bool FileInList(const wchar_t* fileName);
	void ClearStacks();
	//STATUS CheckFiles();
	void CheckFiles();
	std::stack<std::wstring> *dirs;
	std::deque<std::wstring>* fileProcessing;
	std::stack<std::wstring> *listDirs;
	LISTDIRTYPE *listFiles;
	LISTDIRTYPE *TraversedDirs;
	std::wstring* curDir;
	std::wstring* rootDir;
	unsigned int maxLenght;
	unsigned int currentIndex;
	STATUS LastStatus;
	std::wstring Get();
	WAIT_PARAMS waitParams;
};

#endif //PROCESSING_QUEUE_H