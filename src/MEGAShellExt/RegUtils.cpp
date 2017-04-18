#include "RegUtils.h"
#include <strsafe.h>
#include <Winreg.h>
#include <tchar.h>

//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all its subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;
BOOL IsWow64()
{
    BOOL bIsWow64 = FALSE;
    fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
        GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
    if (fnIsWow64Process)
    {
        fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
    }
    return bIsWow64;
}

bool DeleteRegKey(HKEY key, LPTSTR subkey, REGSAM samDesired)
{
    TCHAR keyPath[MAX_PATH];
    DWORD maxSize;
    HKEY hKey;

    LONG result = RegDeleteKeyEx(key, subkey, samDesired, 0);
    if (result == ERROR_SUCCESS)
    {
        return true;
    }

    result = RegOpenKeyEx(key, subkey, 0, samDesired | KEY_READ, &hKey);
    if (result != ERROR_SUCCESS)
    {
        return (result == ERROR_FILE_NOT_FOUND);
    }

    int len =  _tcslen(subkey);
    if (!len || len >= (MAX_PATH - 1) || _tcscpy_s(keyPath, MAX_PATH, subkey))
    {
        RegCloseKey(hKey);
        return false;
    }

    LPTSTR endPos = keyPath + len - 1;
    if (*(endPos++) != TEXT('\\'))
    {
        *(endPos++) =  TEXT('\\');
        *endPos =  TEXT('\0');
        len++;
    }

    do
    {
        maxSize = MAX_PATH - len;
    } while (RegEnumKeyEx(hKey, 0, endPos, &maxSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS
             && DeleteRegKey(key, keyPath, samDesired));

    RegCloseKey(hKey);
    return (RegDeleteKeyEx(key, subkey, samDesired, 0) == ERROR_SUCCESS);
}

bool DeleteRegValue(HKEY key, LPTSTR subkey, LPTSTR value, REGSAM samDesired)
{
    HKEY hKey;
    LONG result = RegOpenKeyEx(key, subkey, 0, samDesired | KEY_SET_VALUE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        return (result == ERROR_FILE_NOT_FOUND);
    }

    result = RegDeleteValue(hKey, value);
    RegCloseKey(hKey);
    return (result == ERROR_SUCCESS);
}

bool CheckLeftPaneIcon(wchar_t *path, bool remove)
{
    HKEY hKey = NULL;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER,
                               L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace",
                               0, KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hKey);
    if (result != ERROR_SUCCESS)
    {
        return false;
    }

    DWORD retCode = ERROR_SUCCESS;
    WCHAR uuid[MAX_PATH];
    DWORD uuidlen = sizeof(uuid);

    for (int i = 0; retCode == ERROR_SUCCESS; i++)
    {
        uuidlen = sizeof(uuid);
        uuid[0] = '\0';
        retCode = RegEnumKeyEx(hKey, i, uuid, &uuidlen, NULL, NULL, NULL, NULL);
        if (retCode == ERROR_SUCCESS)
        {
            HKEY hSubKey;
            TCHAR subKeyPath[MAX_PATH];
            swprintf_s(subKeyPath, MAX_PATH, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace\\%s", uuid);
            result = RegOpenKeyEx(HKEY_CURRENT_USER, subKeyPath, 0, KEY_READ, &hSubKey);
            if (result != ERROR_SUCCESS)
            {
                continue;
            }

            DWORD type;
            TCHAR value[MAX_PATH];
            DWORD valuelen = sizeof(value);
            result = RegQueryValueEx(hSubKey, L"", NULL, &type, (LPBYTE)value, &valuelen);
            RegCloseKey(hSubKey);
            if (result != ERROR_SUCCESS || type != REG_SZ || valuelen != 10 || memcmp(value, L"MEGA", 10))
            {
                continue;
            }

            if (path)
            {
                bool found = false;

                swprintf_s(subKeyPath, MAX_PATH, L"Software\\Classes\\CLSID\\%s\\Instance\\InitPropertyBag", uuid);
                result = RegOpenKeyEx(HKEY_CURRENT_USER, subKeyPath, 0, KEY_READ, &hSubKey);
                if (result == ERROR_SUCCESS)
                {
                    valuelen = sizeof(value);
                    result = RegQueryValueEx(hSubKey, L"TargetFolderPath", NULL, &type, (LPBYTE)value, &valuelen);
                    RegCloseKey(hSubKey);
                    if (result == ERROR_SUCCESS && type == REG_EXPAND_SZ && !_wcsicmp(value, path))
                    {
                        found = true;
                    }
                }

                if (found)
                {
                    swprintf_s(subKeyPath, MAX_PATH, L"Software\\Classes\\CLSID\\%s\\Instance\\InitPropertyBag", uuid);
                    result = RegOpenKeyEx(HKEY_CURRENT_USER, subKeyPath, 0, KEY_WOW64_64KEY | KEY_READ, &hSubKey);
                    if (result == ERROR_SUCCESS)
                    {
                        valuelen = sizeof(value);
                        result = RegQueryValueEx(hSubKey, L"TargetFolderPath", NULL, &type, (LPBYTE)value, &valuelen);
                        RegCloseKey(hSubKey);
                        if (result == ERROR_SUCCESS && type == REG_EXPAND_SZ && !_wcsicmp(value, path))
                        {
                            found = true;
                        }
                    }
                }

                if (!found)
                {
                    continue;
                }
            }

            if (remove)
            {
                wchar_t buffer[MAX_PATH];
                swprintf_s(buffer, MAX_PATH, L"Software\\Classes\\CLSID\\%s", uuid);

                if (IsWow64())
                {
                    DeleteRegKey(HKEY_CURRENT_USER, buffer, KEY_WOW64_64KEY);
                }
                DeleteRegKey(HKEY_CURRENT_USER, buffer, 0);

                DeleteRegValue(HKEY_CURRENT_USER,
                               (LPTSTR)L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel",
                               uuid, 0);
                swprintf_s(buffer, MAX_PATH, L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace\\%s", uuid);
                DeleteRegKey(HKEY_CURRENT_USER, buffer, 0);
            }
            RegCloseKey(hKey);
            return true;
        }
    }
    RegCloseKey(hKey);
    return false;
}

BOOL RegDelNodeRecurse (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    LPTSTR lpEnd;
    LONG lResult;
    DWORD dwSize;
    TCHAR szName[MAX_PATH];
    HKEY hKey;
    FILETIME ftWrite;

    // First, see if we can delete the key without having
    // to recurse.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)
        return TRUE;

    lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

    if (lResult != ERROR_SUCCESS)
    {
        if (lResult == ERROR_FILE_NOT_FOUND) {
            printf("Key not found.\n");
            return TRUE;
        }
        else {
            printf("Error opening key.\n");
            return FALSE;
        }
    }

    // Check for an ending slash and add one if it is missing.

    lpEnd = lpSubKey + lstrlen(lpSubKey);

    if (*(lpEnd - 1) != TEXT('\\'))
    {
        *lpEnd =  TEXT('\\');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    // Enumerate the keys

    dwSize = MAX_PATH;
    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                           NULL, NULL, &ftWrite);

    if (lResult == ERROR_SUCCESS)
    {
        do {

            StringCchCopy (lpEnd, MAX_PATH*2, szName);

            if (!RegDelNodeRecurse(hKeyRoot, lpSubKey)) {
                break;
            }

            dwSize = MAX_PATH;

            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
                                   NULL, NULL, &ftWrite);

        } while (lResult == ERROR_SUCCESS);
    }

    lpEnd--;
    *lpEnd = TEXT('\0');

    RegCloseKey (hKey);

    // Try again to delete the key.

    lResult = RegDeleteKey(hKeyRoot, lpSubKey);

    if (lResult == ERROR_SUCCESS)
        return TRUE;

    return FALSE;
}

//*************************************************************
//
//  RegDelnode()
//
//  Purpose:    Deletes a registry key and all its subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL RegDelNode (HKEY hKeyRoot, LPTSTR lpSubKey)
{
    TCHAR szDelKey[MAX_PATH*2];

    StringCchCopy (szDelKey, MAX_PATH*2, lpSubKey);
    return RegDelNodeRecurse(hKeyRoot, szDelKey);
}

HRESULT SetRegistryKeyAndValue(HKEY  hkey, PCWSTR pszSubKey, PCWSTR pszValueName,
    PCWSTR pszData)
{
    HRESULT hr;
    HKEY hKey = NULL;

    // Creates the specified registry key. If the key already exists, the
    // function opens it.
    hr = HRESULT_FROM_WIN32(RegCreateKeyEx(hkey, pszSubKey, 0,
        NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL));

    if (SUCCEEDED(hr))
    {
        if (pszData != NULL)
        {
            // Set the specified value of the key.
            DWORD cbData = lstrlen(pszData) * sizeof(*pszData);
            hr = HRESULT_FROM_WIN32(RegSetValueEx(hKey, pszValueName, 0,
                REG_SZ, reinterpret_cast<const BYTE *>(pszData), cbData));
        }

        RegCloseKey(hKey);
    }

    return hr;
}

HRESULT GetRegistryKeyAndValue(HKEY hkey, PCWSTR pszSubKey, PCWSTR pszValueName,
    PWSTR pszData, DWORD cbData)
{
    HRESULT hr;
    HKEY hKey = NULL;

    // Try to open the specified registry key.
    hr = HRESULT_FROM_WIN32(RegOpenKeyEx(hkey, pszSubKey, 0,
        KEY_READ, &hKey));

    if (SUCCEEDED(hr))
    {
        // Get the data for the specified value name.
        hr = HRESULT_FROM_WIN32(RegQueryValueEx(hKey, pszValueName, NULL,
            NULL, reinterpret_cast<LPBYTE>(pszData), &cbData));

        RegCloseKey(hKey);
    }

    return hr;
}


HRESULT RegisterInprocServer(PCWSTR pszModule, const CLSID& clsid,
    PCWSTR pszFriendlyName, PCWSTR pszThreadModel)
{
    if (pszModule == NULL || pszThreadModel == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;

    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

    wchar_t szSubkey[MAX_PATH];

    // Create the HKCR\CLSID\{<CLSID>} key.
    hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
    if (SUCCEEDED(hr))
    {
        hr = SetRegistryKeyAndValue(HKEY_CLASSES_ROOT, szSubkey, NULL, pszFriendlyName);

        // Create the HKCR\CLSID\{<CLSID>}\InprocServer32 key.
        if (SUCCEEDED(hr))
        {
            hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey),
                L"CLSID\\%s\\InprocServer32", szCLSID);
            if (SUCCEEDED(hr))
            {
                // Set the default value of the InprocServer32 key to the
                // path of the COM module.
                hr = SetRegistryKeyAndValue(HKEY_CLASSES_ROOT, szSubkey, NULL, pszModule);
                if (SUCCEEDED(hr))
                {
                    // Set the threading model of the component.
                    hr = SetRegistryKeyAndValue(HKEY_CLASSES_ROOT, szSubkey,
                        L"ThreadingModel", pszThreadModel);
                }
            }
        }
    }

    return hr;
}

