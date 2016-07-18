#include "PerformanceQuery.h"
#include "Misc.h"
#include <iostream>
#pragma comment(lib, "wbemuuid.lib")

#include <Windows.h>
#include <psapi.h>
#include <memory.h>
#include <stdio.h>

int
mema() {

    PROCESS_MEMORY_COUNTERS pmc = {};
	while(true){
		GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
		printf("WS: %d\tPeak WS: %d\tCommit: %d\tPeak commit: %d\n", pmc.WorkingSetSize / 1024, pmc.PeakWorkingSetSize / 1024, pmc.PagefileUsage / 1024, pmc.PeakPagefileUsage / 1024);

		auto mem = malloc(1024 * 1024 * 1024);
		printf("malloc(1Gb) - %s\n", (mem != nullptr) ? "check" : "uncheck");

		ZeroMemory(&pmc, sizeof(pmc));
		GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
		printf("WS: %d\tPeak WS: %d\tCommit: %d\tPeak commit: %d\n", pmc.WorkingSetSize / 1024, pmc.PeakWorkingSetSize / 1024, pmc.PagefileUsage / 1024, pmc.PeakPagefileUsage / 1024);

		if (mem != nullptr) {
			free(mem);
			mem = nullptr;
			printf("Fre-e-e-e-e-edom\n");

			ZeroMemory(&pmc, sizeof(pmc));
			GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
			printf("WS: %d\tPeak WS: %d\tCommit: %d\tPeak commit: %d\n", pmc.WorkingSetSize / 1024, pmc.PeakWorkingSetSize / 1024, pmc.PagefileUsage / 1024, pmc.PeakPagefileUsage / 1024);
		}
	}
	return 0;
}

