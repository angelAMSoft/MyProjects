
#include "SleepConsole.h"
#include "Misc.h"
using namespace std;

SleepConsole::SleepConsole()
{
	CurrentOperations = new std::map<std::wstring, Operations>;
	//sleephddQueue = new std::queue<std::pair<std::wstring, Operations> >();
	sleephddQueue = new std::list<std::pair<std::wstring, Operations> >;
	newOperations = new std::stack<std::pair<std::wstring, Operations> >;
	BusyDisks = new ManagerBusyDisk;
	currentSection = SECTION_TITLE;
	consoleOperation = NO_OPERATION;
	commandToSend.command = NULL;
}

SleepConsole::~SleepConsole()
{
	delete CurrentOperations;
	delete sleephddQueue;
	delete newOperations;
}

CONSOLE_SECTIONS SleepConsole::GetSection()
{
	return currentSection;
}

void SleepConsole::SetSection(CONSOLE_SECTIONS section)
{
	currentSection = section;
}

void SleepConsole::AddToQueue(std::pair<std::wstring,Operations> &task)
{
	sleephddQueue->push_back(task);
	wstring msg;
	if(task.second == LOCK){
		msg += L"Task lock partition: ";
		msg += task.first;
	}
	else if(task.second == ACTIVE){
		msg += L"Task active partition: ";
		msg += task.first;
	}
	wcout << msg << " added to queue!" << endl;
}

void SleepConsole::AddOperation(Operations operation, std::vector<std::wstring> &partitions)
{
	switch(operation){
	case LOCK:
		{
			for(vector<wstring>::const_iterator iter = partitions.begin(); iter != partitions.end(); ++iter){
				AddToQueue(make_pair(*iter, operation));
				newOperations->push(make_pair(*iter, operation));
				map<std::wstring, Operations>::iterator lockedPartition = CurrentOperations->find(*iter);
				if(lockedPartition != CurrentOperations->end()){
					newOperations->push(make_pair(*iter, operation));
					if(lockedPartition->second == LOCK)
						continue;
					else if (lockedPartition->second == ACTIVE){
						newOperations->push(make_pair(*iter, INACTIVE));
					}
				}
			}
		}
		break;
	case ACTIVE:
		{
			for(vector<wstring>::const_iterator iter = partitions.begin(); iter != partitions.end(); ++iter){
				AddToQueue(make_pair(*iter, operation));
				newOperations->push(make_pair(*iter, operation));
				map<std::wstring, Operations>::iterator lockedPartition = CurrentOperations->find(*iter);
				if(lockedPartition != CurrentOperations->end()){
					newOperations->push(make_pair(*iter, operation));
					if(lockedPartition->second == ACTIVE)
						continue;
					else if (lockedPartition->second == LOCK){
						newOperations->push(make_pair(*iter, UNLOCK));
					}
				}
			}
		}
		break;
	}
	consoleOperation = NO_OPERATION;
}

void SleepConsole::ConsoleSleepHdd()
{
	collectDiskInformation(disks);
	wstring input;
	bool isNotQuitCommand = true;
	do{
		cout << "\nEnter command: ";
		//wcin >> input;
		wcin.sync();
		getline(wcin, input);
		if(input.empty())
			continue;
		isNotQuitCommand = FindCommand(input);		
	}while(isNotQuitCommand);

}

void SleepConsole::PrintHelpCommandMessage()
{
	cout << "Sleep HDD console client." << endl;
	cout << "Copyright AM Software Group.(2013)" << endl;
	cout << "Main developer - Mikheev Alexey" << endl;
}

