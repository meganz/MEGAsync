#ifndef CONTEXTMENUCOMMANDUPLOAD_H
#define CONTEXTMENUCOMMANDUPLOAD_H

#include "ContextMenuCommandBase.h"

class ContextMenuCommandUpload: public ContextMenuCommandBase
{
public:
    ContextMenuCommandUpload();
    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;

protected:
    EXPCMDSTATE GetState(IShellItemArray* psiItemArray) override;
    std::wstring GetIcon() const override;
};

#endif
