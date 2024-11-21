#ifndef CONTEXTMENUCOMMANDBASE_H
#define CONTEXTMENUCOMMANDBASE_H

#include "../data/ContextMenuData.h"
#include "../utilities/SharedState.h"

class ContextMenuCommandBase: public winrt::implements<ContextMenuCommandBase, IExplorerCommand>
{
public:
    ContextMenuCommandBase();

    IFACEMETHODIMP GetCanonicalName(GUID* pguidCommandName) override;
    IFACEMETHODIMP GetFlags(EXPCMDFLAGS* flags) override;
    IFACEMETHODIMP GetIcon(IShellItemArray* psiItemArray, LPWSTR* ppszIcon) override;
    IFACEMETHODIMP GetState(IShellItemArray* psiItemArray,
                            BOOL fOkToBeSlow,
                            EXPCMDSTATE* pCmdState) override;
    IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand** ppEnum) override;

protected:
    void initializeContextMenuData(IShellItemArray* psiItemArray);
    virtual const EXPCMDSTATE GetState(IShellItemArray* psiItemArray) = 0;

protected:
    std::unique_ptr<SharedState> mState;
    static ContextMenuData mContextMenuData;
};

#endif
