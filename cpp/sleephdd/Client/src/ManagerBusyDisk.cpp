#include "ManagerBusyDisk.h"

ManagerBusyDisk::ManagerBusyDisk()
{
	//pair<Disk, Task>
	BusyDisks = new std::vector<std::pair<int, int> >(); 
}

ManagerBusyDisk::~ManagerBusyDisk()
{
	delete BusyDisks;
}

void ManagerBusyDisk::ClearBusyList()
{
	if(!BusyDisks->empty())
		BusyDisks->clear();
}

bool ManagerBusyDisk::AddToBusyList(int task, int DiskNumber)
{
	bool isAlreadyAdded = false;
	for(std::vector<std::pair<int, int> >::const_iterator SearchedDisk = BusyDisks->begin(); SearchedDisk != BusyDisks->end(); SearchedDisk++){
		if(SearchedDisk->first == DiskNumber){
			isAlreadyAdded = true;
			break;
		}
	}
	if (!isAlreadyAdded)
		BusyDisks->push_back(std::make_pair(DiskNumber, task));

	return isAlreadyAdded;
}

bool ManagerBusyDisk::DiskIsBusy(int DiskNumber)
{
	bool isBusy = false;
	for(std::vector<std::pair<int, int> >::const_iterator SearchedDisk = BusyDisks->begin(); SearchedDisk != BusyDisks->end(); SearchedDisk++){
		if(SearchedDisk->first == DiskNumber){
			isBusy = true;
			break;
		}
	}
	return isBusy;
}

int ManagerBusyDisk::CurrentOperation(int DiskNumber)
{
	int task = 0xFF;
	for(std::vector<std::pair<int, int> >::const_iterator SearchedDisk = BusyDisks->begin(); SearchedDisk != BusyDisks->end(); SearchedDisk++){
		if(SearchedDisk->first == DiskNumber)
			task = SearchedDisk->second;
	}
	return task;
}