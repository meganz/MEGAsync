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
	if (!GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule))) 
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
    int len = lstrlen(szModule)+1;
    if (len > cchMax)
        return S_FALSE;

    memcpy(pwszIconFile, szModule, len*sizeof(TCHAR));
    *pIndex = id;
    *pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;
    return S_OK;
}

HRESULT ShellExt::GetPriority(int *pPriority)
{
	*pPriority = 0;
    return S_OK;
}

HRESULT ShellExt::IsMemberOf(PCWSTR pwszPath, DWORD dwAttrib)
{
   (void)dwAttrib;
   if(MegaInterface::getPathState(pwszPath) == id)
		return S_OK;
	return S_FALSE;
}
