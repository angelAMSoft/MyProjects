#ifdef WIN32
#define WakeOn WakeOnWindows
#define GetMac GetMacFromIPWin
#define ReadSettings ReadSettingsWin
#include <winsock2.h>
#include <iphlpapi.h>
#include <stdio.h>

#else
#define WakeOn WakeOnUnix
#define GetMac GetMacFromIPNix
#define ReadSettings ReadSettingsNix
#define LENGHTLINE 128
//#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <ctype.h>
#endif

const int lenAddr = 16;
const int MACADDRLen = 18;
char* TargetMac;
char* Interface;
char* SrcIP;
char* DestIP;

int ReadSettingsWin();
int ReadSettingsNix();
char* GetMacFromIPWin(const char* targetIpString);
char* GetMacFromIPNix(const char* targetIpString);
int InitializeWSA();

int main(void)
{
	int res;
	#ifdef WIN32
	if (InitializeWSA() > 0)
		return 1;
	#endif
	res = ReadSettings();
	if(res == 1)
		printf("File succesfully created!\n");
	else if(res == 2){
		printf("Current settings:\n");
		printf("Target IP: %s\n", DestIP);
		printf("Target MAC: %s\n", TargetMac);
		printf("Broadcast interface: %s\n", Interface);
	}
	#ifdef WIN32
	WSACleanup();
	#endif
	return 0;
}

#ifdef WIN32
char* GetMacFromIPWin(const char* targetIpString)
{
	IPAddr DestIp = 0;
	IPAddr SrcIp = 0;       /* default for src ip */
	ULONG MacAddr[2];       /* for 6-byte hardware addresses */
	ULONG PhysAddrLen = 6;  /* default to length of six bytes */
	DWORD dwRetVal;
	unsigned int i;
	unsigned char *bPhysAddr;
	int pos = 0;
	char* resultMac = (char*)malloc(sizeof(char) * MACADDRLen);
	DestIp = inet_addr(targetIpString);
    memset(&MacAddr, 0xff, sizeof (MacAddr));
	memset(resultMac, 0, sizeof(char) * MACADDRLen);
    //printf("Sending ARP request for IP address: %s\n", targetIpString);
    dwRetVal = SendARP(DestIp, SrcIp, &MacAddr, &PhysAddrLen);
	if (dwRetVal == NO_ERROR) {
        bPhysAddr = (BYTE *) & MacAddr;
        if (PhysAddrLen) {
            for (i = 0; i < (int) PhysAddrLen; i++) {
                if (i == (PhysAddrLen - 1)){
                    //printf("%.2X\n", (int) bPhysAddr[i]);
					sprintf(&resultMac[pos], "%.2X", (int) bPhysAddr[i]);
					pos += 2;
				}
                else{
                    //printf("%.2X-", (int) bPhysAddr[i]);
					sprintf(&resultMac[pos], "%.2X-", (int) bPhysAddr[i]);
					pos += 3;
				}
            }
        } else
            printf("Warning: SendArp completed successfully, but returned length=0\n");
	}
	return resultMac;
}

int InitializeWSA()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return 1;
    }
	return 0;
}

