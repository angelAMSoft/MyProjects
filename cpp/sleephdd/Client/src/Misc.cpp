#include <Windows.h>


char* CreateBufferToSend(char* inCommand, char* inArguments, int* outLenght)
{
	int lenCommand = lstrlenA(inCommand);
	int lenArguments = lstrlenA(inArguments);
	*outLenght = 8 + 1 + lenCommand + lenArguments;
	char* outBuf = new char[8 + 1 + lenCommand + lenArguments];
	ZeroMemory(outBuf, 8 + 1 + lenCommand + lenArguments);
	char* tmp = (char*)&lenCommand;
	int position = 0;
	for(int i = 0; i < 4; i++){
		outBuf[i] = tmp[i];
		position++;
	}
	int j = 0;
	for(int k = position; j < lenCommand; k++, j++){
		outBuf[k] = inCommand[j];
	}
	position += lenCommand;
	tmp = (char*)&lenArguments;
	for(int i = 0; i < 4; i++){
		outBuf[position++] = tmp[i];
	}
	j = 0;
	for(int k = position; j < lenArguments; k++, j++){
		outBuf[k] = inArguments[j];
	}
	return outBuf;
}

char* WideToChar(const wchar_t* in, UINT TranslateCodePage)
{
	DWORD dwNum = WideCharToMultiByte(TranslateCodePage, NULL,in,-1,NULL,0,NULL,NULL);
	char* buffer = new char[dwNum + 1];
	ZeroMemory(buffer, sizeof(char) * dwNum + 1); 
	char* asciiBuffer = buffer;
	WideCharToMultiByte(TranslateCodePage, NULL, in, lstrlen(in) ,(LPSTR)asciiBuffer, dwNum, NULL,NULL);
	return asciiBuffer;
}

wchar_t* CharToWide(const char* in, UINT TranslateCodePage)
{
	DWORD dwNum = MultiByteToWideChar (TranslateCodePage, 0, in, -1, NULL, 0);
	wchar_t* buffer = new wchar_t[dwNum + 1];
	ZeroMemory(buffer, sizeof(wchar_t) * dwNum + 1); 
	wchar_t* WideBuffer = buffer;
	MultiByteToWideChar (TranslateCodePage, 0, in, -1, WideBuffer, dwNum );
	return WideBuffer;
}