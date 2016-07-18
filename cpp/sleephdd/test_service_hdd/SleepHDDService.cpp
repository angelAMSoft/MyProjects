#include "SleepHDDService.h"

SleepHDDService::SleepHDDService()
{
	CriticalSectionService = new CRITICAL_SECTION;
	InitializeCriticalSection(CriticalSectionService);
	status = HDDSERVICE_RUNNING;
}

SleepHDDService::~SleepHDDService()
{
	DeleteCriticalSection(CriticalSectionService);
	delete CriticalSectionService;
}

HDDSERVICE_STATUS SleepHDDService::GetStatus()
{
	HDDSERVICE_STATUS curStatus;
	EnterCriticalSection(CriticalSectionService);
	curStatus = status;
	LeaveCriticalSection(CriticalSectionService);
	return curStatus;
}

void SleepHDDService::SetStatus(HDDSERVICE_STATUS newStatus)
{
	EnterCriticalSection(CriticalSectionService);
	status = newStatus;
	LeaveCriticalSection(CriticalSectionService);
}