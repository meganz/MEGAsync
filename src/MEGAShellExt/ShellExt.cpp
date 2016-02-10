#include "ShellExt.h"
#include "resource.h"
#include <strsafe.h>
#include <Shlwapi.h>

#include "MegaInterface.h"

#pragma comment(lib, "shlwapi.lib")

extern HINSTANCE g_hInst;
extern long g_cDllRef;

ShellExt::ShellExt(int id) : m_cRef(1)
{
    __try
    {
        szModule[0]=L'\0';
        InterlockedIncrement(&g_cDllRef);

        this->id = id;
        if (g_hInst)
            GetModuleFileName(g_hInst, szModule, MAX_PATH);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
}


ShellExt::~ShellExt()
{
    __try
    {
        InterlockedDecrement(&g_cDllRef);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

// Query to the interface the component supported.
IFACEMETHODIMP ShellExt::QueryInterface(REFIID riid, void **ppv)
{
    __try
    {
        if (ppv == NULL)
        {
            return E_POINTER;
        }

        *ppv = NULL;
        if (riid == __uuidof (IShellIconOverlayIdentifier))
        {
            *ppv = (IShellIconOverlayIdentifier *) this;
        }
        else if (riid == IID_IUnknown)
        {
            *ppv = this;
        }
        else
        {
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return E_NOINTERFACE;
}

// Increase the reference count for an interface or an object.
IFACEMETHODIMP_(ULONG) ShellExt::AddRef()
{
    __try
    {
        return InterlockedIncrement(&m_cRef);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return ++m_cRef;
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) ShellExt::Release()
{
    __try
    {
        ULONG cRef = InterlockedDecrement(&m_cRef);
        if (0 == cRef)
        {
            delete this;
        }
        return cRef;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return 0;
}

HRESULT ShellExt::GetOverlayInfo(PWSTR pwszIconFile, int cchMax, int *pIndex, DWORD *pdwFlags)
{
    __try
    {
        if (pwszIconFile == NULL)
        {
            return E_POINTER;
        }
        if (pIndex == NULL)
        {
            return E_POINTER;
        }
        if (pdwFlags == NULL)
        {
            return E_POINTER;
        }
        if (cchMax < 1)
        {
            return E_INVALIDARG;
        }

        int len = lstrlen(szModule) + 1;
        if (!len || (len > cchMax))
        {
            return E_FAIL;
        }

        memcpy(pwszIconFile, szModule, len * sizeof(TCHAR));
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
        if (MegaInterface::getPathState(pwszPath) == id)
        {
            return S_OK;
        }
        return S_FALSE;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return E_FAIL;
}
