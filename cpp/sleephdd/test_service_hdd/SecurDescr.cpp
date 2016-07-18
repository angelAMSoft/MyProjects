#include "SecurDescr.h"

SecurDescr::SecurDescr(): pEveryoneSID(NULL), pAdminSID(NULL),
						  pACL(NULL), pSD(NULL)
{
	ea = new EXPLICIT_ACCESS[2];
}

SECURITY_ATTRIBUTES SecurDescr::CreateSID()
{
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
	AllocateAndInitializeSid(&SIDAuthWorld, 1,
                     SECURITY_WORLD_RID,
                     0, 0, 0, 0, 0, 0, 0,
                     &pEveryoneSID);
	ZeroMemory(ea, 2 * sizeof(EXPLICIT_ACCESS));
    ea[0].grfAccessPermissions = FILE_GENERIC_READ | FILE_GENERIC_WRITE | SYNCHRONIZE;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance= NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

	AllocateAndInitializeSid(&SIDAuthNT, 2,
                     SECURITY_BUILTIN_DOMAIN_RID,
                     DOMAIN_ALIAS_RID_ADMINS,
                     0, 0, 0, 0, 0, 0,
                     &pAdminSID);

	ea[1].grfAccessPermissions = FILE_GENERIC_READ | FILE_GENERIC_WRITE | SYNCHRONIZE;
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance= NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ea[1].Trustee.ptstrName  = (LPTSTR) pAdminSID;

	SetEntriesInAcl(2, ea, NULL, &pACL);

	pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
                             SECURITY_DESCRIPTOR_MIN_LENGTH);

	InitializeSecurityDescriptor(pSD,
            SECURITY_DESCRIPTOR_REVISION);

	SetSecurityDescriptorDacl(pSD, 
            TRUE,     // bDaclPresent flag   
            pACL, 
            FALSE);

	sa.nLength = sizeof (SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = FALSE;

	return sa;
}

SecurDescr::~SecurDescr()
{
	if (pEveryoneSID) 
        FreeSid(pEveryoneSID);
    if (pAdminSID) 
        FreeSid(pAdminSID);
    if (pACL) 
        LocalFree(pACL);
    if (pSD) 
        LocalFree(pSD);

	delete[] ea;
	ea = NULL;
}

