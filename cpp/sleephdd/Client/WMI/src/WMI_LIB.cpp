#define _WIN32_DCOM
#include <stdio.h>
#include "..\include\WMI_LIB.h"
#include <comdef.h>
#include <Wbemidl.h>
#include <vector>
#include <string>
#include <cstdlib>
#pragma comment(lib, "wbemuuid.lib")


using namespace std;

int GetWMI(wchar_t* wmiClass, std::vector<DISKPROPERTIES> &result)
{
	HRESULT hres;
	int strLen = 0;
    // Step 1: --------------------------------------------------
    // Initialize COM. ------------------------------------------
	bool isCOMInitialized = true;

    hres =  CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
		if(hres == RPC_E_CHANGED_MODE){
			printf("Failed to initialize COM library. Error code = 0x%X\n",hres);
			isCOMInitialized = false;
		}
    }

    // Step 2: --------------------------------------------------
    // Set general COM security levels --------------------------
    // Note: If you are using Windows 2000, you need to specify -
    // the default authentication credentials for a user by using
    // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
    // parameter of CoInitializeSecurity ------------------------

	if(!isCOMInitialized){
		hres =  CoInitializeSecurity(
			NULL,
			-1,                          // COM authentication
			NULL,                        // Authentication services
			NULL,                        // Reserved
			RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
			RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
			NULL,                        // Authentication info
			EOAC_NONE,                   // Additional capabilities
			NULL                         // Reserved
			);

                     
		if (FAILED(hres))
		{
			printf("Failed to initialize COM library. Error code = 0x%X\n",hres);
			CoUninitialize();
			return 1;                    // Program has failed.
		}
	}
   
    // Step 3: ---------------------------------------------------
    // Obtain the initial locator to WMI -------------------------

    IWbemLocator *pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,            
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID *) &pLoc);
 
    if (FAILED(hres))
    {
        printf("Failed to initialize COM library. Error code = 0x%X\n",hres);
        CoUninitialize();
        return 1;                 // Program has failed.
    }

    // Step 4: -----------------------------------------------------
    // Connect to WMI through the IWbemLocator::ConnectServer method

    IWbemServices *pSvc = NULL;
   
    // Connect to the root\cimv2 namespace with
    // the current user and obtain pointer pSvc
    // to make IWbemServices calls.
    hres = pLoc->ConnectServer(
         _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
         NULL,                    // User name. NULL = current user
         NULL,                    // User password. NULL = current
         0,                       // Locale. NULL indicates current
         NULL,                    // Security flags.
         0,                       // Authority (e.g. Kerberos)
         0,                       // Context object
         &pSvc                    // pointer to IWbemServices proxy
         );
   
    if (FAILED(hres))
    {
      printf("Failed to initialize COM library. Error code = 0x%X\n", hres);
        pLoc->Release();    
        CoUninitialize();
        return 1;                // Program has failed.
    }

    //cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


    // Step 5: --------------------------------------------------
    // Set security levels on the proxy -------------------------

    hres = CoSetProxyBlanket(
       pSvc,                        // Indicates the proxy to set
       RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
       RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
       NULL,                        // Server principal name
       RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
       RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
       NULL,                        // client identity
       EOAC_NONE                    // proxy capabilities
    );

    if (FAILED(hres))
    {
        printf("Failed to initialize COM library. Error code = 0x%X\n",hres);
        pSvc->Release();
        pLoc->Release();    
        CoUninitialize();
        return 1;               // Program has failed.
    }

    // Step 6: --------------------------------------------------
    // Use the IWbemServices pointer to make requests of WMI ----

    // For example, get the name of the operating system
    IEnumWbemClassObject* pEnumerator = NULL;
	wchar_t* TextQuery = (wchar_t*) new wchar_t [MAX_UNICODE_SYMBOLS * sizeof(WCHAR)];
	swprintf(TextQuery, MAX_UNICODE_SYMBOLS, L"SELECT * FROM %s",  wmiClass);
	hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(TextQuery),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator); 
	delete TextQuery;
   
    if (FAILED(hres))
    {
       printf("Failed - Error code = 0x%X\n",hres);
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;               // Program has failed.
    }

    // Step 7: -------------------------------------------------
    // Get the data from the query in step 6 -------------------
 
    IWbemClassObject *pclsObj;
    ULONG uReturn = 0;
    DISKPROPERTIES dskProp;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if(0 == uReturn)
        {
            break;
        }
		
		VARIANT vtProp;
		hr = pclsObj->Get(L"MediaType", 0, &vtProp, 0, 0);
		if(lstrcmp(L"Fixed hard disk media", vtProp.bstrVal) != 0)
			continue;
		hr = pclsObj->Get(L"Index", 0, &vtProp, 0, 0);
		dskProp.Index = vtProp.uintVal;       
		VariantClear(&vtProp);

		hr = pclsObj->Get(L"Model", 0, &vtProp, 0, 0);
		wstring modelStr(vtProp.bstrVal);
		dskProp.Model = new wchar_t[modelStr.length() + 1];
		lstrcpy(dskProp.Model, modelStr.c_str());
		VariantClear(&vtProp);

		hr = pclsObj->Get(L"Size", 0, &vtProp, 0, 0);
		dskProp.size = _wtoi64(vtProp.bstrVal);
		VariantClear(&vtProp);

		result.push_back(dskProp);
        pclsObj->Release();
    }

    // Cleanup
    // ========
   
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return 0;   // Program successfully completed.

}