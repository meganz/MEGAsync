#ifndef CONTEXTMENUCOMMANDBASE_H
#define CONTEXTMENUCOMMANDBASE_H

#include "ContextMenuData.h"
#include "framework.h"

#include <windows.h>

class ContextMenuCommandBase: public winrt::implements<ContextMenuCommandBase, IExplorerCommand>
{
public:
    ContextMenuCommandBase(const std::wstring& id);

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
    void initializeContextMenuData(IShellItemArray* psiItemArray);
    virtual EXPCMDSTATE GetState(IShellItemArray* psiItemArray) = 0;
    virtual std::wstring GetIcon() const = 0;

    std::wstring mId;
    static ContextMenuData mContextMenuData;
};
#endif