bool SleepConsole::FindCommand(std::wstring &input)
{
	wchar_t* listCommand = L"list";
	wchar_t* lockCommand = L"lock";
	//wchar_t* unlockCommand = L"unlock";
	wchar_t* activeCommand = L"active";
	//wchar_t* unactiveCommand = L"inActive";
	wchar_t* quitCommand = L"quit";
	wchar_t* ClearPendingOperations = L"delete pending operations";
	wchar_t* ApplyOperations = L"apply operations";
	if(input.find(quitCommand) != wstring::npos)
		return false;
	else if(input.find(listCommand) != wstring::npos){
		wstring subcommand;
		int pos = input.find(listCommand);
		subcommand = input.substr(pos + lstrlen(listCommand));
		FindListSubCommand(subcommand);
	}
	else if (input.find(lockCommand) != wstring::npos){
		consoleOperation = LOCK;
		PreparingCommand();
	}
	else if (input.find(activeCommand) != wstring::npos){
		consoleOperation = ACTIVE;
		PreparingCommand();
	}
	else if(input.find(ClearPendingOperations) != wstring::npos){
		ClearListOfPendingOperations();
		cout << "Task clear list pending operations complete!" << endl;
	}
	else if(input.find(ApplyOperations) != wstring::npos){
		if(!AppyPendingOperations())
			cout << "\nEmpty list pending operations" << endl;
		else
			cout << "\nAll planned operations completed!" << endl;
	}
	else{
		cout << "Not supported command!\n";
	}

	return true;
}

void SleepConsole::GetCurrentOperations(wchar_t* responce, int lenght)
{
	vector<wstring> results;
	if(consoleOperation == QUERY){
		wstring entry;
		for(int i = 0; i< lenght; ++i){
			if(responce[i] != L';')
				entry += responce[i];
			else{
				results.push_back(entry);
				entry.clear();
			}
		}
		if(entry.find(L"No active operations!") != wstring::npos){
			wcout << L"No active operations!" << endl;
			return;
		}
		if(!CurrentOperations->empty())
			CurrentOperations->clear();

		for(vector<wstring>::const_iterator iter = results.begin(); iter != results.end(); iter++){
			int pos = iter->find(L'-');
			wstring cmd = iter->substr(pos + 2);
			wstring vol;
			vol += iter->at(pos - 2);
			if(cmd.find(L"Locked") != wstring::npos)
				CurrentOperations->insert(make_pair(vol, LOCK));
			else if(cmd.find(L"Active") != wstring::npos){
				CurrentOperations->insert(make_pair(vol, ACTIVE));
			}
		}

		BusyDisks->ClearBusyList();
		for(vector<DISKINFORMATION>::const_iterator iter = disks.begin(); iter != disks.end(); ++iter){
			if(DiskBusyAsTask(LOCK, iter->DiskNumber))
				BusyDisks->AddToBusyList(LOCK, iter->DiskNumber);
			else if(DiskBusyAsTask(ACTIVE, iter->DiskNumber))
				BusyDisks->AddToBusyList(ACTIVE,iter->DiskNumber);
		}
	}
}

bool SleepConsole::DiskBusyAsTask(Operations task, int diskNumber)
{
	bool result = false;
	DISKINFORMATION disk = disks.at(diskNumber);
	int BusyPartitions = 0;
	for(std::vector<VOLUMEPROPERTIES>::const_iterator itVol = disk.VOLUMES.begin(); itVol != disk.VOLUMES.end(); ++itVol){
		map<std::wstring, Operations>::iterator BusyPartition = CurrentOperations->find(itVol->MountPoint);
		if(BusyPartition != CurrentOperations->end() && BusyPartition->second == task){
			BusyPartitions++;
		}
	}
	if(task == LOCK && BusyPartitions == disk.VOLUMES.size())
		result = true;
	else if(task == ACTIVE && BusyPartitions > 0)
		result = true;

	return result;
}

bool SleepConsole::ApplyCommand()
{
	DWORD cbRead = 0, cbWritten = 0; 
	WCHAR responce[BUFSIZE] = {0};
    BOOL fSuccess;
	DWORD error = 1;
	if(commandToSend.command == NULL)
		return false;

	while (!WaitNamedPipeA(lpszPipename, NMPWAIT_WAIT_FOREVER) && error != 0){
		error = GetLastError();
		if (error == 2){
			cout << "Service Hdd sleep not started!";
			return false;
		}
	}
	fSuccess = CallNamedPipeA(lpszPipename, commandToSend.command, commandToSend.lenght,
							responce, BUFSIZE*sizeof(TCHAR), &cbRead, NMPWAIT_USE_DEFAULT_WAIT);

	if (consoleOperation == QUERY){
		GetCurrentOperations(responce, cbRead);
		//consoleOperation = NO_OPERATION;
	}

	for(int i = 0; i < cbRead; i++){
		if(responce[i] == L';')
			responce[i] = L'\n';
	}
	wcout << responce << endl;
	consoleOperation = NO_OPERATION;
	return true;
}

