#ifndef CONTEXTMENUCOMMANDGETLINK_H
#define CONTEXTMENUCOMMANDGETLINK_H

#include "ContextMenuCommandBase.h"

class __declspec(uuid("0313D546-594C-49D6-AF85-F53575420E6C")) ContextMenuCommandGetLink:
    public ContextMenuCommandBase
{
public:
    ContextMenuCommandGetLink();
    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;

protected:
    EXPCMDSTATE GetState(IShellItemArray* psiItemArray) override;
};

#endif
