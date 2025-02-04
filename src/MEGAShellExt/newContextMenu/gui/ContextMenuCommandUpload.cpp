#include "ContextMenuCommandUpload.h"

#include "ContextMenuCommandGetLink.h"
#include "ContextMenuCommandView.h"
#include "MEGAinterface.h"
#include "SharedState.h"

ContextMenuCommandUpload::ContextMenuCommandUpload()
{
    mEnumCommands = winrt::make_self<SubCommandEnumerator>();

    {
        winrt::com_ptr<ContextMenuCommandGetLink> comPointer =
            winrt::make_self<ContextMenuCommandGetLink>();

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }

    {
        winrt::com_ptr<ContextMenuCommandView> comPointer =
            winrt::make_self<ContextMenuCommandView>();

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }
}

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

HRESULT ContextMenuCommandUpload::EnumSubCommands(IEnumExplorerCommand** ppEnum)
{
    if (mEnumCommands)
    {
        *ppEnum = mEnumCommands.get();
    }
    else
    {
        *ppEnum = nullptr;
    }

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
