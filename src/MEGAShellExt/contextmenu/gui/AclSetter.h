#ifndef ACLSETTER_H
#define ACLSETTER_H

#include "framework.h"

namespace AclSetter
{
void resetAcl(const std::wstring& path)
{
    auto acl = (PACL)malloc(sizeof(ACL));

    if (acl)
    {
        InitializeAcl(acl, sizeof(ACL), ACL_REVISION);
        SetNamedSecurityInfoW(const_cast<LPWSTR>(path.c_str()),
                              SE_FILE_OBJECT,
                              DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION,
                              NULL,
                              NULL,
                              acl,
                              NULL);

        free(acl);
    }
}
}

#endif
