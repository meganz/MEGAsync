#ifndef CONTEXT_MENU_UNIQUE_COMMAND_H
#define CONTEXT_MENU_UNIQUE_COMMAND_H

#include "ContextMenuCommandBase.h"

class __declspec(uuid("AAA3E837-B080-4BB1-A508-632CC3E75AAA")) ContextMenuUniqueCommand:
    public ContextMenuCommandBase
{
public:
    ContextMenuUniqueCommand();

    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;
    IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand** ppEnum) override;
    IFACEMETHODIMP GetFlags(EXPCMDFLAGS* flags) override;

    EXPCMDSTATE GetCmdState(IShellItemArray* psiItemArray) override;

private:
    winrt::com_ptr<SubCommandEnumerator> mEnumCommands;
};

#endif
