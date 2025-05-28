#include "ContextMenuCommandGetLink.h"

#include "MEGAinterface.h"

static const std::wstring ICON = L"getlink.ico";

ContextMenuCommandGetLink::ContextMenuCommandGetLink():
    ContextMenuCommandBase(L"ContextMenuCommandGetLink")
{}

IFACEMETHODIMP ContextMenuCommandGetLink::GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName)
{
    std::wstring title;
    int syncedFolders = mContextMenuData.getSyncedFolders();
    int syncedFiles = mContextMenuData.getSyncedFiles();
    LPWSTR menuText =
        MegaInterface::getString(MegaInterface::STRING_GETLINK, syncedFiles, syncedFolders);
    if (menuText)
    {
        title = menuText;
        delete menuText;
    }

    SHStrDup(title.data(), ppszName);

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandGetLink::GetToolTip(IShellItemArray* psiItemArray,
                                                     LPWSTR* ppszInfotip)
{
    UNREFERENCED_PARAMETER(psiItemArray);
    UNREFERENCED_PARAMETER(ppszInfotip);

    return GetTitle(psiItemArray, ppszInfotip);
}

IFACEMETHODIMP ContextMenuCommandGetLink::Invoke(IShellItemArray* psiItemArray,
                                                 IBindCtx* pbc) noexcept
{
    UNREFERENCED_PARAMETER(pbc);

    if (GetState(psiItemArray) == ECS_ENABLED)
    {
        mContextMenuData.requestGetLinks();
    }

    return S_OK;
}

EXPCMDSTATE ContextMenuCommandGetLink::GetState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        return ECS_HIDDEN;
    }

    if (mContextMenuData.canRequestGetLinks())
    {
        return ECS_ENABLED;
    }

    return ECS_HIDDEN;
}

std::wstring ContextMenuCommandGetLink::GetIcon() const
{
    return ICON;
}
