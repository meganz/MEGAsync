#ifndef CONTEXTMENUCOMMANDUPLOAD_H
#define CONTEXTMENUCOMMANDUPLOAD_H

#include "ContextMenuCommandBase.h"

class __declspec(uuid("F043C633-970A-43DF-A382-9D51C8D14E97")) ContextMenuCommandUpload:
    public ContextMenuCommandBase
{
public:
    ContextMenuCommandUpload();

    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;
    IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand** ppEnum) override;

protected:
    virtual const EXPCMDSTATE GetState(IShellItemArray* psiItemArray) override;

private:
    winrt::com_ptr<SubCommandEnumerator> mEnumCommands;
};

#endif
