#include "QueryDisks.h"

#define MAX_UNICODE_SYMBOLS 255
int GetWMI(wchar_t* wmiClass, std::vector<DISKPROPERTIES> &result);