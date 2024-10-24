#pragma once
#include "ContextMenuCommandBase.h"

class __declspec(uuid("9172EFEE-604B-492A-B2B3-9CC6164F52CC")) ContextMenuCommandView:
    public ContextMenuCommandBase
{
public:
    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;

protected:
    virtual const EXPCMDSTATE GetState(IShellItemArray* psiItemArray) override;
};
