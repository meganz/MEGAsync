#include "ContextMenuCommandGetLink.h"

#include "MEGAinterface.h"
#include "SharedState.h"

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

    if (GetCmdState(psiItemArray) == ECS_ENABLED)
    {
        mContextMenuData.requestGetLinks();
    }

    return S_OK;
}

EXPCMDSTATE ContextMenuCommandGetLink::GetCmdState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        mExpCmdState = ECS_HIDDEN;
    }
    else
    {
        mState->SetState(mId, Set);

        initializeContextMenuData(psiItemArray);
        if (mContextMenuData.canRequestGetLinks())
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
