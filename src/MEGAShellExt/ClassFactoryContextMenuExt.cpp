#include "ClassFactoryContextMenuExt.h"
#include "ContextMenuExt.h"

#include <new>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

IFACEMETHODIMP ClassFactoryContextMenuExt::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
    HRESULT hr = CLASS_E_NOAGGREGATION;

    if (ppv == NULL)
    {
        return E_POINTER;
    }

    *ppv = NULL;
    if (pUnkOuter == NULL)
    {
        hr = E_OUTOFMEMORY;

        // Create the COM component.
        ContextMenuExt *pExt = new (std::nothrow) ContextMenuExt();
        if (pExt)
        {
            // Query the specified interface.
            hr = pExt->QueryInterface(riid, ppv);
            pExt->Release();
        }
    }

    return hr;
}
