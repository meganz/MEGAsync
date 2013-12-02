#ifndef CLASSFACTORYSHELLEXTSYNCING_H
#define CLASSFACTORYSHELLEXTSYNCING_H

#include "classfactory.h"
class ClassFactoryShellExtSyncing :
	public ClassFactory
{
public:
    IFACEMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv);
};

#endif
