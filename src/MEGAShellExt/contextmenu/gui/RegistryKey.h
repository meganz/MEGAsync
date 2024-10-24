#pragma once
#include "framework.h"

class RegistryKey
{
public:
    RegistryKey(HKEY hKey,
                const wstring& subKey = L"",
                REGSAM access = KEY_READ,
                bool createIfMissing = false);
    ~RegistryKey();

    wstring GetStringValue(const wstring& valueName);

private:
    HKEY m_hKey;
    REGSAM m_regsam;
    HKEY m_originalHKey;
    wstring m_originalSubKey;
};
