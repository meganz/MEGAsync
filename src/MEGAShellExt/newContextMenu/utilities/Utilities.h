// clang-format off
#ifndef UTILITIES_H
#define UTILITIES_H

#include <windows.h>
#include <xstring>

namespace Utilities
{
std::wstring getRegisterKeyStringValue(HKEY hKey,
                                       const std::wstring& subKey = L"",
                                       const std::wstring& valueName = L"");

bool isWindows11();

// Security and Identity
void resetAcl();

// Paths
const std::wstring GetMEGADesktopAppPath();
const std::wstring GetContextMenuPath();
const std::wstring GetExecutingModuleName();
void log(const std::wstring file, const std::wstring& message);
}

#endif // UTILITIES_H
