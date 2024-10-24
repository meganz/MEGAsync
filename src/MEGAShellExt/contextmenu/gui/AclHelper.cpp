#include "AclHelper.h"

AclHelper::AclHelper()
{
    emptyAcl = (PACL)malloc(sizeof(ACL));

    if (emptyAcl)
    {
        InitializeAcl(emptyAcl, sizeof(ACL), ACL_REVISION);
    }
}

AclHelper::~AclHelper()
{
    if (emptyAcl)
    {
        free(emptyAcl);
    }
}

DWORD AclHelper::ResetAcl(const wstring& path)
{
    if (emptyAcl)
    {
        return SetNamedSecurityInfoW(const_cast<LPWSTR>(path.c_str()),
                                     SE_FILE_OBJECT,
                                     DACL_SECURITY_INFORMATION |
                                         UNPROTECTED_DACL_SECURITY_INFORMATION,
                                     NULL,
                                     NULL,
                                     emptyAcl,
                                     NULL);
    }
    else
    {
        return ERROR_OUTOFMEMORY;
    }
}
