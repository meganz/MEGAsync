#ifndef SIMPLEFACTORY_H
#define SIMPLEFACTORY_H

#include "framework.h"

template<class T>
struct SimpleFactory: winrt::implements<SimpleFactory<T>, IClassFactory>
{
    IFACEMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override
    try
    {
        *ppvObject = nullptr;

        if (!pUnkOuter)
        {
            return winrt::make<T>().as(riid, ppvObject);
        }
        else
        {
            return CLASS_E_NOAGGREGATION;
        }
    }

    catch (...)
    {
        return winrt::to_hresult();
    }

    IFACEMETHODIMP LockServer(BOOL fLock) noexcept override
    {
        // COM pins/unpins the server through LockServer; count it on the WinRT
        // module lock so DllCanUnloadNow honours the pin.
        if (fLock)
        {
            ++winrt::get_module_lock();
        }
        else
        {
            --winrt::get_module_lock();
        }

        return S_OK;
    }
};

#endif
