#include "ShellExt.h"
#include "resource.h"
#include <strsafe.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

extern HINSTANCE g_hInst;

ShellExt::ShellExt(int id)
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
	memcpy(pwszIconFile, szModule,  (lstrlen(szModule)+1)*sizeof(TCHAR));
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
   TCHAR chReadBuf[2]; 
   BOOL fSuccess; 
   DWORD cbRead; 
   LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\MEGApipe"); 
   TCHAR  lpszWrite[MAX_PATH+2];

   HRESULT hr = StringCchPrintf(lpszWrite, sizeof lpszWrite, L"P:%s", pwszPath);
   if (!SUCCEEDED(hr)) return S_FALSE;

   fSuccess = CallNamedPipe( 
      lpszPipename,							// pipe name 
      lpszWrite,							// message to server 
      (lstrlen(lpszWrite)+1)*sizeof(TCHAR), // message length 
      chReadBuf,							// buffer to receive reply 
      sizeof(chReadBuf),					// size of read buffer 
      &cbRead,								// number of bytes read 
      NMPWAIT_NOWAIT);						// waits
 
   if (fSuccess && cbRead>sizeof(TCHAR) && ((int)(chReadBuf[0]-L'0') == id)) 
		return S_OK;
	return S_FALSE;
}
