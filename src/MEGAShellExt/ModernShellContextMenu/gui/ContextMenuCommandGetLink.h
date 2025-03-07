#ifndef CONTEXTMENUCOMMANDGETLINK_H
#define CONTEXTMENUCOMMANDGETLINK_H

#include "ContextMenuCommandBase.h"

class ContextMenuCommandGetLink: public ContextMenuCommandBase
{
public:
    ContextMenuCommandGetLink();
    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;

protected:
    EXPCMDSTATE GetState(IShellItemArray* psiItemArray) override;
    std::wstring GetIcon() const override;
};

#endif
