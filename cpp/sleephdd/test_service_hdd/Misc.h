#ifndef MISC_H
#define MISC_H
#include "SleepHDD.h"


void ParsingData(const WCHAR *data, int size, std::vector<std::wstring> &out);
void CreateVolumePath(std::wstring& path);
void CreateLetterFromVolume(const WCHAR *volume, WCHAR* letter);
bool IsFileToHDDExists(const wchar_t* root, std::wstring& fullPath);
char** GetCommandAndArgs(char* in, int size);
wchar_t* CharToWide(const char* in, UINT TranslateCodePage = CP_ACP);
char* WideToChar(const wchar_t* in, UINT TranslateCodePage = CP_UTF8);

#endif //MISC_H