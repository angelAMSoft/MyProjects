#include <time.h>

#ifdef WIN32
#define WakeOn WakeOnWindows
#define GetMac GetMacFromIPWin
#define ReadSettings ReadSettingsWin
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <ws2tcpip.h>
#include <windows.h>

#else
#define WakeOn WakeOnUnix
#define GetMac GetMacFromIPNix
#define ReadSettings ReadSettingsNix
#define LENGHTLINE 128
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#endif
int server(void);
//Размер очереди для запросов
#define QUEUE_LENGTH 40
//Размер буфера для различных нужд
#define BUF_LEN 24
//Порт сервера
#define P_PORT 4622

unsigned char* StrToMac(char* input);
char* TargetMac;
char* Interface;
char* SrcIP;
char* DestIP;

const int lenAddr = 16;
const int MACADDRLen = 18;

int WakeOnWindows(unsigned char* mac);
int WakeOnUnix(unsigned char* mac);
int ReadSettingsWin();
int ReadSettingsNix();
char* GetMacFromIPWin(const char* targetIpString);
char* GetMacFromIPNix(const char* targetIpString);
void writeLog();

int main(void)
{
	int res = ReadSettings();
	if(res == 3){
		writeLog("Run wakeon_config for create settings file!");
		return 1;
	}
	writeLog("Server started succesfully!");
	server();
	return 0;
}

int HexIntToDecInt(char* byte)
{
	int result;
	char smallLetters[] = {'a', 'b', 'c', 'd', 'e', 'f'};
	char bigLetters[]   = {'A', 'B', 'C', 'D', 'E', 'F'};
	int numbers[]       = {10 , 11 ,  12,  13,  14, 15};
	int hexDigit;
	int isBig = 1;
	int found = 0;
	int i;
	char digits[2] = {0};
	digits[0] = byte[0];
	if(isdigit(digits[0]) > 0)
		result = atoi(digits);
	else{
		for(i = 0; i < sizeof(smallLetters); ++i){
			if(smallLetters[i] == digits[0]){
				hexDigit = i;
				isBig = 0;
				found = 1;
				break;
			}
		}
		if(isBig){
			for(i = 0; i < sizeof(bigLetters); ++i){
				if(bigLetters[i] == digits[0]){
					hexDigit = i;
					found = 1;
					break;
				}
			}
		}
		if(found)
			result = numbers[hexDigit];
	}
	result *= 16;
	digits[0] = byte[1];
	if(isdigit(digits[0]) > 0)
		result += atoi(digits);
	else{
		for(i = 0; i < sizeof(smallLetters); ++i){
			if(smallLetters[i] == digits[0]){
				hexDigit = i;
				isBig = 0;
				found = 1;
				break;
			}
		}
		if(isBig){
			for(i = 0; i < sizeof(bigLetters); ++i){
				if(bigLetters[i] == digits[0]){
					hexDigit = i;
					found = 1;
					break;
				}
			}
		}
		if(found)
			result += numbers[hexDigit];
	}
	return result;
}

unsigned char* StrToMac(char* input)
{
	unsigned char splits[3] = {':', ' ', '-'};
	unsigned char* p = (unsigned char*)malloc(sizeof(unsigned char) * 102);
	unsigned char StrBytes[6][3];
	unsigned char byte[3] = {0};
	unsigned char buff[6] = {0};
	int indexMac = 0;
	int mac = 0;
	int digit = 0;
	int indexSplits = 0;
	int hexNum;
	int i, j, n, m;
	int k = 0;
	int find = 0;
	int lenghtInput = strlen((const char*)input);
	memset(p, 0, sizeof(unsigned char) * 102);
	while(indexSplits < 3 && !find){
		for(i = 0; i < lenghtInput; i++){
			if(input[i] == splits[indexSplits]){
				find = 1;
				break;
			}
		}
		if(!find)
			indexSplits++;
	}

	for(j = 0; j < lenghtInput; j++){
		if(input[j] != splits[indexSplits])
		{
			byte[digit++] = input[j];
		}
		else{
			digit = 0;
			StrBytes[mac][0] = byte[0];
			StrBytes[mac][1] = byte[1];
			StrBytes[mac][2] = '\0';
			mac++;
		}
	}
	StrBytes[mac][0] = byte[0];
	StrBytes[mac][1] = byte[1];
	StrBytes[mac][2] = '\0';
	for(j = 0; j < 6; j++){
		hexNum = HexIntToDecInt((char*)StrBytes[j]);
		buff[indexMac] = (unsigned char)hexNum;
		indexMac++;
	}
	for(; k < 6; ++k)
		p[k] = (unsigned char)0xFF;

	for(n= 0; n < 16; n++)
		for(m = 0; m < 6; m++)
			p[k++] = buff[m];

	return p;
}

void writeLog(char* message)
{
	FILE* fp;
	time_t rawtime;
	char * strTime;
	char *logMessage;
	struct tm * timeinfo;
	int lenghtMessage = 0, lenghtDate;
	
#if WIN32
	const char *fileName = "c:\\windows\logs\\wakeon_server.log";
#else
	const char *fileName = "/etc/wakeon/wakeon_server.log";
#endif
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	strTime = asctime(timeinfo);
	lenghtMessage = strlen(message);
	lenghtDate = strlen(strTime);
	strTime[lenghtDate - 1] = '\0';
	logMessage = (char*)malloc((lenghtMessage + lenghtDate + 4) * sizeof(char));
	memset(logMessage, 0, (lenghtMessage + lenghtDate + 4) * sizeof(char));
	sprintf(logMessage, "%s - %s\n", strTime, message);
	fp = fopen(fileName, "a");
	fwrite(logMessage, sizeof(char), strlen(logMessage) * sizeof(char), fp);
	free(logMessage);
	fclose(fp);
}

