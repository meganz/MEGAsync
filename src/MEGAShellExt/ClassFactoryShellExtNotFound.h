#ifndef CLASSFACTORYSHELLEXTNOTFOUND_H
#define CLASSFACTORYSHELLEXTNOTFOUND_H

#include "classfactory.h"

class ClassFactoryShellExtNotFound :
    public ClassFactory
{
public:
    IFACEMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv);
};

#endif
