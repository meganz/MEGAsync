#include "ContextMenuCommandViewVersions.h"

#include "MEGAinterface.h"

static const std::wstring ICON = L"cmd5.ico";

ContextMenuCommandViewVersions::ContextMenuCommandViewVersions():
    ContextMenuCommandBase(L"ContextMenuCommandViewVersions")
{}

IFACEMETHODIMP ContextMenuCommandViewVersions::GetTitle(IShellItemArray* psiItemArray,
                                                        LPWSTR* ppszName)
{
    std::wstring title;
    int syncedFolders = mContextMenuData.getSyncedFolders();
    int syncedFiles = mContextMenuData.getSyncedFiles();
    LPWSTR menuText =
        MegaInterface::getString(MegaInterface::STRING_VIEW_VERSIONS, syncedFiles, syncedFolders);
    if (menuText)
    {
        title = menuText;
        delete menuText;
    }

    SHStrDup(title.data(), ppszName);

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandViewVersions::GetToolTip(IShellItemArray* psiItemArray,
                                                          LPWSTR* ppszInfotip)
{
    return GetTitle(psiItemArray, ppszInfotip);
}

IFACEMETHODIMP ContextMenuCommandViewVersions::Invoke(IShellItemArray* psiItemArray,
                                                      IBindCtx* pbc) noexcept
{
    UNREFERENCED_PARAMETER(pbc);

    mContextMenuData.viewVersions();

    return S_OK;
}

EXPCMDSTATE ContextMenuCommandViewVersions::GetState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        return ECS_HIDDEN;
    }

    if (mContextMenuData.canViewVersions())
    {
        return ECS_ENABLED;
    }

    return ECS_HIDDEN;
}

std::wstring ContextMenuCommandViewVersions::GetIcon() const
{
    return ICON;
}
