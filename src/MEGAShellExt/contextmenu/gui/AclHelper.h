#pragma once
#include "framework.h"

class AclHelper
{
public:
    AclHelper();
    ~AclHelper();

    DWORD ResetAcl(const wstring& path);

private:
    PACL emptyAcl;
};
