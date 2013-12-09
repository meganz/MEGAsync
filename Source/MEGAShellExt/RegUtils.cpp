#include "RegUtils.h"
#include <strsafe.h>

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
    if (SUCCEEDED(hr))
    {
        hr = HRESULT_FROM_WIN32(RegDeleteTree(HKEY_CLASSES_ROOT, szSubkey));
    }

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
    
    hr = HRESULT_FROM_WIN32(RegDeleteTree(HKEY_CLASSES_ROOT, szSubkey));
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

    hr = HRESULT_FROM_WIN32(RegDeleteTree(HKEY_LOCAL_MACHINE, szSubkey));
    if (!SUCCEEDED(hr)) return hr;


	HKEY hkey;
	hr = RegOpenKey(HKEY_LOCAL_MACHINE,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved",
		&hkey);
	if (!SUCCEEDED(hr)) return hr;

    hr = HRESULT_FROM_WIN32(RegDeleteValue(hkey, szCLSID));
    if (!SUCCEEDED(hr)) return hr;

    return hr;
}
