#ifndef LOCKVOLUME_H
#define LOCKVOLUME_H
#include "SleepHDD.h"

DWORD WINAPI  LockVolume(LPVOID lpParam);
int UnlockVolume(std::wstring& disk);

#endif //LOCKVOLUME_H