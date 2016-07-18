#include "QueryDisks.h"
#include <string>
#include <fstream>

void DisplayVolumePaths(
        __in wchar_t* VolumeName,
		__out wchar_t* Letter
        );
void CreateVolumePath(wchar_t* path);

void FindVolumeFromDisks(std::vector<VOLUMEPROPERTIES> &result);

unsigned int FindDiskNumber(const wchar_t* dev);
unsigned __int64 CalculateVolumeSize(const wchar_t* dev);