#ifndef CONTEXTMENUCOMMANDBASE_H
#define CONTEXTMENUCOMMANDBASE_H

#include "ContextMenuData.h"
#include "framework.h"

#include <windows.h>

class ContextMenuCommandBase: public winrt::implements<ContextMenuCommandBase, IExplorerCommand>
{
public:
    ContextMenuCommandBase(const std::wstring& id);

    void SetTitle(MegaInterface::StringID id, LPWSTR* ppszName);

    IFACEMETHODIMP GetCanonicalName(GUID* pguidCommandName) override;
    IFACEMETHODIMP GetFlags(EXPCMDFLAGS* flags) override;
    IFACEMETHODIMP GetIcon(IShellItemArray* psiItemArray, LPWSTR* ppszIcon) override;
    IFACEMETHODIMP GetState(IShellItemArray* psiItemArray,
                            BOOL fOkToBeSlow,
                            EXPCMDSTATE* pCmdState) override;
    IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand** ppEnum) override;

    std::wstring GetId() const
    {
        return mId;
    }

protected:
    // Rebuilds mContextMenuData from psiItemArray. Invoke() implementations
    // must call this with the array Explorer passes them: while building the
    // menu, Explorer hands GetState() at most 16 items of a larger selection,
    // so the data cached at that point can be a subset of what the user
    // actually selected (SNC-6885). Only Invoke() receives the full selection.
    void initializeContextMenuData(IShellItemArray* psiItemArray);
    virtual EXPCMDSTATE GetState(IShellItemArray* psiItemArray) = 0;
    virtual std::wstring GetIcon() const = 0;

    std::wstring mId;
    static ContextMenuData mContextMenuData;
};
#endif
