#include "ContextMenuCommandUpload.h"

#include "MEGAinterface.h"
#include "SharedState.h"

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

    if (GetCmdState(psiItemArray) == ECS_ENABLED)
    {
        mContextMenuData.requestUpload();
    }

    return S_OK;
}

EXPCMDSTATE ContextMenuCommandUpload::GetCmdState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        mExpCmdState = ECS_ENABLED;
    }
    else
    {
        mState->SetState(mId, Set);

        initializeContextMenuData(psiItemArray);

        if (mContextMenuData.canRequestUpload())
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
