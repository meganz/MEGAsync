#include "ContextMenuCommandUpload.h"

#include "MEGAinterface.h"

static const std::wstring ICON = L"cmd3.ico";

ContextMenuCommandUpload::ContextMenuCommandUpload():
    ContextMenuCommandBase(L"ContextMenuCommandUpload")
{}

IFACEMETHODIMP ContextMenuCommandUpload::GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName)
{
    std::wstring title;

    int unsyncedFolders = mContextMenuData.getUnsyncedFolders();
    int unsyncedFiles = mContextMenuData.getUnsyncedFiles();
    LPWSTR menuText =
        MegaInterface::getString(MegaInterface::STRING_UPLOAD, unsyncedFiles, unsyncedFolders);
    if (menuText)
    {
        title = menuText;
        delete menuText;
    }

    SHStrDup(title.data(), ppszName);

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandUpload::GetToolTip(IShellItemArray* psiItemArray,
                                                    LPWSTR* ppszInfotip)
{
    return GetTitle(psiItemArray, ppszInfotip);
}

IFACEMETHODIMP ContextMenuCommandUpload::Invoke(IShellItemArray* psiItemArray,
                                                IBindCtx* pbc) noexcept
{
    UNREFERENCED_PARAMETER(pbc);

    mContextMenuData.requestUpload();

    return S_OK;
}

EXPCMDSTATE ContextMenuCommandUpload::GetState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        return ECS_HIDDEN;
    }

    if (mContextMenuData.canRequestUpload())
    {
        return ECS_ENABLED;
    }

    return ECS_HIDDEN;
}

std::wstring ContextMenuCommandUpload::GetIcon() const
{
    return ICON;
}
