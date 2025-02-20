#include "ContextMenuCommandViewVersions.h"

#include "MEGAinterface.h"
#include "SharedState.h"

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

    if (GetCmdState(psiItemArray) == ECS_ENABLED)
    {
        mContextMenuData.viewVersions();
    }

    return S_OK;
}

EXPCMDSTATE ContextMenuCommandViewVersions::GetCmdState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        mExpCmdState = ECS_HIDDEN;
    }
    else
    {
        mState->SetState(mId, Set);

        initializeContextMenuData(psiItemArray);

        if (mContextMenuData.canViewVersions())
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