HRESULT UnregisterInprocServer(const CLSID& clsid)
{
    HRESULT hr = S_OK;

    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

    wchar_t szSubkey[MAX_PATH];

    hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
    if (SUCCEEDED(hr)) RegDelNode(HKEY_CLASSES_ROOT, szSubkey);

    return hr;
}


HRESULT RegisterShellExtContextMenuHandler(
    PCWSTR pszFileType, const CLSID& clsid, PCWSTR pszFriendlyName)
{
    if (pszFileType == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;

    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

    wchar_t szSubkey[MAX_PATH];

    // If pszFileType starts with '.', try to read the default value of the
    // HKCR\<File Type> key which contains the ProgID to which the file type
    // is linked.
    if (*pszFileType == L'.')
    {
        wchar_t szDefaultVal[260];
        hr = GetRegistryKeyAndValue(HKEY_CLASSES_ROOT, pszFileType, NULL, szDefaultVal,
            sizeof(szDefaultVal));

        // If the key exists and its default value is not empty, use the
        // ProgID as the file type.
        if (SUCCEEDED(hr) && szDefaultVal[0] != L'\0')
        {
            pszFileType = szDefaultVal;
        }
    }

    // Create the key HKCR\<File Type>\shellex\ContextMenuHandlers\pszFriendlyName {<CLSID>}
    hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey),
        L"%s\\shellex\\ContextMenuHandlers\\%s", pszFileType, pszFriendlyName);
    if (!SUCCEEDED(hr)) return hr;

    // Set the default value of the key.
    hr = SetRegistryKeyAndValue(HKEY_CLASSES_ROOT, szSubkey, NULL, szCLSID);
    if (!SUCCEEDED(hr)) return hr;

    // Create the key HKCR\Directory\shellex\ContextMenuHandlers\pszFriendlyName {<CLSID>}
    hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey),
        L"Directory\\shellex\\ContextMenuHandlers\\%s", pszFriendlyName);
    if (!SUCCEEDED(hr)) return hr;

    // Set the default value of the key.
    hr = SetRegistryKeyAndValue(HKEY_CLASSES_ROOT, szSubkey, NULL, szCLSID);
    return hr;
}


