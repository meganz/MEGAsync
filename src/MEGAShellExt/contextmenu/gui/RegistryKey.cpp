#include "RegistryKey.h"

RegistryKey::RegistryKey(HKEY hKey, const wstring& subKey, REGSAM access, bool createIfMissing):
    m_hKey(nullptr),
    m_regsam(access),
    m_originalHKey(hKey),
    m_originalSubKey(subKey)
{
    if (RegOpenKeyExW(hKey, subKey.data(), 0, access, &m_hKey) == ERROR_SUCCESS)
    {
        return;
    }

    if (!createIfMissing)
    {
        return;
    }

    DWORD disposition = 0;

    if (RegCreateKeyExW(hKey,
                        subKey.data(),
                        0,
                        nullptr,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        nullptr,
                        &m_hKey,
                        &disposition) != ERROR_SUCCESS)
    {
        throw runtime_error("Failed to create registry key.");
    }
}

RegistryKey::~RegistryKey()
{
    if (m_hKey != nullptr)
    {
        RegCloseKey(m_hKey);
    }
}

wstring RegistryKey::GetStringValue(const wstring& valueName)
{
    if (m_hKey == nullptr)
    {
        throw runtime_error("Registry key is not open.");
    }

    DWORD dataSize = 0;

    if (RegGetValueW(m_hKey,
                     nullptr,
                     valueName.empty() ? NULL : valueName.data(),
                     RRF_RT_REG_SZ,
                     nullptr,
                     nullptr,
                     &dataSize) != ERROR_SUCCESS)
    {
        throw runtime_error("Failed to get registry value size.");
    }

    wstring value(dataSize / sizeof(wchar_t), L'\0');

    if (RegGetValueW(m_hKey,
                     nullptr,
                     valueName.empty() ? NULL : valueName.data(),
                     RRF_RT_REG_SZ,
                     nullptr,
                     _Bit_cast<BYTE*>(value.data()),
                     &dataSize) != ERROR_SUCCESS)
    {
        throw runtime_error("Failed to get registry value.");
    }

    return value;
}
