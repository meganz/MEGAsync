#pragma once
#include "../data/ContextMenuData.h"
#include "SharedCounter.h"
#include "SharedState.h"

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
    unique_ptr<SharedCounter> mCounter;
    unique_ptr<SharedState> mState;
    static ContextMenuData mContextMenuData;
};
