#include "..\include\QueryDisks.h"
#include <Windows.h>

void ObtainDiskInformation(
	__in std::vector<VOLUMEPROPERTIES> &volumes,
	__in std::vector<DISKPROPERTIES> &disks,
	__out std::vector<DISKINFORMATION> &result
	)
{
	DISKINFORMATION dskInfo;
	bool DiskFound = false;
	for(std::vector<VOLUMEPROPERTIES>::iterator iterVol = volumes.begin(); iterVol != volumes.end(); iterVol++)
	{
		for(std::vector<DISKPROPERTIES>::iterator iterDisk = disks.begin(); iterDisk != disks.end(); iterDisk++)
		{
			if(iterVol->DiskNumber == iterDisk->Index)
			{
				if(!DiskFound)
				{
					dskInfo.DiskNumber = iterVol->DiskNumber;
					std::wstring tmpStr(iterDisk->Model);
					dskInfo.Model = new wchar_t[tmpStr.length() + 1];
					lstrcpy(dskInfo.Model, tmpStr.c_str());
					dskInfo.size = iterDisk->size;
					DiskFound = true;
				}
				dskInfo.VOLUMES.push_back(*iterVol);

			}
		}
		if (DiskFound){
			result.push_back(dskInfo);
			dskInfo.VOLUMES.clear();
		}
		DiskFound = false;	
	}
}