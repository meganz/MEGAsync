#include "PathHelper.h"

#include <filesystem>
using namespace std::filesystem;

extern HMODULE g_hInst;

const path GetModulePath()
{
    wchar_t pathBuffer[MAX_PATH] = {0};
    GetModuleFileNameW(g_hInst, pathBuffer, MAX_PATH);
    return path(pathBuffer);
}

const wstring GetApplicationPath()
{
    path modulePath = GetModulePath();
    return modulePath.parent_path().parent_path().wstring();
}

const wstring GetContextMenuPath()
{
    path modulePath = GetModulePath();
    return modulePath.parent_path().wstring();
}

const wstring GetExecutingModuleName()
{
    wchar_t pathBuffer[FILENAME_MAX] = {0};
    GetModuleFileNameW(NULL, pathBuffer, FILENAME_MAX);
    PathStripPathW(pathBuffer);

    wstring moduleName(pathBuffer);
    transform(moduleName.begin(), moduleName.end(), moduleName.begin(), towlower);

    return moduleName;
}