bool SleepConsole::GenerateCommandToSend(std::wstring& arguments)
{
	bool result = false;
	int lenght;
	string cmd;
	switch(consoleOperation){
	case QUERY:
			cmd += "Command:Query";
			arguments = L"ALL";
			result = true;
			break;

	case LOCK:
			cmd += "Command:Lock";
			result = true;
			break;
		
	case ACTIVE:
			cmd += "Command:Active";
			result = true;
			break;

	case UNLOCK:
			cmd += "Command:Unlock";
			result = true;
			break;

	case INACTIVE:
			cmd += "Command:InActive";
			result = true;
			break;	

	}

	if(result){
		char *arg = WideToChar(arguments.c_str());
		commandToSend.command = CreateBufferToSend(const_cast<char*>(cmd.c_str()), arg, &lenght);
		delete arg;
		arg = NULL;
		commandToSend.lenght = lenght;
	}
	
	return result;
}

/*
Operations SleepConsole::DiskTask(int number)
{
	return NO_OPERATION;
}
*/

bool SleepConsole::PreparingActiveCommand()
{
	bool result = false;
	int diskCounts = 0;
	WCHAR winDir[MAX_PATH] = {0};
	GetWindowsDirectory(winDir, MAX_PATH);
	WCHAR systemDisk[4];
	systemDisk[0] = winDir[0];
	systemDisk[1] = winDir[1];
	systemDisk[2] = winDir[2];
	systemDisk[3] = L'\0';
	int systemDiskNumber = 0xFF;
	cout << "\nDisk list for ACTIVE command: " << endl;
	for(vector<DISKINFORMATION>::const_iterator iter = disks.begin(); iter != disks.end(); ++iter, diskCounts++){
		bool NextDisk = false;
		for(std::vector<VOLUMEPROPERTIES>::const_iterator itVol = iter->VOLUMES.begin(); itVol != iter->VOLUMES.end(); ++itVol){
			if(wstring::npos != itVol->MountPoint.find(systemDisk)){
				systemDiskNumber = iter->DiskNumber;
				NextDisk = true;
				break;
			}
		}
		if (NextDisk)
			continue;

		int diskNum = iter->DiskNumber;
		int operation = BusyDisks->CurrentOperation(diskNum);
		if (operation == 0xFF)
			operation = NO_OPERATION;

		if(operation == consoleOperation)
			continue;

		wstring diskName(L"Disk ");
		diskNum = iter->DiskNumber;
		wchar_t buff[2] = {0};
		_itow(diskNum, buff, 10);
		diskName += buff;
		int DiskSizeInGB = iter->size / 1073741824;
		wcout << L"[" << buff << L"] " << diskName << L"(" << iter->Model << L" size: " << DiskSizeInGB << L" GB" << L")" << endl;		
	}
	cout << "[" << diskCounts << "] " << "ALL" <<endl;
	int abortCommand = diskCounts + 1;
	cout << "[" << abortCommand << "] " << "Abort" << endl;

	int choice = 0;
	while(diskCounts){
		cout << "\nChoice disk: ";
		cin >> choice;
		if(choice >= 0 && choice <= diskCounts)
			break;
		else if(choice == abortCommand){
			cout << "\nCancel operation." << endl;
			return result;
		}
		else
			cout <<"\nDisk not found!" << endl;
	}
	result = true;
	vector<wstring> partitions;
	if(choice == diskCounts){
		for(vector<DISKINFORMATION>::const_iterator iter = disks.begin(); iter != disks.end(); ++iter){
			if (iter->DiskNumber == systemDiskNumber)
				continue;
			for(std::vector<VOLUMEPROPERTIES>::const_iterator itVol = iter->VOLUMES.begin(); itVol != iter->VOLUMES.end(); ++itVol){
				map<std::wstring, Operations>::iterator BusyPartition = CurrentOperations->find(itVol->MountPoint);
				if(BusyPartition == CurrentOperations->end() || (BusyPartition != CurrentOperations->end() && BusyPartition->second != consoleOperation)) //LOCK
					partitions.push_back(itVol->MountPoint);
			}
		}
	}
	else{
		DISKINFORMATION choicedDisk = disks.at(choice);
		VOLUMEPROPERTIES volume = choicedDisk.VOLUMES.at(0);
		cout << "\nChoised disk " << choicedDisk.DiskNumber <<endl;
		wcout << L"Locked partition: " <<  volume.MountPoint << endl;
		partitions.push_back(volume.MountPoint);
	}

	AddOperation(consoleOperation, partitions);	
	return result;
}

