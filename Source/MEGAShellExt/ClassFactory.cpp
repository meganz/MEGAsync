#include "ClassFactory.h"

#include <new>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

extern long g_cDllRef;

ClassFactory::ClassFactory() : m_cRef(1)
{
    InterlockedIncrement(&g_cDllRef);
}

ClassFactory::~ClassFactory()
{
    InterlockedDecrement(&g_cDllRef);
}


//
// IUnknown
//
IFACEMETHODIMP ClassFactory::QueryInterface(REFIID riid, void **ppv)
{
    __try
    {
        if(ppv == NULL)
            return E_POINTER;

        *ppv = NULL;
        if (riid == __uuidof (IClassFactory))
            *ppv = (IClassFactory *) this;
        else if (riid == IID_IUnknown)
            *ppv = (IUnknown *) this;
        else
            return E_NOINTERFACE;

        AddRef();
        return S_OK;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return E_NOINTERFACE;
}

IFACEMETHODIMP_(ULONG) ClassFactory::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

IFACEMETHODIMP_(ULONG) ClassFactory::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}


//
// IClassFactory
//
IFACEMETHODIMP ClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        InterlockedIncrement(&g_cDllRef);
    }
    else
    {
        InterlockedDecrement(&g_cDllRef);
    }
    return S_OK;
}
