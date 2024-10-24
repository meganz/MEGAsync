#include "ContextMenuCommandViewVersions.h"

#include "../../MEGAinterface.h"
#include "SharedState.h"

IFACEMETHODIMP ContextMenuCommandViewVersions::GetTitle(IShellItemArray* psiItemArray,
                                                        LPWSTR* ppszName)
{
    wstring title;
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

    if (GetState(psiItemArray) == ECS_ENABLED)
        mContextMenuData.viewVersions();

    return S_OK;
}

const EXPCMDSTATE ContextMenuCommandViewVersions::GetState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
        return ECS_HIDDEN;

    mState->SetState(L"ContextMenuCommandViewVersions", Set);

    initializeContextMenuData(psiItemArray);
    if (mContextMenuData.canViewVersions())
        return ECS_ENABLED;

    return ECS_HIDDEN;
}
