#ifndef SLEEP_HDD_SLEEP_SERVICE_H
#define SLEEP_HDD_SLEEP_SERVICE_H
#include <Windows.h>

enum HDDSERVICE_STATUS{
	HDDSERVICE_RUNNING,
	HDDSERVICE_STOPPING
};

class SleepHDDService{
public:
	SleepHDDService();
	~SleepHDDService();
	void SetStatus(HDDSERVICE_STATUS newStatus);
	HDDSERVICE_STATUS GetStatus();
private:
	CRITICAL_SECTION* CriticalSectionService;
	HDDSERVICE_STATUS status;
};

#endif //SLEEP_HDD_SLEEP_SERVICE_H