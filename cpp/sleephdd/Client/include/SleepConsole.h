#ifndef SLEEPCONSOLE_H
#define SLEEPCONSOLE_H

#include <string>
#include <stack>
//#include <queue>
#include <list>
#include <map>
#include <utility>
#include <iostream>
#include "QueryDisks.h"
#include "SleppHDDClients.h"
#include "ManagerBusyDisk.h"

enum CONSOLE_SECTIONS{
	SECTION_TITLE,
	SECTION_LISTDISK
};

enum TYPE_COMMAND{
	ADD_COMMAND,
	CLEAR_COMMAND
};

struct CommandAndLenght{
	char* command;
	int lenght;
};

class SleepConsole{
public:
	SleepConsole();
	~SleepConsole();
	CONSOLE_SECTIONS GetSection();
	void SetSection(CONSOLE_SECTIONS newSection);	
	void ConsoleSleepHdd();
	void PrintHelpCommandMessage();
private:
	void AddOperation(Operations operation, std::vector<std::wstring> &partitions); 
	void AddToQueue(std::pair<std::wstring,Operations> &task); 
	bool FindCommand(std::wstring &input);
	bool FindListSubCommand(std::wstring& input);
	bool PreparingCommand();
	bool PreparingActiveCommand();
	bool PreparingLockCommand();
	bool GenerateCommandToSend(std::wstring& arguments = std::wstring());
	bool ApplyCommand();
	void GetCurrentOperations(wchar_t* responce, int lenght);
	bool DiskBusyAsTask(Operations task, int diskNumber);
	bool AppyPendingOperations();
	void ClearListOfPendingOperations();

	//Operations DiskTask(int number);

	CommandAndLenght commandToSend;
	Operations consoleOperation;
	CONSOLE_SECTIONS currentSection;
	std::vector<DISKINFORMATION> disks;
	//std::vector<std::pair<int, Operations> > BusyDisks;
	ManagerBusyDisk* BusyDisks;
	std::map<std::wstring, Operations> *CurrentOperations;
	//std::queue<std::pair<std::wstring, Operations> > *sleephddQueue;
	std::list<std::pair<std::wstring, Operations> > *sleephddQueue;
	std::stack<std::pair<std::wstring, Operations> > *newOperations;
};

#endif //SLEEPCONSOLE_H