#ifndef SHELLEXTNOTASYNC_H
#define SHELLEXTNOTASYNC_H

#include <windows.h>
#include <shlobj.h>
#include "ShellExt.h"

class ShellExtNotASync : public ShellExt
{
public:
    STDMETHOD(GetOverlayInfo)(LPWSTR pwszIconFile, int cchMax, int *pIndex, DWORD* pdwFlags) override;
    STDMETHOD(IsMemberOf)(LPCWSTR pwszPath,DWORD dwAttrib) override;

    ShellExtNotASync();
};

#endif
