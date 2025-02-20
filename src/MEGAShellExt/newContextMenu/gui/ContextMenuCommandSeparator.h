#ifndef CONTEXTMENUCOMMANDSEPARATOR_H
#define CONTEXTMENUCOMMANDSEPARATOR_H

#include "ContextMenuCommandBase.h"

class __declspec(uuid("F85CF690-330F-4EC2-89E5-3E7ED0344BF5")) ContextMenuCommandSeparator:
    public ContextMenuCommandBase
{
public:
    ContextMenuCommandSeparator();
    IFACEMETHODIMP GetFlags(EXPCMDFLAGS* flags) override;
    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;
    EXPCMDSTATE GetCmdState(IShellItemArray* psiItemArray) override;
};

#endif
