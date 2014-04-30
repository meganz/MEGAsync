#include "ShellExt.h"
#include "resource.h"
#include <strsafe.h>
#include <Shlwapi.h>

#include "MegaInterface.h"

#pragma comment(lib, "shlwapi.lib")

extern HINSTANCE g_hInst;

ShellExt::ShellExt(int id) : m_cRef(1)
{
	this->id = id;
    if (!g_hInst || !GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)))
		szModule[0]=L'\0';
}


ShellExt::~ShellExt()
{
}

// Query to the interface the component supported.
IFACEMETHODIMP ShellExt::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(ShellExt, IShellIconOverlayIdentifier),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) ShellExt::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) ShellExt::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }

    return cRef;
}

HRESULT ShellExt::GetOverlayInfo(PWSTR pwszIconFile, int cchMax, int *pIndex, DWORD *pdwFlags)
{
    __try
    {
        if(pwszIconFile == NULL)
            return E_POINTER;
        if(pIndex == NULL)
            return E_POINTER;
        if(pdwFlags == NULL)
            return E_POINTER;
        if(cchMax < 1)
            return E_INVALIDARG;

        int len = lstrlen(szModule)+1;
        if (len > cchMax)
            return E_FAIL;

        memcpy(pwszIconFile, szModule, len*sizeof(wchar_t));
        *pIndex = id;
        *pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;
        return S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return E_FAIL;
}

HRESULT ShellExt::GetPriority(int *pPriority)
{
    __try
    {
        if (pPriority == 0)
            return E_POINTER;

        *pPriority = 0;
        return S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return E_FAIL;
}

HRESULT ShellExt::IsMemberOf(PCWSTR pwszPath, DWORD dwAttrib)
{
    __try
    {
        (void)dwAttrib;
        if(MegaInterface::getPathState(pwszPath) == id)
            return S_OK;
        return S_FALSE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return E_FAIL;
}
