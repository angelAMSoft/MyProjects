#ifndef QUERY_DISKS_H
#define QUERY_DISKS_H
#include <vector>

typedef struct DiskProperties{
  unsigned int   Index;
  wchar_t*   Model;
  unsigned __int64   size;
} DISKPROPERTIES, *PDISKPROPERTIES;

typedef struct VolumeProperties{
  unsigned int   DiskNumber;
  std::wstring   MountPoint;
  std::wstring   GlobalName;
  unsigned __int64   size;
} VOLUMEPROPERTIES, *PVOLUMEPROPERTIES;

typedef struct DiskInformation{
  unsigned int   DiskNumber;
  wchar_t*   Model;
  unsigned __int64   size;
  std::vector<VolumeProperties> VOLUMES;
} DISKINFORMATION, *PDISKINFORMATION;

void ObtainDiskInformation(
	__in std::vector<VOLUMEPROPERTIES> &volumes,
	__in std::vector<DISKPROPERTIES> &disks,
	__out std::vector<DISKINFORMATION> &result);

void collectDiskInformation(std::vector<DISKINFORMATION> &result);

#endif //QUERY_DISKS_H
