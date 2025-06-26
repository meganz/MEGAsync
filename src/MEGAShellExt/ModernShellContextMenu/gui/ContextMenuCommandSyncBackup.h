#ifndef CONTEXTMENUCOMMANDSYNCBACKUP_H
#define CONTEXTMENUCOMMANDSYNCBACKUP_H

#include "ContextMenuCommandBase.h"
#include "MEGAinterface.h"

class ContextMenuCommandSyncBackup: public ContextMenuCommandBase
{
public:
    ContextMenuCommandSyncBackup(MegaInterface::SyncType type);
    IFACEMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) override;
    IFACEMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) override;
    IFACEMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept override;

protected:
    EXPCMDSTATE GetState(IShellItemArray* psiItemArray) override;
    std::wstring GetIcon() const override;

private:
    MegaInterface::SyncType mType;
};

#endif
