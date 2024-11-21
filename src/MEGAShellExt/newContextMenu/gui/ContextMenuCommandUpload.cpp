#include "ContextMenuCommandUpload.h"

#include "MEGAinterface.h"
#include "SharedState.h"

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

    if (GetState(psiItemArray) == ECS_ENABLED)
        mContextMenuData.requestUpload();

    return S_OK;
}

const EXPCMDSTATE ContextMenuCommandUpload::GetState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
        return ECS_HIDDEN;

    mState->SetState(L"ContextMenuCommandUpload", Set);

    initializeContextMenuData(psiItemArray);
    if (mContextMenuData.canRequestUpload())
    {
        return ECS_ENABLED;
    }

    return ECS_HIDDEN;
}
