// clang-format off
#include "Utilities.h"

#include "framework.h"
#include <winrt/Windows.UI.ViewManagement.h>

#include <filesystem>
#include <string>
#include <iostream>
#include <sstream>

using namespace winrt::Windows::UI::ViewManagement;

extern HMODULE g_hInst;

namespace Utilities
{
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Register Values
std::wstring getRegisterKeyStringValue(HKEY hKey,
                                       const std::wstring& subKey,
                                       const std::wstring& valueName)
{
    std::wstring value;

    HKEY keyRef;
    if (RegOpenKeyExW(hKey, subKey.data(), 0, KEY_READ, &keyRef) != ERROR_SUCCESS)
    {
        return value;
    }

    DWORD dataSize = 0;
    if (RegGetValueW(keyRef,
                     nullptr,
                     valueName.empty() ? NULL : valueName.data(),
                     RRF_RT_REG_SZ,
                     nullptr,
                     nullptr,
                     &dataSize) != ERROR_SUCCESS)
    {
        return value;
    }

    value = dataSize / sizeof(wchar_t);

    if (RegGetValueW(keyRef,
                     nullptr,
                     valueName.empty() ? NULL : valueName.data(),
                     RRF_RT_REG_SZ,
                     nullptr,
                     std::_Bit_cast<BYTE*>(value.data()),
                     &dataSize) != ERROR_SUCCESS)
    {
        return value;
    }

    return value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Paths
constexpr int WINDOWS11_FIRST_BUILD_NUMBER = 22000;

bool haveModernContextMenu()
{
    std::wstring buildNumberStr(
        getRegisterKeyStringValue(HKEY_LOCAL_MACHINE,
                                  L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
                                  L"CurrentBuildNumber"));

    const int buildNumber = std::stoi(buildNumberStr);

    return buildNumber >= WINDOWS11_FIRST_BUILD_NUMBER;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Security and Identity
void resetAcl()
{
    auto acl = (PACL)malloc(sizeof(ACL));

    if (acl)
    {
        InitializeAcl(acl, sizeof(ACL), ACL_REVISION);
        SetNamedSecurityInfoW(const_cast<LPWSTR>(GetMEGADesktopAppPath().c_str()),
                              SE_FILE_OBJECT,
                              DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION,
                              NULL,
                              NULL,
                              acl,
                              NULL);

        free(acl);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Paths
const std::filesystem::path GetModulePath(HMODULE inst, bool onlyFileName)
{
    wchar_t pathBuffer[MAX_PATH] = {0};
    GetModuleFileNameW(inst, pathBuffer, MAX_PATH);
    if (onlyFileName)
    {
        PathStripPathW(pathBuffer);
    }
    return std::filesystem::path(pathBuffer);
}

const std::wstring GetMEGADesktopAppPath()
{
    std::filesystem::path modulePath = GetModulePath(g_hInst, false);
    return modulePath.parent_path().parent_path().wstring();
}

const std::wstring GetContextMenuPath()
{
    std::filesystem::path modulePath = GetModulePath(g_hInst, false);
    return modulePath.parent_path().wstring();
}

const std::wstring GetExecutingModuleName()
{
    std::wstring moduleName(GetModulePath(NULL, true).wstring());
    transform(moduleName.begin(), moduleName.end(), moduleName.begin(), towlower);

    return moduleName;
}

void log(const std::wstring& file, const std::wstring& message)
{
    HANDLE hFileCreate = CreateFile(
        file.c_str(),
        FILE_APPEND_DATA,
        0,
        NULL,
        CREATE_NEW,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    CloseHandle(hFileCreate);

    HANDLE hFile = CreateFile(
        file.c_str(),
        FILE_APPEND_DATA,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    SetFilePointer(hFile, 0, NULL, FILE_END);

    std::wstringstream wss;
    wss << message;
    wss << std::endl;

    std::wstring linedContent = wss.str();

    DWORD bytesWritten;
    WriteFile(
        hFile,
        linedContent.c_str(),
        static_cast<DWORD>(linedContent.size() * sizeof(wchar_t)),
        &bytesWritten,
        NULL);

    CloseHandle(hFile);
}

/*
 * These functions have been copied from microsoft samples.
 * https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/ui/apply-windows-themes
 */
inline bool isColorLight(winrt::Windows::UI::Color& clr)
{
    return (((5 * clr.G) + (2 * clr.R) + clr.B) > (8 * 128));
}

bool isDarkModeActive()
{
    auto settings = UISettings();
    auto foreground = settings.GetColorValue(UIColorType::Foreground);

    return isColorLight(foreground);
}

void updateExplorer()
{
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
}

}
