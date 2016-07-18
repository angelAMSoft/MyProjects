#ifndef CONSOLE_MISC_H
#define CONSOLE_MISC_H

char* CreateBufferToSend(char* inCommand, char* inArguments, int* outLenght);
char* WideToChar(const wchar_t* in, UINT TranslateCodePage = CP_UTF8);
wchar_t* CharToWide(const char* in, UINT TranslateCodePage = CP_ACP);

#define BUFSIZE 4096
#define lpszPipename "\\\\.\\pipe\\mynamedpipe"

#endif //CONSOLE_MISC_H