#ifndef SLEEPHDD_H
#define SLEEPHDD_H

#include "ManageThread.h"
#include <vector>
#include <string>
#include <Windows.h>
#include <winioctl.h>
#include <map>
#include "SleppHDDClients.h"

#define BUFSIZE 8192
#define SMALLBUF 2048
#define SIZELETTERWCHAR 2
#define MAX_SZ	0x69
const int _SECOND = 10000000;

extern LPTSTR lpszPipename;
extern LPTSTR lockMutex;
extern CRITICAL_SECTION* CriticalSection;

typedef std::map<std::wstring, std::wstring>::value_type mapValType;

extern std::map<std::wstring, std::wstring>* ListMutex;
extern std::map<std::wstring, std::wstring>* ListTimers;
extern std::map<std::wstring, Operations>* ListCurrentBusyDisks;

extern ManageThread* ListThreads;

#endif //SLEEPHDD_H