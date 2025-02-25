#ifndef CONTEXTMENUCOMMAND_H
#define CONTEXTMENUCOMMAND_H

#include "ContextMenuCommandBase.h"

class __declspec(uuid("F9E3E837-B080-4BB1-A508-632CC3E751B7")) ContextMenuCommand:
    public ContextMenuCommandBase
{
public:
    ContextMenuCommand();

    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;
    IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand** ppEnum) override;
    IFACEMETHODIMP GetFlags(EXPCMDFLAGS* flags) override;

protected:
    EXPCMDSTATE GetState(IShellItemArray* psiItemArray) override;

private:
    winrt::com_ptr<SubCommandEnumerator> mEnumCommands;
};

#endif