bool SleepConsole::PreparingLockCommand()
{
	bool result = false;
	int diskCounts = 0;
	cout << "\nDisk list for LOCK command: " << endl;
	for(vector<DISKINFORMATION>::const_iterator iter = disks.begin(); iter != disks.end(); ++iter, diskCounts++){
		int diskNum = iter->DiskNumber;
		int operation = BusyDisks->CurrentOperation(diskNum);
		if (operation == 0xFF)
			operation = NO_OPERATION;
		//Operations operation = DiskTask(diskNum);
		//if(operation == LOCK)
		if(operation == consoleOperation)
			continue;

		wstring diskName(L"Disk ");
		diskNum = iter->DiskNumber;
		wchar_t buff[2] = {0};
		_itow(diskNum, buff, 10);
		diskName += buff;
		int DiskSizeInGB = iter->size / 1073741824;
		wcout << L"[" << buff << L"] " << diskName << L"(" << iter->Model << L" size: " << DiskSizeInGB << L" GB" << L")" << endl;		
	}
	cout << "[" << diskCounts << "] " << "ALL" <<endl;
	int abortCommand = diskCounts + 1;
	cout << "[" << abortCommand << "] " << "Abort" << endl;

	int choice = 0;
	while(diskCounts){
		cout << "\nChoice disk: ";
		cin >> choice;
		if(choice >= 0 && choice <= diskCounts)
			break;
		else if(choice == abortCommand){
			cout << "\nCancel operation." << endl;
			return result;
		}
		else
			cout <<"\nDisk not found!" << endl;
	}
	vector<wstring> partitions;
	WCHAR winDir[MAX_PATH] = {0};
	GetWindowsDirectory(winDir, MAX_PATH);
	WCHAR systemDisk[4];
	systemDisk[0] = winDir[0];
	systemDisk[1] = winDir[1];
	systemDisk[2] = winDir[2];
	systemDisk[3] = L'\0';

	if(choice == diskCounts){
		for(vector<DISKINFORMATION>::const_iterator iter = disks.begin(); iter != disks.end(); ++iter){
			for(std::vector<VOLUMEPROPERTIES>::const_iterator itVol = iter->VOLUMES.begin(); itVol != iter->VOLUMES.end(); ++itVol){
				if(itVol->MountPoint.find(systemDisk) != wstring::npos)
					continue;
				map<std::wstring, Operations>::iterator BusyPartition = CurrentOperations->find(itVol->MountPoint);
				if(BusyPartition == CurrentOperations->end() || (BusyPartition != CurrentOperations->end() && BusyPartition->second != consoleOperation)) //LOCK
					partitions.push_back(itVol->MountPoint);
			}
		}
	}
	else{
		DISKINFORMATION choicedDisk = disks.at(choice);
		cout << "\nChoised disk " << choicedDisk.DiskNumber <<endl;
		cout << "Partition list:" << endl;
		int partitionCount = 0;
		for(std::vector<VOLUMEPROPERTIES>::const_iterator itVol = choicedDisk.VOLUMES.begin(); itVol != choicedDisk.VOLUMES.end(); ++itVol, partitionCount++){
			if(itVol->MountPoint.find(systemDisk) != wstring::npos)
				continue;
			map<std::wstring, Operations>::iterator BusyPartition = CurrentOperations->find(itVol->MountPoint);
			if(BusyPartition == CurrentOperations->end() || (BusyPartition != CurrentOperations->end() && BusyPartition->second != consoleOperation)) //LOCK
				wcout << L"[" << partitionCount << L"] " << itVol->MountPoint << endl;
		}
		cout << "[" << partitionCount << "] " << "ALL" <<endl;
		abortCommand = partitionCount + 1;
		cout << "[" << abortCommand << "] " << "Abort" << endl;

		while(partitionCount){
		cout << "\nChoice partition: ";
		cin >> choice;
		if(choice >= 0 && choice <= partitionCount)
			break;
		else if(choice == abortCommand){
			cout << "\nCancel operation." << endl;
			return result;
		}
		else
			cout <<"\nPartition not found!";
		}
		
		result = true;

		if(choice == partitionCount)
		{
			for(std::vector<VOLUMEPROPERTIES>::const_iterator itVol = choicedDisk.VOLUMES.begin(); itVol != choicedDisk.VOLUMES.end(); ++itVol, partitionCount++){
				map<std::wstring, Operations>::iterator BusyPartition = CurrentOperations->find(itVol->MountPoint);
				if(BusyPartition == CurrentOperations->end() || (BusyPartition != CurrentOperations->end() && BusyPartition->second != consoleOperation)) //LOCK
					partitions.push_back(itVol->MountPoint);
			}
		}
		else{
			VOLUMEPROPERTIES volume = choicedDisk.VOLUMES.at(choice);
			partitions.push_back(volume.MountPoint);
		}
	}
	//AddOperation(LOCK, partitions);
	AddOperation(consoleOperation, partitions);	
	return result;
}