HRESULT RegisterShellExtIconHandler(const CLSID& clsid, PCWSTR pszFriendlyName)
{
    HRESULT hr;

    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));
    wchar_t szSubkey[MAX_PATH];

    // Create the key HKCR\<File Type>\shellex\ContextMenuHandlers\pszFriendlyName {<CLSID>}
    hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey),
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers\\%s", pszFriendlyName);
    if (!SUCCEEDED(hr)) return hr;


    // Set the default value of the key.
    hr = SetRegistryKeyAndValue(HKEY_LOCAL_MACHINE, szSubkey, NULL, szCLSID);
    if (!SUCCEEDED(hr)) return hr;

    hr = SetRegistryKeyAndValue(HKEY_LOCAL_MACHINE,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved",
        szCLSID, pszFriendlyName);

    return hr;
}

HRESULT UnregisterShellExtContextMenuHandler(
    PCWSTR pszFileType, const CLSID& clsid, PCWSTR pszFriendlyName)
{
    if (pszFileType == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr;

    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

    wchar_t szSubkey[MAX_PATH];

    // If pszFileType starts with '.', try to read the default value of the
    // HKCR\<File Type> key which contains the ProgID to which the file type
    // is linked.
    if (*pszFileType == L'.')
    {
        wchar_t szDefaultVal[260];
        hr = GetRegistryKeyAndValue(HKEY_CLASSES_ROOT, pszFileType, NULL, szDefaultVal,
            sizeof(szDefaultVal));

        // If the key exists and its default value is not empty, use the
        // ProgID as the file type.
        if (SUCCEEDED(hr) && szDefaultVal[0] != L'\0')
        {
            pszFileType = szDefaultVal;
        }
    }

    hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey),
        L"%s\\shellex\\ContextMenuHandlers\\%s", pszFileType, pszFriendlyName);
    if (!SUCCEEDED(hr)) return hr;
    
    RegDelNode(HKEY_CLASSES_ROOT, szSubkey);
    return hr;
}


HRESULT UnregisterShellExtIconHandler(const CLSID& clsid, PCWSTR pszFriendlyName)
{
    HRESULT hr;
    wchar_t szCLSID[MAX_PATH];
    StringFromGUID2(clsid, szCLSID, ARRAYSIZE(szCLSID));

    wchar_t szSubkey[MAX_PATH];
    hr = StringCchPrintf(szSubkey, ARRAYSIZE(szSubkey),
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellIconOverlayIdentifiers\\%s", pszFriendlyName);
    if (!SUCCEEDED(hr)) return hr;
    RegDelNode(HKEY_LOCAL_MACHINE, szSubkey);

    HKEY hkey;
    hr = RegOpenKey(HKEY_LOCAL_MACHINE,
                    L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved",
                    &hkey);
    if (!SUCCEEDED(hr)) return hr;

    hr = HRESULT_FROM_WIN32(RegDeleteValue(hkey, szCLSID));
    if (!SUCCEEDED(hr)) return hr;

    return hr;
}
