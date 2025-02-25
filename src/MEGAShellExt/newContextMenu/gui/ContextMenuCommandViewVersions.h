#ifndef CONTEXTMENUCOMMANDVIEWVERSIONS_H
#define CONTEXTMENUCOMMANDVIEWVERSIONS_H

#include "ContextMenuCommandBase.h"

class __declspec(uuid("D13F4639-DCFF-4866-98B8-3FA95388E5E1")) ContextMenuCommandViewVersions:
    public ContextMenuCommandBase
{
public:
    ContextMenuCommandViewVersions(bool isSubCommand = false);
    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;
    EXPCMDSTATE GetCmdState(IShellItemArray* psiItemArray) override;
};

#endif
