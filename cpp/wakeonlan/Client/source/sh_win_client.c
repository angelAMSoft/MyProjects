#ifndef UNICODE
#define UNICODE 1
#endif

#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"Kernel32.lib")

#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <Tchar.h>
#define P_PORT 4622 //���� ��� ���������� � ��������
#define BUF_LEN 4096
#define NO_COMPUTERS -1

int SH_ReadFile(LPCTSTR FileName, char *out_buf);
//������� ������ ���� � ����� � ���������� ���������� ����������� ����

int main(void)

{
//������������� ������ � ��������� sockaddr
SOCKET ClientSock = INVALID_SOCKET;
struct sockaddr_in ClientSAddr;
char * buf = "WakeON";
int ConVal;
int iResult;
char ip_addr_string[BUF_LEN]="";
char tmp_buf[BUF_LEN]="";
int bytes_read; //���������� ����������� ���� �����
int i; //�������� ��� �������������� ������ ���� �����������

//������������� WSA ������ 2.0
WSADATA WSStartData;
if (WSAStartup (MAKEWORD (2, 0), &WSStartData) != 0)
		printf("WSAStart error %d\n",WSAGetLastError());
//�������� ������ ��� ������ ����������� �� ����
ClientSock = socket(AF_INET, SOCK_STREAM, 0);
if (ClientSock == INVALID_SOCKET){
		fprintf(stderr, "socket() error - %d\n",WSAGetLastError());
		getchar();
		WSACleanup();
		return 1;
	}
bytes_read = SH_ReadFile(L"SH_settings.ini",tmp_buf);
if (bytes_read != -1){
		//�������� ������� �� ������
for(i=0; i < bytes_read; i++)
    {
		if(tmp_buf[i] !=';')
			ip_addr_string[i] = tmp_buf[i];
		else
		{
			break;
		}
	}
	
}
else
{
	printf("Not found file settings\n");
	printf("IP server - localhost\n");
	strcpy_s(ip_addr_string,BUF_LEN,"127.0.0.1");
}


//���������� ��������� sockaddr
memset( &ClientSAddr, 0, sizeof( ClientSAddr ) );
	ClientSAddr.sin_family = AF_INET; //����� �� ��������� IP4
	ClientSAddr.sin_addr.s_addr = inet_addr(ip_addr_string); //����� ����������
	ClientSAddr.sin_port = htons(P_PORT); //���� ����������
//������� ���������� � ��������
ConVal = connect (ClientSock, (struct sockaddr *)&ClientSAddr, sizeof(ClientSAddr));
	if (ConVal == SOCKET_ERROR){ 
	fprintf(stderr, "connect() to %s error - %d\n",ip_addr_string,WSAGetLastError());
	closesocket(ClientSock);
	WSACleanup();
	printf("Press any key to close\n");
	getchar();
	return 1;
	}
	
	//�������� ������ �� ������
iResult = send( ClientSock, buf, (int)strlen(buf), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSock);
        WSACleanup();
        return 1;
    }
	printf("transmitted %d bytes\n", iResult);
	
	//�������� ������ ����� ������ ��� ������ ��� ��������
  iResult = shutdown(ClientSock, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSock);
        WSACleanup();
        return 1;
    }

    WSACleanup();
    return 0;
}

int SH_ReadFile(LPCTSTR FileName, char *out_buf)
{
	HANDLE file_in; //���������� �����
	DWORD  dwBytesRead; //���������� ����������� ����
    file_in = CreateFile(FileName,           // ��� ������������ �����
                          GENERIC_READ,          // ����� ������
                          FILE_SHARE_READ,       
                          NULL,                 
                          OPEN_EXISTING,         // �������� ������ ������������� �����
                          FILE_ATTRIBUTE_NORMAL, // ����������� ��������� �����
                          NULL);                 

    if (file_in == INVALID_HANDLE_VALUE)
    {
        _tprintf(TEXT("Terminal failure: unable to open file \"%s\" for read.in error - %d\n"), FileName,GetLastError());
        return NO_COMPUTERS;
    }
	//������ ����� ����������� ����� �����������
    if( FALSE == ReadFile(file_in, out_buf, BUF_LEN, &dwBytesRead, NULL) )
    {
        printf("Terminal failure: Unable to read from file.in error %d \n", GetLastError());
        CloseHandle(file_in);
        return NO_COMPUTERS;
    }
	//�������� ���� ��� ������ ���� �������
    if (dwBytesRead > 0 && dwBytesRead <= BUF_LEN-1)
    {
        out_buf[dwBytesRead]='\0';
        //_tprintf(TEXT("Data read from %s (%d bytes): \n"), FileName, dwBytesRead);

    }
    else if (dwBytesRead == 0)
    {
        _tprintf(TEXT("No data read from file %s\n"), FileName);
		CloseHandle(file_in);
		return NO_COMPUTERS;
    }
    else
    {
        printf("\n ** Unexpected value for dwBytesRead ** \n");
    }

    CloseHandle(file_in); //�������� ����������� �����
	return (int)dwBytesRead;

}