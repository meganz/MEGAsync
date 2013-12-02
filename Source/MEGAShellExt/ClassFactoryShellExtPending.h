#ifndef CLASSFACTORYSHELLEXTPENDING_H
#define CLASSFACTORYSHELLEXTPENDING_H

#include "classfactory.h"
class ClassFactoryShellExtPending :
	public ClassFactory
{
public:
    IFACEMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv);
};

#endif
