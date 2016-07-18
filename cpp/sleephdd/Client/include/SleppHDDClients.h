//Interface to sleephdd service
#ifndef SLEEPHDDCLIENTS_H
#define SLEEPHDDCLIENTS_H
#include <Windows.h>

typedef struct Operation{
	TCHAR* Data;
	DWORD size;
}OPERATION, *POPERATION;

enum Operations{
	NO_OPERATION,
	LOCK,
	UNLOCK,
	ACTIVE,
	INACTIVE,
	QUERY,
	ENDALLACTIVETASK,
	ENDALLLOCKEDTASK
};

#endif SLEEPHDDCLIENTS_H