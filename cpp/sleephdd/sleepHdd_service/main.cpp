#include "main.h"
#include "service.h"


std::ofstream logService;
using namespace std;

int main(int argc, char **argv)
{
	if (argc == 2)
	{
		if (lstrcmpA(argv[1], "/i") == 0)
		{
			SC_HANDLE hServiceIOControlManager, hService;
			hServiceIOControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
			hService = OpenService(hServiceIOControlManager, serviceName, GENERIC_READ);
			if (hService == NULL){
				HMODULE hmodule = GetModuleHandle(NULL);
				WCHAR psz_file_name[MAX_PATH + 1] = L"";
				GetModuleFileName(hmodule,psz_file_name,MAX_PATH + 1);
				hService = CreateService(hServiceIOControlManager, serviceName, L"Sleep HDD service",
										 SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, 
										 SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
										 psz_file_name, NULL, NULL, NULL, NULL, NULL);
				if (hService != NULL)
				{	
					MessageBox(NULL, L"Service succesfully installed!", L"Sleep HDD service", MB_OK | MB_ICONINFORMATION);		
					StartService(hService, 0, NULL);
					CloseServiceHandle(hService);
				}
				CloseServiceHandle(hServiceIOControlManager);
				return 1;
			}
			else
			{
				MessageBox(NULL, L"Service already installed!", L"Sleep HDD service", MB_OK | MB_ICONINFORMATION);
				CloseServiceHandle(hService);
				CloseServiceHandle(hServiceIOControlManager);
				return 0;
			}
		}
		else if (lstrcmpA(argv[1], "/u") == 0)
		{
			SC_HANDLE hServiceIOControlManager, hService;
			hServiceIOControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
			hService = OpenService(hServiceIOControlManager, serviceName, SERVICE_ALL_ACCESS | DELETE);
			if (hService == NULL){
				MessageBox(NULL, L"Service not installed!", L"Sleep HDD service", MB_OK | MB_ICONERROR);
				CloseServiceHandle(hServiceIOControlManager);
				return 2;
			}
			if (DeleteService(hService))
				MessageBox(NULL, L"Service succesfully removed!", L"Sleep HDD service", MB_OK | MB_ICONINFORMATION);
			CloseServiceHandle(hService);
			CloseServiceHandle(hServiceIOControlManager);
			return 0;
		}
	}
	SERVICE_TABLE_ENTRY service_table[] = 
	{
		{serviceName, ServiceMain},
		{NULL, NULL}
	};
	if (!StartServiceCtrlDispatcher(service_table))
	{
		logService.open(servicePahtToLog, std::ofstream::out | std::ofstream::app);
		logService << "Start service failed!" << endl << flush;
		return 3;
	}
	return 0;
}

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	hServiceStatus = RegisterServiceCtrlHandler(serviceName,ServiceCtrlHandler);
	if (!hServiceStatus){
		logService << "Register service handler control failed!" <<endl <<flush;
		return;
	}
	service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	service_status.dwCurrentState = SERVICE_START_PENDING;
	service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	service_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
	service_status.dwServiceSpecificExitCode = 0;
	service_status.dwCheckPoint = 0;
	service_status.dwWaitHint = 5000;

	if(!SetServiceStatus(hServiceStatus, &service_status))
	{			
		logService << "Set service status Service Start pending failed!"<<endl <<flush;
		return;
	}

	service_status.dwWin32ExitCode = NO_ERROR;
	service_status.dwCurrentState = SERVICE_RUNNING;
	if(!SetServiceStatus(hServiceStatus, &service_status))
	{			
		logService << "Set service status Start pending failed!"<<endl <<flush;
		return;
	}

	logService << "Service is started!" << endl << flush;

	while(service_status.dwCurrentState == SERVICE_RUNNING)
	{
		//main function
	}
}

VOID WINAPI ServiceCtrlHandler(DWORD dwControl)
{
	switch(dwControl)
	{
	case SERVICE_CONTROL_STOP:
		logService << "Service is finished."<< endl << flush;
		ShutFlag = TRUE;
		service_status.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hServiceStatus, &service_status);
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		service_status.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hServiceStatus, &service_status);
		logService.flush();
		logService.close();
		break;
	default:
		if (dwControl > 127 && dwControl < 256) /* User Defined */
		break;
	}
	++service_status.dwCheckPoint;
	SetServiceStatus(hServiceStatus, &service_status);
	return;
}