int ReadSettingsWin()
{
	int result = 0;
	FILE *fp;
	const char *fileName = "settings.ini";
	int answer = 0;
	char host_name[128] = {0};
	char ch = '\0';
	char* myIp;
	char** Readbuf;
	struct hostent *remoteHost;
	struct in_addr addr;
	int i = 0;
	int ethNum;
	int lastDot = 0;
	int params = 0;
	int pos = 0;
	if((fp = fopen(fileName, "r")) == NULL){
		printf("File %s not found!\n", fileName);
		printf("Create settings file?(1 - Yes, 2 - No)\n");
		scanf("%d", &answer);
		if(answer == 2)
			return result;
		else {
			fp = fopen(fileName, "w");
			gethostname(host_name, 128);
			remoteHost = gethostbyname(host_name);
			if (remoteHost->h_addrtype == AF_INET)
			{
				while (remoteHost->h_addr_list[i] != 0) {
					addr.s_addr = *(u_long *) remoteHost->h_addr_list[i++];
					printf("\tIP Address #%d: %s\n", i, inet_ntoa(addr));
				}
			}
			while(1){
				printf("\nEnter a number of interface: ");
				scanf("%d", &ethNum);
				if(ethNum >= 0 && ethNum <= i)
					break;
			}
			addr.s_addr = *(u_long *) remoteHost->h_addr_list[ethNum - 1];
			SrcIP = inet_ntoa(addr);
			myIp = (char*)malloc(sizeof(char) * lenAddr);
			memset(myIp, 0, sizeof(char) * lenAddr);
			memcpy(myIp, SrcIP, strlen((const char*)SrcIP));
			for(i = 0; i < lenAddr; i++){
				if(myIp[i] == '.')
					lastDot = i;
			}
			myIp[lastDot + 1] = '2';
			myIp[lastDot + 2] = '5';
			myIp[lastDot + 3] = '5';
			Interface = myIp;
			printf("\nEnter a target IP adress: ");
			DestIP = (char*)malloc(sizeof(char) * lenAddr);
			memset(DestIP, 0, sizeof(char) * lenAddr);
			scanf("%s", DestIP);
			TargetMac = GetMac(DestIP);
			fputs(DestIP, fp);
			fputc('\n', fp);
			fputs(TargetMac, fp);
			fputc('\n', fp);
			fputs(myIp, fp);
			fclose(fp);
			result = 1;
		}
	}
	else{
		//0 destination ip
		//1 MAC address
		//2 Broadcast intrface
		DestIP = (char*)malloc(sizeof(char) * lenAddr);
		memset(DestIP, 0, sizeof(char) * lenAddr);
		TargetMac = (char*)malloc(sizeof(char) * MACADDRLen);
		memset(TargetMac, 0, sizeof(char) * MACADDRLen);
		Interface = (char*)malloc(sizeof(char) * lenAddr);
		memset(Interface, 0, sizeof(char) * lenAddr);
		Readbuf = (char**)malloc(sizeof(char*) * 3);
		Readbuf[0] = DestIP;
		Readbuf[1] = TargetMac;
		Readbuf[2] = Interface;
		ch = fgetc(fp);
		while(ch != EOF){
			if(ch == '\n'){
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
		Readbuf = NULL;
		result = 2;
		fclose(fp);
	}
	return result;
}

#else
char* GetMacFromIPNix(const char* targetIpString)
{
	char commandPing[27] = {0};
	const char* arpFile = "/proc/net/arp";
	char line[LENGHTLINE] = {0};
	FILE* fp;
	char* findIpStr = NULL;
	char* mac = (char*)malloc(MACADDRLen * sizeof(char));
	int counter;
	sprintf(commandPing, "ping %s -c 1", targetIpString);
	system(commandPing);
	memset(mac, 0, MACADDRLen * sizeof(char));
	if((fp = fopen(arpFile, "r")) != NULL){
		while(fgets(line, LENGHTLINE, fp) != NULL){
			findIpStr = strstr(line, targetIpString);
			if(findIpStr == NULL){
				memset(line, 0, LENGHTLINE);
				continue;
			}
			findIpStr = strtok(findIpStr, " ");
			for(counter = 0; counter < 5; counter++){
				findIpStr = strtok(NULL, " ");
				if(counter == 2)
					memcpy(mac, findIpStr, MACADDRLen);
			}
		}
	}
	return mac;
}

void ReadListInterfaces(char **netcards)
{
	FILE *fresult;
	char* resultBuf;
	char* ethIp;
	char* positionIP = NULL;
	int pos = 0;
	int lenghtstr;
	int linenum = 0;
	int ethNum = 0;
	int i = 0;
	system("ifconfig > result");
	resultBuf = (char*)malloc(256 * sizeof(char));
	if (resultBuf == NULL){
	    printf("Alloc resultfbuf error!\n");
	    return;
	}
	printf("alloc resultbuf complete!\n");
	memset(resultBuf, 0, 256 * sizeof(char));
	fresult = fopen("result", "r");
	if (fresult == NULL){
		printf("File result not created!");
		return;
	}
	printf("file result open completed!\n");
	while(!feof(fresult)){
		if(fgets(resultBuf, 256, fresult) != NULL){
			//printf("%s", resultBuf);
			lenghtstr = strlen(resultBuf);
			if(lenghtstr == 0 || lenghtstr == 1){
				linenum = 0;
				ethNum++;
				i = 0;
				continue;
			}
			if(linenum == 0){
				for(pos = 0; pos < lenghtstr; pos++){
					if (resultBuf[pos] == ' ')
						break;
				}
				ethIp = (char*)malloc((pos + 1) * sizeof(char));
				if(ethIp == NULL)
					printf("ethip alloc error!\n");
				memset(ethIp, 0, (pos + 1) * sizeof(char));
				memcpy(ethIp, resultBuf, pos);
				printf("Interface %d: %s ", ethNum, ethIp);
				memset(resultBuf, 0, 256 * sizeof(char));
				free(ethIp);
			}
			else if(linenum == 1){
				positionIP = strstr(resultBuf, "inet addr");
				if(positionIP != NULL){
					positionIP = &positionIP[10];
					ethIp = (char*)malloc(sizeof(char) * lenAddr);
					if(ethIp == NULL)
						printf("ethip alloc error!\n");
					memset(ethIp, 0, sizeof(char) * lenAddr);
					//printf("Start reading ip!");
					i  = 0;
					while(positionIP[i] != ' ' && i < lenAddr){
						ethIp[i] = positionIP[i];
						//putchar(ethIp[i]);
						//printf("i: %d \n", i); 
						i++;
					}
					printf("(IP: %s)\n", ethIp);
				}
				else printf("\n");
				netcards[ethNum] = ethIp;
			}

			linenum++;
		}
	}

	fclose(fresult);
	remove("result");
	free(resultBuf);

}
int ReadSettingsNix()
{
	int result = 0;
	FILE *fp;
	const char *fileName = "/etc/wakeon/settings_wakeGitServer";
	//char ethDev[5] = {'e','t','h','0','\0'};
	int answer = 0;
	//char* myIp;
	char ch;
	char** Readbuf;
	//struct ifreq ifr;
	//struct sockaddr_in* ipaddr;
	//
	char** netcards;
	int ethNum;
	//
	int i = 0;
	int lastDot = 0;
	//int fd;
	//int num = 0x30;
	int params = 0;
	int pos = 0;
	//size_t if_name_len;
	if((fp = fopen(fileName, "r")) == NULL){
		printf("File %s not found!\n", fileName);
		printf("Create settings file?(1 - Yes, 2 - No)\n");
		scanf("%d", &answer);
		if(answer == 2)
			return result;
		else {
			netcards = (char**)malloc(20 * sizeof(char*));
			ReadListInterfaces(netcards);
			/*
			netcards = (char**)malloc(9 * sizeof(char*));
			//struct sockaddr_in** adresses = (struct sockaddr_in**)malloc(sizeof(struct sockaddr_in*) * 9);
			fd=socket(AF_INET,SOCK_DGRAM,0);
			for(i = 0; i < 10; i++, num++){
				ethDev[3] = num;
				if_name_len=strlen(ethDev);
				if (if_name_len<sizeof(ifr.ifr_name)) {
					memcpy(ifr.ifr_name,ethDev,if_name_len);
					ifr.ifr_name[if_name_len]=0;
				}
				if (ioctl(fd,SIOCGIFADDR,&ifr)==-1) {
					continue;
				}

			 ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
			 netcards[i] = inet_ntoa(ipaddr->sin_addr);
			 printf("IP address eth%d: %s\n",i,inet_ntoa(ipaddr->sin_addr));

			 close(fd);
			 */
			}
			while(1){
				printf("\nEnter a number of interface: ");
				scanf("%d", &ethNum);
				if(ethNum >= 0 && ethNum <= i)
					break;
			}
			SrcIP = netcards[ethNum];
			Interface = (char*)malloc(sizeof(char) * lenAddr);
			memset(Interface, 0, sizeof(char) * lenAddr);
			memcpy(Interface, SrcIP, strlen((const char*)SrcIP));
			for(i = 0; i < lenAddr; i++){
				if(Interface[i] == '.')
					lastDot = i;
			}
			Interface[lastDot + 1] = '2';
			Interface[lastDot + 2] = '5';
			Interface[lastDot + 3] = '5';
			printf("\nEnter a target IP adress: ");
			DestIP = (char*)malloc(sizeof(char) * lenAddr);
			memset(DestIP, 0, sizeof(char) * lenAddr);
			scanf("%s", DestIP);
			TargetMac = GetMac(DestIP);
			fp = fopen(fileName, "w");
			fputs(DestIP, fp);
			fputc('\n', fp);
			fputs(TargetMac, fp);
			fputc('\n', fp);
			fputs(Interface, fp);
			fclose(fp);
			result = 1;
		}
	else{
		//0 destination ip
		//1 MAC address
		//2 Broadcast interface
		DestIP = (char*)malloc(sizeof(char) * lenAddr);
		memset(DestIP, 0, sizeof(char) * lenAddr);
		TargetMac = (char*)malloc(sizeof(char) * MACADDRLen);
		memset(TargetMac, 0, sizeof(char) * MACADDRLen);
		Interface = (char*)malloc(sizeof(char) * lenAddr);
		memset(Interface, 0, sizeof(char) * lenAddr);
		Readbuf = (char**)malloc(sizeof(char*) * 3);
		Readbuf[0] = DestIP;
		Readbuf[1] = TargetMac;
		Readbuf[2] = Interface;
		ch = fgetc(fp);
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
		Readbuf = NULL;
		result = 2;
		fclose(fp);
	}
	return result;
}
#endif
