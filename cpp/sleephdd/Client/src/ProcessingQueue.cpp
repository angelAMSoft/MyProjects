#include "ProcessingQueue.h"

//const int maxLenght = 10;

ProcessingQueue::ProcessingQueue(const wchar_t* dir, unsigned int max)
{
	maxLenght = max;
	currentIndex = 0;
	fileProcessing = new std::deque<std::wstring>();
	dirs = new std::stack<std::wstring>();
	listDirs = new std::stack<std::wstring>();
	listFiles = new LISTDIRTYPE;
	TraversedDirs = new  LISTDIRTYPE;
	LastStatus = PROCESSING_SUCCESS;
	curDir = new std::wstring(dir);
	rootDir = new std::wstring(dir);
}

ProcessingQueue::~ProcessingQueue()
{
	delete fileProcessing;
	delete TraversedDirs;
	delete dirs;
	delete listDirs;
	delete listFiles;
	delete curDir;
	delete rootDir;
}

void ProcessingQueue::PrepareQueue()
{
	CreateDirList();
}

void ProcessingQueue::SetParams(WAIT_PARAMS newParams)
{
	waitParams = newParams;
}

void ProcessingQueue::CheckFiles()
{	
	if(listFiles->empty()){
		LastStatus = PROCESSING_NOFILES;
		return;
	}
	for(LISTDIRTYPE::iterator file = listFiles->begin(); file != listFiles->end(); ++file){
		std::wstring path(*curDir);
		path += *file;
		HANDLE fileActive = CreateFile(path.c_str(), 
						   GENERIC_READ,          // open for reading
						   FILE_SHARE_READ,       // share for reading
						   NULL,                  // default security
						   OPEN_EXISTING,         // existing file only
						   FILE_ATTRIBUTE_NORMAL, // normal file
						   NULL);
		DWORD err = GetLastError();
		if(fileActive != INVALID_HANDLE_VALUE){
			fileProcessing->push_back(path);
			CloseHandle(fileActive);
		}
		if(fileProcessing->size() == maxLenght){
			LastStatus = PROCESSING_QUEUE_FULL;
			break;
		}
	}	
}

void ProcessingQueue::ChangeDir(const wchar_t *path, bool child)
{
	if(child){
		dirs->push(*curDir);
		*curDir += L"\\";
		*curDir += path;
		*curDir += L"\\";
	}
	else{
		*curDir = dirs->top();	
		dirs->pop();
	}
}
void ProcessingQueue::ClearStacks()
{
	while(!listDirs->empty())
		listDirs->pop();
	if(!listFiles->empty())
		listFiles->clear();

}

bool ProcessingQueue::IsAlreadyTraverse(const wchar_t *path)
{
	bool result = false;
	std::wstring searchedPath(path);
	for(LISTDIRTYPE::iterator it = TraversedDirs->begin(); it != TraversedDirs->end(); ++it){
		if(*it == searchedPath){
			result = true;
			break;
		}
	}
	return result;
}

bool ProcessingQueue::FileInList(const wchar_t* fileName)
{
	bool result = false;
	std::wstring searchedFile(fileName);
	for(std::deque<std::wstring>::iterator it = fileProcessing->begin(); it != fileProcessing->end(); ++it)
	{
		if(*it == searchedFile){
			result = true;
			break;
		}
	}
	return result;
}

void ProcessingQueue::CreateDirList()
{
	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;
	std::wstring Dir(*curDir);
	Dir += L"\\*";
	ClearStacks();
	hFind = FindFirstFile(Dir.c_str(), &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		LastStatus = PROCESSING_ERROR;
		return;
	}
	do{
		if(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if((ffd.cFileName[0] == L'.') && (ffd.cFileName[1] == L'\0' || ffd.cFileName[1] == L'.'))
				continue;
			if(ffd.cFileName[0] == L'$')
				continue;
			std::wstring checkedPath(*curDir);
			checkedPath += ffd.cFileName;
			checkedPath += L"\\";

			if(!IsAlreadyTraverse(checkedPath.c_str()))
				listDirs->push(std::wstring(ffd.cFileName));
		}
		else {
			std::wstring checkedFile(*curDir);
			checkedFile += ffd.cFileName;
			if(!FileInList(checkedFile.c_str()))
				listFiles->push_back(std::wstring(ffd.cFileName));
		}
	}while(FindNextFile(hFind, &ffd) != 0);

	CheckFiles();
	if(listDirs->empty()){
		LastStatus = PROCESSING_NODIRS;
	}

	while(LastStatus != PROCESSING_READY_TO_ACTIVE){
		switch(LastStatus){
		case PROCESSING_QUEUE_FULL:
			LastStatus = PROCESSING_READY_TO_ACTIVE;
			break;
		case PROCESSING_NODIRS:
			if (curDir == rootDir && listDirs->empty())
				LastStatus = PROCESSING_READY_TO_ACTIVE;
			else{
				ChangeDir(NULL, false);
				//
				LastStatus = PROCESSING_SUCCESS;
				goto exitDir;		
			}
			break;

		case PROCESSING_NOFILES:
		case PROCESSING_SUCCESS:
			if(!listDirs->empty()){
				std::wstring NextDir(*curDir);
				NextDir += listDirs->top();
				NextDir += L"\\";
				TraversedDirs->push_back(NextDir);
				ChangeDir(listDirs->top().c_str());
				listDirs->pop();
			}
			CreateDirList();
		}
	}

exitDir:
	FindClose(hFind);
	;
}

std::wstring ProcessingQueue::Get()
{
	if(currentIndex == fileProcessing->size())
		currentIndex = 0;
	std::wstring value(fileProcessing->at(currentIndex));
	currentIndex++;
	return value;
}

bool ProcessingQueue::ActiveDisk()
{
	//const int count = 5;
	LARGE_INTEGER fullSize;
	HANDLE curFile = INVALID_HANDLE_VALUE;
	std::wstring fileName = Get();
	curFile = CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
						 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
	if(INVALID_HANDLE_VALUE == curFile)
		return false;
	if (!GetFileSizeEx(curFile, &fullSize))
		return false;

	//const int BUFFER_SIZE = 4096;
	//char buffer[BUFFER_SIZE] = {0};
	//DWORD bytesPerSector;
	//GetDiskFreeSpace(rootDir->c_str(), NULL, &bytesPerSector, NULL, NULL); 
	unsigned int TargetFileSize = fullSize.LowPart > 73400320 ? 73400320 : fullSize.LowPart;
	char* buffer = new char[1024*1024];
	//buffer[40] = '\0';
	DWORD Readedsize = 0, ReadFromFile = 0;	
	for(int i = 0; i < waitParams.CountPasses; ++i){
		if(INVALID_HANDLE_VALUE == curFile)
			curFile = CreateFile(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
								 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING, NULL);
		while(Readedsize < TargetFileSize){
			ReadFile(curFile, buffer, 1024*1024, &ReadFromFile, NULL);
			//buffer[40] = '\0';
			Readedsize += ReadFromFile;
		}
		Readedsize = 0;
		CloseHandle(curFile);
		curFile = INVALID_HANDLE_VALUE;
		SleepEx(waitParams.timeWaitAfterPass *1000, FALSE);
	}
	delete buffer;
	buffer = NULL;
	if(INVALID_HANDLE_VALUE != curFile)
		CloseHandle(curFile);
	return true;
}


