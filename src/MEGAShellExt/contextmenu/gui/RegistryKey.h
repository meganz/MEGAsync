#ifndef REGISTERUTILITIES_H
#define REGISTERUTILITIES_H

#include "framework.h"

class RegistryKey
{
public:
    RegistryKey(HKEY hKey,
                const std::wstring& subKey = L"",
                REGSAM access = KEY_READ,
                bool createIfMissing = false);
    ~RegistryKey();

    std::wstring GetStringValue(const std::wstring& valueName);

private:
    HKEY m_hKey;
    REGSAM m_regsam;
    HKEY m_originalHKey;
    std::wstring m_originalSubKey;
};

#endif
