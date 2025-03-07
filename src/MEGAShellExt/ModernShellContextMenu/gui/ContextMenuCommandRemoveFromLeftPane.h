#ifndef CONTEXTMENUCOMMANDREMOVEFROMLEFTPANE_H
#define CONTEXTMENUCOMMANDREMOVEFROMLEFTPANE_H

#include "ContextMenuCommandBase.h"

class ContextMenuCommandRemoveFromLeftPane: public ContextMenuCommandBase
{
public:
    ContextMenuCommandRemoveFromLeftPane();
    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;

protected:
    EXPCMDSTATE GetState(IShellItemArray* psiItemArray) override;
    std::wstring GetIcon() const override;
};

#endif