bool SleepConsole::PreparingCommand()
{
	bool result = false;
	switch(consoleOperation){
	case ACTIVE:
		result = PreparingActiveCommand();
		break;
	case LOCK:
		result = PreparingLockCommand();
		break;
	}
	return result;
}

void SleepConsole::ClearListOfPendingOperations()
{
	while(!newOperations->empty()){
		newOperations->pop();
	}
	if(!sleephddQueue->empty())
		sleephddQueue->clear();
}

bool SleepConsole::AppyPendingOperations()
{
	//std::vector<std::pair<std::wstring, Operations> > listTasks;
	if(newOperations->empty())
		return false;
	while(!newOperations->empty()){
		//Operations op;
		std::pair<std::wstring, Operations> task = newOperations->top();
		//if(sleephddQueue
		//listTasks.push_back
		newOperations->pop();
		consoleOperation = task.second;
		if(GenerateCommandToSend(task.first))
			ApplyCommand();
	}
	return true;
}



bool SleepConsole::FindListSubCommand(std::wstring& input)
{
	bool result = false;
	wstring diskListCommand(L"disks");
	wstring currentOperations(L"current operations");
	wstring pendingOperations(L"pending operations");
	if(input.find(diskListCommand) != wstring::npos){
		result = true;
		for(vector<DISKINFORMATION>::const_iterator iter = disks.begin(); iter != disks.end(); ++iter){
			wstring diskName(L"Disk ");
			int diskNum = iter->DiskNumber;
			wchar_t buff[2] = {0};
			_itow(diskNum, buff, 10);
			diskName += buff;
			wcout << diskName << endl;
			for(std::vector<VOLUMEPROPERTIES>::const_iterator itVol = iter->VOLUMES.begin(); itVol != iter->VOLUMES.end(); ++itVol){
				wcout << L"\t" << itVol->MountPoint << endl;
			}
		}
	}
	else if(input.find(currentOperations) != wstring::npos){
		consoleOperation = QUERY;
		if (GenerateCommandToSend()){
			if (ApplyCommand())
					result = true;
		}
	}
	else if(input.find(pendingOperations) != wstring::npos){
		wchar_t* buffer = new wchar_t[80];
		ZeroMemory(buffer, 80);
		result = true;
		if (sleephddQueue->empty()){
			cout << "No pending operations!\n";
			delete buffer;
			buffer = NULL;
			return result;
		}
		for(std::list<std::pair<std::wstring, Operations> >::iterator iter = sleephddQueue->begin(); iter != sleephddQueue->end(); iter++){
			if(iter->second == ACTIVE)
				swprintf(buffer,L"Partition %s - Active\n", iter->first.c_str());
			else
				swprintf(buffer,L"Partition %s - Locked\n", iter->first.c_str());
			wcout << buffer;
			ZeroMemory(buffer, 80);
		}
		delete buffer;
		buffer = NULL;
	}
	else{
		cout << "Not supported command!\n";
	}

	return result;
}