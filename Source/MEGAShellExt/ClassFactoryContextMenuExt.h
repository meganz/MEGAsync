#ifndef CLASSFACTORYCONTEXTMENU_H
#define CLASSFACTORYCONTEXTMENU_H

#include "ClassFactory.h"
#include <unknwn.h>     // For IClassFactory
#include <windows.h>

class ClassFactoryContextMenuExt : public ClassFactory
{
public:
    IFACEMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv);
};

#endif