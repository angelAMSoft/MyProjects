#include <windows.h>
#include <stdio.h>
#include "..\include\FindVolume.h"


using namespace std;
ofstream logService;

void DisplayVolumePaths(
        __in PWCHAR VolumeName,
		__out PWCHAR Letter
        )
{
    DWORD  CharCount = MAX_PATH + 1;
    PWCHAR Names     = NULL;
    PWCHAR NameIdx   = NULL;
    BOOL   Success   = FALSE;

    for (;;) 
    {
        //
        //  Allocate a buffer to hold the paths.
        Names = (PWCHAR) new BYTE [CharCount * sizeof(WCHAR)];

        if ( !Names ) 
        {
            //
            //  If memory can't be allocated, return.
            return;
        }

        //
        //  Obtain all of the paths
        //  for this volume.
        Success = GetVolumePathNamesForVolumeNameW(
            VolumeName, Names, CharCount, &CharCount
            );

        if ( Success ) 
        {
            break;
        }

        if ( GetLastError() != ERROR_MORE_DATA ) 
        {
            break;
        }

        //
        //  Try again with the
        //  new suggested size.
        delete [] Names;
        Names = NULL;
    }

    if ( Success )
    {
		lstrcpy(Letter, Names);
    }

    if ( Names != NULL ) 
    {
        delete [] Names;
        Names = NULL;
    }

    return;
}
void CreateVolumePath(WCHAR* path)
{
	wstring oldPath(path);
	wstring newPath(L"\\\\.\\");
	int lenght = oldPath.size() - 1;
	for(string::size_type index = 0; index != lenght; index++)
		newPath += oldPath[index];
	lstrcpy(path, newPath.c_str());
}

void FindVolumeFromDisks(std::vector<VOLUMEPROPERTIES> &result)
{
	DWORD  CharCount            = 0;
    WCHAR  DeviceName[MAX_PATH] = L"";
    DWORD  Error                = ERROR_SUCCESS;
    HANDLE FindHandle           = INVALID_HANDLE_VALUE;
    BOOL   Found                = FALSE;
    size_t Index                = 0;
    BOOL   Success              = FALSE;
    WCHAR  VolumeName[MAX_PATH] = L"";

	FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

    if (FindHandle == INVALID_HANDLE_VALUE)
    {
        Error = GetLastError();
		logService << L"FindFirstVolumeW failed with error code "<< Error <<endl << flush;
        return;
    }

	VOLUMEPROPERTIES vol;
	wstring tmpStr;
	int resultDiskNumber = 0;

	for (;;)
    {
        //
        //  Skip the \\?\ prefix and remove the trailing backslash.
        Index = wcslen(VolumeName) - 1;

        if (VolumeName[0]     != L'\\' ||
            VolumeName[1]     != L'\\' ||
            VolumeName[2]     != L'?'  ||
            VolumeName[3]     != L'\\' ||
            VolumeName[Index] != L'\\') 
        {
            Error = ERROR_BAD_PATHNAME;
			logService << L"FindFirstVolumeW/FindNextVolumeW returned a bad path:  "<< VolumeName <<endl<< flush;
            break;
        }

	    VolumeName[Index] = L'\0';

        CharCount = QueryDosDeviceW(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName)); 

        VolumeName[Index] = L'\\';

		if ( CharCount == 0 ) 
        {
            Error = GetLastError();
			logService << L"QueryDosDeviceW failed with error code "<< Error <<endl<< flush;
            break;
        }
	    WCHAR letter[8] = {0};
        DisplayVolumePaths(VolumeName, letter);
		tmpStr = letter;
		vol.MountPoint = tmpStr;

		CreateVolumePath(letter);
		tmpStr = letter;
		vol.GlobalName = tmpStr;
		resultDiskNumber = FindDiskNumber(tmpStr.c_str());
		vol.DiskNumber = resultDiskNumber;
		vol.size = CalculateVolumeSize(tmpStr.c_str());
		result.push_back(vol);
		Success = FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName));

        if ( !Success ) 
        {
            Error = GetLastError();

            if (Error != ERROR_NO_MORE_FILES) 
            {
				logService << L"FindNextVolumeW failed with error cod "<< Error <<endl<< flush;
                break;
            }

            Error = ERROR_SUCCESS;
            break;
        }
    }

    FindVolumeClose(FindHandle);
    FindHandle = INVALID_HANDLE_VALUE;

}

unsigned int FindDiskNumber(const wchar_t* dev)
{
	int result = 16;
	HANDLE hDevice = CreateFile(dev,GENERIC_READ|GENERIC_WRITE,
								FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,0,0); 

	DWORD err = GetLastError();
	if (hDevice != INVALID_HANDLE_VALUE){
		VOLUME_DISK_EXTENTS diskExtents;
		DWORD dwSize;

		BOOL iRes = DeviceIoControl(
					hDevice,
					IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
					NULL,
					0,
					(LPVOID) &diskExtents,
					(DWORD) sizeof(diskExtents),
					(LPDWORD) &dwSize,
					NULL);
		result = diskExtents.Extents[0].DiskNumber;
	}
    return result;
}

unsigned __int64 CalculateVolumeSize(const wchar_t* dev)
{
	unsigned __int64  result = 0;
	HANDLE hDevice = CreateFile(dev,GENERIC_READ|GENERIC_WRITE,
								FILE_SHARE_READ|FILE_SHARE_WRITE,0,OPEN_EXISTING,0,0); 
	if (hDevice != INVALID_HANDLE_VALUE){
		PARTITION_INFORMATION_EX partitionExtents;
		DWORD dwSize;
		BOOL iRes = DeviceIoControl(
					hDevice,
					IOCTL_DISK_GET_PARTITION_INFO_EX,
					NULL,
					0,
					(LPVOID) &partitionExtents,
					(DWORD) sizeof(partitionExtents),
					(LPDWORD) &dwSize,
					NULL);
		result = partitionExtents.PartitionLength.QuadPart;
	}
    return result;
}
