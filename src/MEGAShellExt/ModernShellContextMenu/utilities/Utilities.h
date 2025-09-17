// clang-format off
#ifndef UTILITIES_H
#define UTILITIES_H

#include <windows.h>
#include <xstring>

namespace Utilities
{
static const size_t MAX_LONG_PATH = 32768;

std::wstring getRegisterKeyStringValue(HKEY hKey,
                                       const std::wstring& subKey = L"",
                                       const std::wstring& valueName = L"");

void updateExplorer();

bool haveModernContextMenu();

// Security and Identity
void resetAcl();

// Paths
const std::wstring GetMEGADesktopAppPath();
const std::wstring GetContextMenuPath();
const std::wstring GetExecutingModuleName();
void log(const std::wstring& file, const std::wstring& message);

bool isDarkModeActive();

bool isContextMenuDisabled();
}

#endif // UTILITIES_H
