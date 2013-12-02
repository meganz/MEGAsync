#ifndef CLASSFACTORY_H
#define CLASSFACTORY_H

#include <unknwn.h>     // For IClassFactory
#include <windows.h>

class ClassFactory : public IClassFactory
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IClassFactory
    virtual IFACEMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv) = 0;
    IFACEMETHODIMP LockServer(BOOL fLock);

    ClassFactory();

protected:
    virtual ~ClassFactory();

private:
    long m_cRef;
};

#endif
