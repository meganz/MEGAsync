#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#pragma warning(disable: 4324)

#define _SILENCE_CLANG_COROUTINE_MESSAGE
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

// Windows Header Files
#include <Unknwn.h>

#include <aclapi.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shobjidl_core.h>
#include <strsafe.h>
#include <windows.h>

// WinRT Header Files
#include <winrt/base.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Management.Deployment.h>

// Link libraries
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "runtimeobject.lib")

#endif
