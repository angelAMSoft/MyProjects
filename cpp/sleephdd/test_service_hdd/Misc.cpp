#include "Misc.h"
using std::wstring;
using std::vector;

void CreateLetterFromVolume(const WCHAR *volume, WCHAR* letter)
{
	letter[0] = volume[0];
	letter[1] = L'\0';
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

char* WideToChar(const wchar_t* in, UINT TranslateCodePage)
{
	DWORD dwNum = WideCharToMultiByte(TranslateCodePage, NULL,in,-1,NULL,0,NULL,NULL);
	char* buffer = new char[dwNum + 1];
	ZeroMemory(buffer, sizeof(char) * dwNum + 1); 
	char* asciiBuffer = buffer;
	WideCharToMultiByte(TranslateCodePage, NULL, in, lstrlen(in) ,(LPSTR)asciiBuffer, dwNum, NULL,NULL);
	return asciiBuffer;
}

void CreateVolumePath(std::wstring& path)
{
	wstring oldPath(path);
	wstring newPath(L"\\\\.\\");
	int lenght = oldPath.size();
	for(wstring::size_type index = 0; index != lenght; index++)
		newPath += oldPath[index];
	path.clear();
	path = newPath;
}

void ParsingData(const WCHAR *data, int size, vector<wstring> &out)
{
	wstring tmpStr;
	int lenght = size / sizeof(WCHAR);
	for(int i = 0; i < lenght; ++i)
	{
		if(data[i] != L';')
		{
			tmpStr += data[i];
		}
		else
		{
			out.push_back(tmpStr);
			tmpStr.clear();
		}
	}
}

bool IsFileToHDDExists(const wchar_t* root, wstring& fullPath){
	HANDLE fileActive = INVALID_HANDLE_VALUE;
	WCHAR fileName[] = L"SleepHDD.dat";
	bool FileExists = true;
	int lenght = 0;
	wstring FullFilePath(root);
	FullFilePath += '\\';
	FullFilePath += fileName;
	fileActive = CreateFile(FullFilePath.c_str(), GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL, // normal file
                       NULL);

	if (fileActive == INVALID_HANDLE_VALUE){
		DWORD err = GetLastError();
		if(err == ERROR_FILE_NOT_FOUND){
			FileExists = false;
		}
	}
	else
	{
		CloseHandle(fileActive);
	}
	fullPath = FullFilePath.c_str();
	return FileExists;
}


char** GetCommandAndArgs(char* in, int size)
{
	char** result = new char*[2];
	int position = 4;
	int* Commandlenght;
	int* argsLenght;
	char tmp[4] = {0};
	for(int i = 0; i < 4; i++){
		tmp[i] = in[i];
	}
	Commandlenght = (int*)tmp;
	result[0] = new char[*Commandlenght + 1];
	ZeroMemory(result[0], *Commandlenght + 1);
	for(int i = 0; i < *Commandlenght; i++){
		result[0][i] = in[position++];
	}
	for(int i = 0; i < 4; i++, position++){
		tmp[i] = in[position];
	}
	argsLenght = (int*)tmp;
	result[1] = new char[*argsLenght + 1];
	ZeroMemory(result[1], *argsLenght + 1);
	for(int i = 0; i < *argsLenght; i++, position++){
		result[1][i] = in[position];
	}
	return result;
}