#ifndef MAKEACTIVEDISK_H
#define MAKEACTIVEDISK_H
#include "SleepHDD.h"

DWORD WINAPI ProcessHDDIsActive(LPVOID lpParam);
int ProcessHDDIsInActive(std::wstring& disk);
VOID CALLBACK TimerAPCProc(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue);

#endif // MAKEACTIVEDISK_H