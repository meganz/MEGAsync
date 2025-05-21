#include "ContextMenuCommandSyncBackup.h"

#include "MEGAinterface.h"

static const std::wstring ICON_SYNC = L"Create sync.ico";
static const std::wstring ICON_BACKUP = L"Create backup.ico";

ContextMenuCommandSyncBackup::ContextMenuCommandSyncBackup(MegaInterface::SyncType type):
    mType(type),
    ContextMenuCommandBase(L"ContextMenuCommandSyncBackup")
{}

IFACEMETHODIMP ContextMenuCommandSyncBackup::GetTitle(IShellItemArray*, LPWSTR* ppszName)
{
    SetTitle(mType == MegaInterface::SyncType::TYPE_TWOWAY ? MegaInterface::STRING_SYNC :
                                                             MegaInterface::STRING_BACKUP,
             ppszName);

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandSyncBackup::GetToolTip(IShellItemArray* psiItemArray,
                                                        LPWSTR* ppszInfotip)
{
    return GetTitle(psiItemArray, ppszInfotip);
}

IFACEMETHODIMP ContextMenuCommandSyncBackup::Invoke(IShellItemArray* psiItemArray,
                                                    IBindCtx* pbc) noexcept
{
    UNREFERENCED_PARAMETER(pbc);

    mContextMenuData.requestSync(mType);

    return S_OK;
}

EXPCMDSTATE ContextMenuCommandSyncBackup::GetState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        return ECS_HIDDEN;
    }

    if (mContextMenuData.canSync(mType))
    {
        return ECS_ENABLED;
    }

    return ECS_HIDDEN;
}

std::wstring ContextMenuCommandSyncBackup::GetIcon() const
{
    switch (mType)
    {
        case MegaInterface::SyncType::TYPE_BACKUP:
        {
            return ICON_BACKUP;
        }
        case MegaInterface::SyncType::TYPE_TWOWAY:
        {
            return ICON_SYNC;
        }
    }

    return std::wstring();
}
