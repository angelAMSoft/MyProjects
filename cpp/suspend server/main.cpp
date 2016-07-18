#include <winsock2.h>
#include <Windows.h>
#include <Ws2tcpip.h>
#include <string>
#include <fstream>
#include <cstdlib>
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"Kernel32.lib")
#pragma comment(lib,"Advapi32.lib")


#define P_PORT 3663 
//Размер буфера для различных нужд
#define BUF_LEN 4096
#define QUEUE_LENGTH 40
using namespace std;
char serviceName[] = "SuspendServer";
char servicePahtToLog[] = "C:\\Windows\\logs\\Suspend server service.log";

void SuspendThisServer(void);
VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
DWORD Server(void);
VOID WINAPI ServiceCtrlHandler(DWORD dwControl);
SERVICE_STATUS service_status;
SERVICE_STATUS_HANDLE hServiceStatus;
HANDLE  hThread;
BOOL ShutFlag = FALSE;
ofstream logService;

typedef struct Arguments {
    int arg1;
    int arg2;
} ARGUMENTS, *PARGUMENTS;



int main(int argc, char **argv)
{	
	if (argc == 2)
	{
		if (lstrcmp(argv[1], "i") == 0)
		{
			SC_HANDLE hServiceIOControlManager, hService;
			hServiceIOControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
			hService = OpenService(hServiceIOControlManager, serviceName, GENERIC_READ);
			if (hService == NULL){
				HMODULE hmodule = GetModuleHandle(NULL);
				char psz_file_name[MAX_PATH + 1] = "";
				GetModuleFileName(hmodule,psz_file_name,MAX_PATH + 1);
				hService = CreateService(hServiceIOControlManager, serviceName, "Suspend Server",
										 SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, 
										 SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
										 psz_file_name, NULL, NULL, NULL, NULL, NULL);
				if (hService != NULL)
				{	
					MessageBox(NULL, "Service succesfully installed!", "Suspend Server", MB_OK | MB_ICONINFORMATION);		
					StartService(hService, 0, NULL);
					CloseServiceHandle(hService);
				}
				CloseServiceHandle(hServiceIOControlManager);
				return 1;
			}
			else
			{
				MessageBox(NULL, "Service already installed!", "Suspend Server", MB_OK | MB_ICONINFORMATION);
				CloseServiceHandle(hService);
				CloseServiceHandle(hServiceIOControlManager);
				return 0;
			}
		}
		else if (lstrcmp(argv[1], "u") == 0)
		{
			SC_HANDLE hServiceIOControlManager, hService;
			hServiceIOControlManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
			hService = OpenService(hServiceIOControlManager, serviceName, SERVICE_ALL_ACCESS | DELETE);
			if (hService == NULL){
				MessageBox(NULL, "Service not installed!", "Suspend Server", MB_OK | MB_ICONERROR);
				CloseServiceHandle(hServiceIOControlManager);
				return 2;
			}
			if (DeleteService(hService))
				MessageBox(NULL, "Service succesfully removed!", "Suspend Server", MB_OK | MB_ICONINFORMATION);
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
		logService << "Start service failed!";
		logService.close();
		return 3;
	}
	return 0;
	}

void SuspendThisServer(void)
{
	HANDLE hToken; //Дескриптор уровня привилегий приложения
	TOKEN_PRIVILEGES *NewState; // Указатель для переопределения привилегий

	OpenProcessToken (GetCurrentProcess (), TOKEN_ADJUST_PRIVILEGES, &hToken); //Чтение привилегий приложения
	NewState = (TOKEN_PRIVILEGES*) malloc (sizeof
                                        (TOKEN_PRIVILEGES) + sizeof (LUID_AND_ATTRIBUTES));
	NewState->PrivilegeCount = 1;
	LookupPrivilegeValue (NULL, 
						SE_SHUTDOWN_NAME, //Привилегии для удаленной перезагрузки
                        &NewState->Privileges[0].Luid);
	NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 
	AdjustTokenPrivileges (hToken, FALSE, NewState, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL);
	SetSystemPowerState(TRUE, TRUE);
	free (NewState);
	CloseHandle (hToken); //Закрытие дескрипторов
}

VOID WINAPI ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
	hServiceStatus = RegisterServiceCtrlHandler(serviceName,ServiceCtrlHandler);
	if (!hServiceStatus){
		logService.open(servicePahtToLog, std::ofstream::out | std::ofstream::app);
		logService << "Register service handler control failed!";
		logService.close();
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
		logService.open(servicePahtToLog, std::ofstream::out | std::ofstream::app);
		logService << "Set service status Service Start pending failed!";
		logService.close();
		return;
	}

	service_status.dwWin32ExitCode = NO_ERROR;
	service_status.dwCurrentState = SERVICE_RUNNING;
	if(!SetServiceStatus(hServiceStatus, &service_status))
	{			
		logService.open(servicePahtToLog, std::ofstream::out | std::ofstream::app);
		logService << "Set service status Start pending failed!";
		logService.close();
		return;
	}

	logService.open(servicePahtToLog, std::ofstream::out | std::ofstream::app);
	logService << "Service is started!" << endl << flush;
	logService.close();

	while(service_status.dwCurrentState == SERVICE_RUNNING)
	{
		Server();
	}
}

