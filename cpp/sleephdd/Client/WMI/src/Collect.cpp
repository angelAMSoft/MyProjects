#include "..\include\WMI_LIB.h"
#include "..\include\FindVolume.h"
extern std::ofstream logService;
char servicePahtToLog[] = "C:\\Windows\\logs\\sleep_hdd.log";

void collectDiskInformation(std::vector<DISKINFORMATION> &result)
{
	logService.open(servicePahtToLog, std::ofstream::out | std::ofstream::app);
	std::vector<VOLUMEPROPERTIES> volumes;
	FindVolumeFromDisks(volumes);
	logService.close();
	std::vector<DISKPROPERTIES>  disks;
	GetWMI(L"Win32_DiskDrive", disks);
	ObtainDiskInformation(volumes, disks, result);
}