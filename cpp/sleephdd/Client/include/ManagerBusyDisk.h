#ifndef MANAGER_BUSY_DISK_H
#define MANAGER_BUSY_DISK_H
#include <utility>
#include <vector>

class ManagerBusyDisk{
public:
	ManagerBusyDisk();
	~ManagerBusyDisk();
	bool DiskIsBusy(int DiskNumber);
	bool AddToBusyList(int task, int DiskNumber);
	void ClearBusyList();
	int CurrentOperation(int DiskNumber);
private:
	//pair<Disk, Task>
	std::vector<std::pair<int, int> > *BusyDisks;
};
#endif // MANAGER_BUSY_DISK_H