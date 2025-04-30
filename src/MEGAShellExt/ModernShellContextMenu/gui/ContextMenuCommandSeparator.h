#ifndef CONTEXTMENUCOMMANDSEPARATOR_H
#define CONTEXTMENUCOMMANDSEPARATOR_H

#include "ContextMenuCommandBase.h"

class ContextMenuCommandSeparator: public ContextMenuCommandBase
{
public:
    ContextMenuCommandSeparator();
    IFACEMETHODIMP GetFlags(EXPCMDFLAGS* flags) override;
    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;

protected:
    EXPCMDSTATE GetState(IShellItemArray* psiItemArray) override;
    std::wstring GetIcon() const override;
};

#endif
