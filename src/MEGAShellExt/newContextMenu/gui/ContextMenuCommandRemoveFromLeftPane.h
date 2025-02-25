#ifndef CONTEXTMENUCOMMANDREMOVEFROMLEFTPANE_H
#define CONTEXTMENUCOMMANDREMOVEFROMLEFTPANE_H

#include "ContextMenuCommandBase.h"

class __declspec(uuid("7FBA45D9-8232-4EA6-A61B-717C1F1819FE")) ContextMenuCommandRemoveFromLeftPane:
    public ContextMenuCommandBase
{
public:
    ContextMenuCommandRemoveFromLeftPane();
    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;

protected:
    EXPCMDSTATE GetState(IShellItemArray* psiItemArray) override;
};

#endif
