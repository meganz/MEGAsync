#ifndef SHELLEXT_H
#define SHELLEXT_H

#include <windows.h>
#include <shlobj.h>

class ShellExt : public IShellIconOverlayIdentifier
{
public:
	// IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    STDMETHOD(GetOverlayInfo)(LPWSTR pwszIconFile, int cchMax, int *pIndex, DWORD* pdwFlags);
    STDMETHOD(GetPriority)(int* pPriority);
    STDMETHOD(IsMemberOf)(LPCWSTR pwszPath,DWORD dwAttrib);

	ShellExt(int id);
	~ShellExt(void);

private:
	// Reference count of component.
    long m_cRef;
	int id;
    TCHAR szModule[MAX_PATH];
};

#endif
