#include <ShObjIdl.h>
#include <Windows.h>
#include <windows.ui.notifications.h>
#include <WinUser.h>

#include <functional>

#ifndef WINTOASTCOMPAT_H
#define WINTOASTCOMPAT_H

template<class T>
class ComPtr
{
private:
    T* mPtr;

public:
    ComPtr()
    {
        mPtr = NULL;
    }

    ComPtr(T* ptr)
    {
        if (ptr != NULL)
        {
            mPtr = ptr;
            mPtr->AddRef();
        }
        else
        {
            mPtr = NULL;
        }
    }

    ComPtr(const ComPtr<T>& comPtr)
    {
        mPtr = (T*)comPtr;
        if (mPtr)
        {
            mPtr->AddRef();
        }
    }

    virtual ~ComPtr()
    {
        if (mPtr)
        {
            mPtr->Release();
        }
    }

    operator T*() const
    {
        return mPtr;
    }

    T& operator*() const
    {
        return *mPtr;
    }

    T** operator&()
    {
        if (mPtr)
        {
            mPtr->Release();
            mPtr = NULL;
        }
        return &mPtr;
    }

    T* operator->() const
    {
        return mPtr;
    }

    T* operator=(T* ptr)
    {
        if (ptr)
        {
            ptr->AddRef();
        }
        if (mPtr)
        {
            mPtr->Release();
        }
        mPtr = ptr;
        return mPtr;
    }

    T* operator=(const ComPtr<T>& comPtr)
    {
        T* tmp = mPtr;
        mPtr = (T*)comPtr;

        if (mPtr)
        {
            mPtr->AddRef();
        }
        if (tmp)
        {
            tmp->Release();
        }
        return mPtr;
    }

    T* Get()
    {
        return mPtr;
    }

    template <class TARGET>
    HRESULT As(TARGET **target)
    {
        if (!target)
        {
            return E_POINTER;
        }

        if (*target)
        {
            (*target)->Release();
        }

        if (mPtr)
        {
            return mPtr->QueryInterface(IID_PPV_ARGS(target));
        }
        return E_NOINTERFACE;
    }
};


template <typename T, typename I>
class MegaCompatToastCallback : public ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification*, T*>
{
public:
    LONG m_cRef;
    const std::function<HRESULT(ABI::Windows::UI::Notifications::IToastNotification*, I*)> f;

    MegaCompatToastCallback(const std::function<HRESULT(ABI::Windows::UI::Notifications::IToastNotification*, I*)> func) : f(func)
    {
        m_cRef = 0;
    }

    virtual ~MegaCompatToastCallback()
    {
        m_cRef = 0;
    }

    HRESULT __stdcall QueryInterface(REFIID riid, PVOID *ppv)
    {
        if (ppv == NULL)
        {
            return E_POINTER;
        }

        if (riid == __uuidof (ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification*, T*>))
        {
            *ppv = (ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification*, T*> *) this;
        }
        else if (riid == IID_IUnknown)
        {
            *ppv = (IUnknown *) this;
        }
        else
        {
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    ULONG __stdcall AddRef()
    {
        InterlockedIncrement(&m_cRef);
        return m_cRef;
    }

    ULONG __stdcall Release()
    {
        ULONG ulRefCount = InterlockedDecrement(&m_cRef);
        if (m_cRef == 0)
        {
            delete this;
        }
        return ulRefCount;
    }

    HRESULT __stdcall Invoke(ABI::Windows::UI::Notifications::IToastNotification *toast, I*inspectable)
    {
        return f(toast, inspectable);
    }
};

template <typename T, typename I>
class MegaCompatToastCallbackWrapper : public ComPtr<MegaCompatToastCallback<T, I> >
{
public:
    MegaCompatToastCallbackWrapper(const std::function<HRESULT(ABI::Windows::UI::Notifications::IToastNotification*, I*)> f)
        : ComPtr<MegaCompatToastCallback<T, I> > (new MegaCompatToastCallback<T, I>(f)) { }

    virtual ~MegaCompatToastCallbackWrapper() { }
};

#endif // WINTOASTCOMPAT_H