#ifdef WIN32
#else
int WakeOnUnix(unsigned char* mac)
{
	int udpSocket;
    struct sockaddr_in udpClient, udpServer;
    int broadcast = 1;

    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

    /** you need to set this so you can broadcast **/
    if (setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
		return 1;
    }
    udpClient.sin_family = AF_INET;
    udpClient.sin_addr.s_addr = INADDR_ANY;
    udpClient.sin_port = 0;

    bind(udpSocket, (struct sockaddr*)&udpClient, sizeof(udpClient));

    /** …make the packet as shown above **/

    /** set server end point (the broadcast addres)**/
    udpServer.sin_family = AF_INET;
    udpServer.sin_addr.s_addr = inet_addr(Interface);
    udpServer.sin_port = htons(9);

    /** send the packet **/
    sendto(udpSocket, (const char*)mac, sizeof(unsigned char) * 102, 0, (struct sockaddr*)&udpServer, sizeof(udpServer));
	return 0;
}

int ReadSettingsNix()
{
	int result = 0;
	FILE *fp;
	const char *fileName = "/etc/wakeon/settings_wakeGitServer";
	char ch = '\0';
	char** Readbuf;
	int params = 0;
	int pos = 0;
	if((fp = fopen(fileName, "r")) == NULL){
		printf("File %s not found!\n", fileName);
		//printf("Run wakeon_config!\n");
		result = 3;
	}
	else{
		//0 destination ip
		//1 MAC address
		//2 Broadcast intrface
		printf("Alloc destip\n");
		DestIP = (char*)malloc(sizeof(char) * lenAddr);
		if(DestIP == NULL)
			printf("Alloc destip failed\n");
		memset(DestIP, 0, sizeof(char) * lenAddr);
		printf("Alloc targetmac\n");
		TargetMac = (char*)malloc(sizeof(char) * MACADDRLen);
		if(TargetMac == NULL)
			printf("Alloc targetip failed\n");
		memset(TargetMac, 0, sizeof(char) * MACADDRLen);
		Interface = (char*)malloc(sizeof(char) * lenAddr);
		if(Interface == NULL)
			printf("Alloc Interface failed\n");
		memset(Interface, 0, sizeof(char) * lenAddr);
		
		Readbuf = (char**)malloc(sizeof(char*) * 4);
		if(Readbuf == NULL)
			printf("Readbuf alloc failed\n");
		Readbuf[0] = DestIP;
		Readbuf[1] = TargetMac;
		Readbuf[2] = Interface;
		ch = fgetc(fp);
		printf("Read file...\n");
		while(!feof(fp)){
			if(ch == 0xa){
				params++;
				pos = 0;
				ch = fgetc(fp);
			}
			else{
				Readbuf[params][pos++] = ch;
				ch = fgetc(fp);
			}
		}
		free(Readbuf);
		printf("read file completed!\n");
		Readbuf = NULL;
		result = 1;
		fclose(fp);
	}
	return result;
}

int server(void)
{
	struct sockaddr_in stSockAddr;
	int sock,clientsock;
	char * buf;
	int count;
	int bsock;
	int res;
	unsigned char* mac;
	char* message = NULL;
	//Инициализация сокета
	
	sock = socket(PF_INET, SOCK_STREAM,0);
	if (sock == -1){
		printf("socket() error\n");
		return 1;
	}
	printf("socket init...\n");
	buf = (char *) malloc(BUF_LEN * sizeof(char));
	memset(buf, 0, BUF_LEN * sizeof(char));
	if(buf == NULL) {
		printf("malloc() error\n");
		free(buf);
		close(sock);
		return 1;
	}
	//Настройка сервера работы на порт указаный в определении P_PORT и прием сообщений от клиента с любым адресом
	memset( &stSockAddr, 0, sizeof( stSockAddr ) );
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_addr.s_addr = INADDR_ANY;
	stSockAddr.sin_port = htons(P_PORT);
	printf("Bind socket...\n");
	if (bind(sock, (struct sockaddr *)&stSockAddr,sizeof(struct sockaddr)) == -1){
		printf("bind() error \n");
		close(sock);
		return 1;
	}
	printf("Listen socket...\n");
	if (listen(sock, QUEUE_LENGTH) == -1){
		printf ("listen() error \n");
		close(sock);
		return 1;
	}

	//Чтение данных от клиента
	while (1){
	printf("Wait connection...\n");
		clientsock = accept(sock, NULL, NULL);

		if (clientsock == -1){
			printf("accept() error \n");
			close(sock);
			return 1;
		}
		if ((count = read(clientsock,buf,BUF_LEN-1)) == -1){
			printf("read() error\n");
			close(sock);
			return 1;
		}

		buf[count] = '\0';
		if (strcmp(buf, "WakeON") == 0) {
			//printf("Wake computer...\n");
			mac = StrToMac(TargetMac);
			res = WakeOn(mac);
			message = (char*)malloc(sizeof(char) * (30 + MACADDRLen + lenAddr));
			memset(message, 0, sizeof(char) * (30 + MACADDRLen + lenAddr));
			sprintf(message, "Wake computer %s(%s) succesfull!", DestIP, TargetMac); 
			writeLog(message);
			free(message);
			free(mac);
		}
		close(clientsock);

	}
	return 0;
}

#endif