int queryPerf()
{
	   HRESULT                 hr = S_OK;
    IWbemRefresher          *pRefresher = NULL;
    IWbemConfigureRefresher *pConfig = NULL;
    IWbemHiPerfEnum         *pEnum = NULL;
    IWbemServices           *pNameSpace = NULL;
    IWbemLocator            *pWbemLocator = NULL;
    IWbemObjectAccess       **apEnumAccess = NULL;
    BSTR                    bstrNameSpace = NULL;
    long                    lID = 0;
    long                    lVirtualBytesHandle = 0;
    long                    lIDProcessHandle = 0;
	//---------------------
	long NameHandle = 0;
	long DescriptionHandle = 0;
	long PercentTimeHandle = 0;
	long PercentReadTimeHandle = 0;
	long PercentWriteTimeHandle = 0;
	long PercentIdleTimeHandle = 0;

	DWORD dwPercentTime = 0;
	DWORD dwPercentReadTime = 0;
	DWORD dwPercentWriteTime = 0;
	DWORD dwPercentIdleTime = 0;
	unsigned char* NameValue = 0;
	unsigned char* DescValue = 0;
	//*********************
    //DWORD                   dwVirtualBytes = 0;
    //DWORD                   dwProcessId = 0;
    DWORD                   dwNumObjects = 0;
    DWORD                   dwNumReturned = 0;
    //DWORD                   dwIDProcess = 0;
    DWORD                   i=0;
    int                     x=0;
	    if (FAILED (hr = CoInitializeEx(NULL,COINIT_MULTITHREADED)))
    {
        goto CLEANUP;
    }

    if (FAILED (hr = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_NONE,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, 0)))
    {
        goto CLEANUP;
    }

    if (FAILED (hr = CoCreateInstance(
        CLSID_WbemLocator, 
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator,
        (void**) &pWbemLocator)))
    {
        goto CLEANUP;
    }

    // Connect to the desired namespace.
    bstrNameSpace = SysAllocString(L"\\\\.\\root\\cimv2");
    if (NULL == bstrNameSpace)
    {
        hr = E_OUTOFMEMORY;
        goto CLEANUP;
    }
    if (FAILED (hr = pWbemLocator->ConnectServer(
        bstrNameSpace,
        NULL, // User name
        NULL, // Password
        NULL, // Locale
        0L,   // Security flags
        NULL, // Authority
        NULL, // Wbem context
        &pNameSpace)))
    {
        goto CLEANUP;
    }
    pWbemLocator->Release();
    pWbemLocator=NULL;
    SysFreeString(bstrNameSpace);
    bstrNameSpace = NULL;

    if (FAILED (hr = CoCreateInstance(
        CLSID_WbemRefresher,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IWbemRefresher, 
        (void**) &pRefresher)))
    {
        goto CLEANUP;
    }

    if (FAILED (hr = pRefresher->QueryInterface(
        IID_IWbemConfigureRefresher,
        (void **)&pConfig)))
    {
        goto CLEANUP;
    }

    // Add an enumerator to the refresher.
    if (FAILED (hr = pConfig->AddEnum(
        pNameSpace, 
        //L"Win32_PerfRawData_PerfProc_Process", 
		L"Win32_PerfFormattedData_PerfDisk_LogicalDisk",
        0, 
        NULL, 
        &pEnum, 
        &lID)))
    {
        goto CLEANUP;
    }
    pConfig->Release();
    pConfig = NULL;

    // Get a property handle for the VirtualBytes property.

    // Refresh the object ten times and retrieve the value.
    for(x = 0; x < 10; x++)
    {
        dwNumReturned = 0;
        dwNumObjects = 0;

        if (FAILED (hr =pRefresher->Refresh(0L)))
        {
            goto CLEANUP;
        }

        hr = pEnum->GetObjects(0L, 
            dwNumObjects, 
            apEnumAccess, 
            &dwNumReturned);
        // If the buffer was not big enough,
        // allocate a bigger buffer and retry.
        if (hr == WBEM_E_BUFFER_TOO_SMALL 
            && dwNumReturned > dwNumObjects)
        {
            apEnumAccess = new IWbemObjectAccess*[dwNumReturned];
            if (NULL == apEnumAccess)
            {
                hr = E_OUTOFMEMORY;
                goto CLEANUP;
            }
            SecureZeroMemory(apEnumAccess,
                dwNumReturned*sizeof(IWbemObjectAccess*));
            dwNumObjects = dwNumReturned;

            if (FAILED (hr = pEnum->GetObjects(0L, 
                dwNumObjects, 
                apEnumAccess, 
                &dwNumReturned)))
            {
                goto CLEANUP;
            }
        }
        else
        {
            if (hr == WBEM_S_NO_ERROR)
            {
                hr = WBEM_E_NOT_FOUND;
                goto CLEANUP;
            }
        }

        // First time through, get the handles.
        if (0 == x)
        {
            //CIMTYPE VirtualBytesType;
			CIMTYPE ValueType;

			//CIM_STRING 

            if (FAILED (hr = apEnumAccess[0]->GetPropertyHandle(
                L"Name",
                &ValueType,
                &NameHandle)))
            {
                goto CLEANUP;
            }

			if (FAILED (hr = apEnumAccess[0]->GetPropertyHandle(
                L"Description",
                &ValueType,
                &DescriptionHandle)))
            {
                goto CLEANUP;
            }

            if (FAILED (hr = apEnumAccess[0]->GetPropertyHandle(
                L"PercentDiskTime",
                &ValueType,
                &PercentTimeHandle)))
            {
                goto CLEANUP;
            }

			if (FAILED (hr = apEnumAccess[0]->GetPropertyHandle(
                L"PercentDiskReadTime",
                &ValueType,
                &PercentReadTimeHandle)))
            {
                goto CLEANUP;
            }

			if (FAILED (hr = apEnumAccess[0]->GetPropertyHandle(
                L"PercentDiskWriteTime",
                &ValueType,
                &PercentWriteTimeHandle)))
            {
                goto CLEANUP;
            }

			if (FAILED (hr = apEnumAccess[0]->GetPropertyHandle(
                L"PercentIdleTime",
                &ValueType,
                &PercentIdleTimeHandle)))
            {
                goto CLEANUP;
            }

        }
           	NameValue = new unsigned char[256];
			DescValue = new unsigned char[256];
			ZeroMemory(NameValue, 256);
			ZeroMemory(DescValue, 256);
        for (i = 0; i < dwNumReturned; i++)
        {
            /*if (FAILED (hr = apEnumAccess[i]->ReadDWORD(
                lVirtualBytesHandle,
                &dwVirtualBytes)))
            */
			long readLenght;
			if (FAILED (hr = apEnumAccess[i]->ReadPropertyValue(NameHandle, 256,  &readLenght, NameValue)))
			{
                goto CLEANUP;
            }

			if (FAILED (hr = apEnumAccess[i]->ReadPropertyValue(DescriptionHandle, 256,  &readLenght, DescValue)))
			{
                goto CLEANUP;
            }

            if (FAILED (hr = apEnumAccess[i]->ReadDWORD(
                PercentTimeHandle,
                &dwPercentTime)))
            {
                goto CLEANUP;
            }

			if (FAILED (hr = apEnumAccess[i]->ReadDWORD(
                PercentReadTimeHandle,
                &dwPercentReadTime)))
            {
                goto CLEANUP;
            }

			 if (FAILED (hr = apEnumAccess[i]->ReadDWORD(
                PercentWriteTimeHandle,
                &dwPercentWriteTime)))
            {
                goto CLEANUP;
            }

			if (FAILED (hr = apEnumAccess[i]->ReadDWORD(
                PercentIdleTimeHandle,
                &dwPercentIdleTime)))
            {
                goto CLEANUP;
            }

            //wprintf(L"Process ID %lu is using %lu bytes\n",
             //   dwIDProcess, dwVirtualBytes);

			wchar_t* buffer = (wchar_t*)NameValue;
			wprintf(L"\nDisk %s\n", buffer);
			buffer = (wchar_t*)DescValue;
			wprintf(L"Decription %s\n", buffer);
			std::cout << "Total Disk time: " << dwPercentTime << std::endl;
			std::cout << "Read Disk time: " << dwPercentReadTime << std::endl;
			std::cout << "Write Disk time: " << dwPercentWriteTime << std::endl;
			std::cout << "Idle Disk time: " << dwPercentIdleTime << std::endl;

            // Done with the object
            apEnumAccess[i]->Release();
            apEnumAccess[i] = NULL;
			ZeroMemory(NameValue, 256);
			ZeroMemory(DescValue, 256);
        }
			ZeroMemory(NameValue, 256);
			ZeroMemory(DescValue, 256);

        if (NULL != apEnumAccess)
        {
            delete [] apEnumAccess;
            apEnumAccess = NULL;
        }

       // Sleep for a second.
       Sleep(1000);
    }
    // exit loop here
    CLEANUP:

    if (NULL != bstrNameSpace)
    {
        SysFreeString(bstrNameSpace);
    }

    if (NULL != apEnumAccess)
    {
        for (i = 0; i < dwNumReturned; i++)
        {
            if (apEnumAccess[i] != NULL)
            {
                apEnumAccess[i]->Release();
                apEnumAccess[i] = NULL;
            }
        }
        delete [] apEnumAccess;
    }
    if (NULL != pWbemLocator)
    {
        pWbemLocator->Release();
    }
    if (NULL != pNameSpace)
    {
        pNameSpace->Release();
    }
    if (NULL != pEnum)
    {
        pEnum->Release();
    }
    if (NULL != pConfig)
    {
        pConfig->Release();
    }
    if (NULL != pRefresher)
    {
        pRefresher->Release();
    }

    CoUninitialize();

    if (FAILED (hr))
    {
        wprintf (L"Error status=%08x\n",hr);
    }

    return 1;
}