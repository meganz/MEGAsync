#ifndef REGUTILS_H
#define REGUTILS_H

#include <windows.h>

HRESULT SetRegistryKeyAndValue(HKEY  hkey, PCWSTR pszSubKey, PCWSTR pszValueName,
    PCWSTR pszData);

HRESULT GetRegistryKeyAndValue(HKEY hkey, PCWSTR pszSubKey, PCWSTR pszValueName,
    PWSTR pszData, DWORD cbData);

HRESULT RegisterInprocServer(PCWSTR pszModule, const CLSID& clsid,
    PCWSTR pszFriendlyName, PCWSTR pszThreadModel);

HRESULT UnregisterInprocServer(const CLSID& clsid);

HRESULT RegisterShellExtContextMenuHandler(
    PCWSTR pszFileType, const CLSID& clsid, PCWSTR pszFriendlyName);

HRESULT RegisterShellExtIconHandler(const CLSID& clsid, PCWSTR pszFriendlyName);

HRESULT UnregisterShellExtContextMenuHandler(
    PCWSTR pszFileType, const CLSID& clsid, PCWSTR pszFriendlyName);

HRESULT UnregisterShellExtIconHandler(const CLSID& clsid, PCWSTR pszFriendlyName);

#endif