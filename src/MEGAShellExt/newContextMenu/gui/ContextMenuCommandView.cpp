#include "ContextMenuCommandView.h"

#include "MEGAinterface.h"
#include "SharedState.h"

ContextMenuCommandView::ContextMenuCommandView():
    ContextMenuCommandBase(L"ContextMenuCommandView")
{}

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

    if (GetCmdState(psiItemArray) == ECS_ENABLED)
    {
        mContextMenuData.viewOnMEGA();
    }

    return S_OK;
}

EXPCMDSTATE ContextMenuCommandView::GetCmdState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        mExpCmdState = ECS_HIDDEN;
    }
    else
    {
        mState->SetState(mId, Set);

        initializeContextMenuData(psiItemArray);

        if (mContextMenuData.canViewOnMEGA())
        {
            mExpCmdState = ECS_ENABLED;
        }
        else
        {
            mExpCmdState = ECS_HIDDEN;
        }
    }

    return mExpCmdState;
}