DWORD Server(void)
{
	WSADATA WSStartData;
    //Инициализация сокетов
    SOCKET ServerSock = INVALID_SOCKET; //Сокет сервера
    SOCKET SockIO = INVALID_SOCKET; //сокет для принятия запросов

    struct sockaddr_in ServerSAddr; //структура адреса для сокета сервера
    char recvbuf[BUF_LEN] = ""; //буфер поступивших запросов
    int recvbuflen = BUF_LEN; //размер буфера для поступифших запросов
    int ioResult; //размер принятых байт
	WSAStartup (MAKEWORD (2, 0), &WSStartData);
	ServerSock = socket(
		AF_INET, //Обмен по IP протоколу
		SOCK_STREAM, //Транспортный протокол TCP
		0);

	memset( &ServerSAddr, 0, sizeof( ServerSAddr ) ); // обнуление структуры
    //Заполнение структуры sockaddr
    ServerSAddr.sin_family = AF_INET;
    ServerSAddr.sin_addr.s_addr = htonl(INADDR_ANY); //Принятие запросов с любых адресов
    ServerSAddr.sin_port = htons(P_PORT); //порт на котором будет работать сервер

	if (bind( ServerSock,
              (SOCKADDR*) &ServerSAddr,
              sizeof(ServerSAddr)) == SOCKET_ERROR)
    {
		logService.open(servicePahtToLog, std::ofstream::out | std::ofstream::app);
		logService << "bind() failed. - " << WSAGetLastError();
		logService.close();
        closesocket(ServerSock);
        return 1;
    }


	if (listen(ServerSock, QUEUE_LENGTH) == SOCKET_ERROR)
    {
		logService.open(servicePahtToLog, std::ofstream::out | std::ofstream::app);
		logService << "listen() error - " << WSAGetLastError();
		logService.close();
        WSACleanup();
        return 1;
    }

	
	  do
    {
        // Принятие поступивших запросов
        SockIO = accept(ServerSock, NULL, NULL);
        if (SockIO == SOCKET_ERROR)
        {
			logService.open(servicePahtToLog, std::ofstream::out | std::ofstream::app);
			logService << "Accept error";
			logService.close();
            return 1;
        }
        //Чтение пакетов
        ioResult = recv(SockIO, recvbuf, recvbuflen, 0);
        if ( ioResult > 0 )
        {

            if (strcmp(recvbuf, "off") == 0) //Если в буфере содержится Turn, то начинается обработка компьютеров
			{
			logService.open(servicePahtToLog, std::ofstream::out | std::ofstream::app);
			logService << "signal recieve";
			logService.close();
			SuspendThisServer();
			}
		}
        else if ( ioResult == 0 )
        {
            closesocket(SockIO);
        }
        else
		{
			logService.open(servicePahtToLog, std::ofstream::out | std::ofstream::app);
			logService << "recv failed - " << WSAGetLastError();
			logService.close();
		}
	}
    while( (ioResult > 0) && !ShutFlag);

	shutdown (ServerSock, 2);
	closesocket (ServerSock);
	WSACleanup();
	return 0;
}

VOID WINAPI ServiceCtrlHandler(DWORD dwControl)
{
	switch(dwControl)
	{
	case SERVICE_CONTROL_STOP:
		logService.open(servicePahtToLog, std::ofstream::out | std::ofstream::app);
		logService << "Service is finished.";
		logService.close();
		ShutFlag = TRUE;
		service_status.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hServiceStatus, &service_status);
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		service_status.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(hServiceStatus, &service_status);
		break;
	default:
		++service_status.dwCheckPoint;
		SetServiceStatus(hServiceStatus, &service_status);
		break;
	}
	return;
}