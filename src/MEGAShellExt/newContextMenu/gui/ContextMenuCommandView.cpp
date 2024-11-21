#include "ContextMenuCommandView.h"

#include "MEGAinterface.h"
#include "SharedState.h"

IFACEMETHODIMP ContextMenuCommandView::GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName)
{
    std::wstring title;
    int syncedFolders = mContextMenuData.getSyncedFolders();
    int syncedFiles = mContextMenuData.getSyncedFiles();
    LPWSTR menuText =
        MegaInterface::getString(MegaInterface::STRING_VIEW_ON_MEGA, syncedFiles, syncedFolders);
    if (menuText)
    {
        title = menuText;
        delete menuText;
    }

    SHStrDup(title.data(), ppszName);

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandView::GetToolTip(IShellItemArray* psiItemArray,
                                                  LPWSTR* ppszInfotip)
{
    return GetTitle(psiItemArray, ppszInfotip);
}

IFACEMETHODIMP ContextMenuCommandView::Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept
{
    UNREFERENCED_PARAMETER(pbc);

    if (GetState(psiItemArray) == ECS_ENABLED)
        mContextMenuData.viewOnMEGA();

    return S_OK;
}

const EXPCMDSTATE ContextMenuCommandView::GetState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
        return ECS_HIDDEN;

    mState->SetState(L"ContextMenuCommandView", Set);

    initializeContextMenuData(psiItemArray);
    if (mContextMenuData.canViewOnMEGA())
        return ECS_ENABLED;

    return ECS_HIDDEN;
}
