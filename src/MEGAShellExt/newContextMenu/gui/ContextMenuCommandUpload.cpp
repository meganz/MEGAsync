#include "ContextMenuCommandUpload.h"

#include "ContextMenuCommandGetLink.h"
#include "ContextMenuCommandView.h"
#include "MEGAinterface.h"
#include "SharedState.h"

ContextMenuCommandUpload::ContextMenuCommandUpload()
{
    {
        winrt::com_ptr<ContextMenuCommandGetLink> comPointer =
            winrt::make_self<ContextMenuCommandGetLink>();

        // Si necesitas un puntero crudo (raw pointer), puedes obtenerlo de esta manera:
        ContextMenuCommandGetLink* rawPointer = comPointer.get();

        mSubCommands.push_back(rawPointer);
    }

    {
        winrt::com_ptr<ContextMenuCommandView> comPointer =
            winrt::make_self<ContextMenuCommandView>();

        // Si necesitas un puntero crudo (raw pointer), puedes obtenerlo de esta manera:
        ContextMenuCommandView* rawPointer = comPointer.get();

        mSubCommands.push_back(rawPointer);
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
    // winrt::com_ptr<SubCommandEnumerator> comPointer = winrt::make_self<SubCommandEnumerator>();

    // SubCommandEnumerator* rawPointer = comPointer.get();
    // rawPointer->subCommands = mSubCommands;
    // *ppEnum = rawPointer;
    *ppEnum = nullptr;

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
