#ifndef SECURDESCR_H
#define SECURDESCR_H
#include <Windows.h>
#include <aclapi.h>

class SecurDescr{
public:
	SecurDescr();
	~SecurDescr();
	SECURITY_ATTRIBUTES CreateSID(void);

private:
	PSID pEveryoneSID, pAdminSID;
	PACL pACL;
	PSECURITY_DESCRIPTOR pSD;
    EXPLICIT_ACCESS *ea;
    SECURITY_ATTRIBUTES sa;
};


#endif //SECURDESCR_H