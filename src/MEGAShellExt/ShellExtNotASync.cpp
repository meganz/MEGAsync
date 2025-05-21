#include "ShellExtNotASync.h"

#include "resource.h"
#include <strsafe.h>
#include <Shlwapi.h>
#include <shellapi.h>

#include "MegaInterface.h"

#pragma comment(lib, "shlwapi.lib")

extern HINSTANCE g_hInst;
extern long g_cDllRef;

ShellExtNotASync::ShellExtNotASync():
    ShellExt(0)
{

}

HRESULT ShellExtNotASync::GetOverlayInfo(PWSTR, int, int*, DWORD*)
{
    return S_OK;
}

HRESULT ShellExtNotASync::IsMemberOf(PCWSTR pwszPath, DWORD dwAttrib)
{
    __try
    {
        (void)dwAttrib;

        MegaInterface::FileState state = MegaInterface::getPathState(pwszPath);

        if (state >= MegaInterface::FILE_NOTFOUND_NON_SYNCABLE)
        {
            SHFILEINFOW sfi = {0};
            DWORD_PTR hr = SHGetFileInfo(pwszPath,
                                       -1,
                                       &sfi,
                                       sizeof(sfi),
                                       SHGFI_ICONLOCATION);

            if (SUCCEEDED(hr))
            {
                wchar_t* pwc = wcsstr(sfi.szDisplayName, L"MEGAsync.exe");
                if (pwc != NULL)
                {
                    SHFOLDERCUSTOMSETTINGS fcs = {0};
                    fcs.dwSize = sizeof(SHFOLDERCUSTOMSETTINGS);
                    fcs.dwMask = FCSM_ICONFILE | FCSM_INFOTIP;
                    SHGetSetFolderCustomSettings(&fcs, pwszPath, FCS_FORCEWRITE);
                }
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return E_FAIL;
}
