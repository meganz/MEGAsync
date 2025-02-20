#include "ContextMenuCommandRemoveFromLeftPane.h"

#include "MEGAinterface.h"
#include "SharedState.h"

ContextMenuCommandRemoveFromLeftPane::ContextMenuCommandRemoveFromLeftPane():
    ContextMenuCommandBase(L"ContextMenuCommandRemoveFromLeftPane")
{}

IFACEMETHODIMP ContextMenuCommandRemoveFromLeftPane::GetTitle(IShellItemArray* psiItemArray,
                                                              LPWSTR* ppszName)
{
    std::wstring title;
    LPWSTR menuText = MegaInterface::getString(MegaInterface::STRING_REMOVE_FROM_LEFT_PANE, 0, 0);
    if (menuText)
    {
        title = menuText;
        delete menuText;
    }

    SHStrDup(title.data(), ppszName);

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandRemoveFromLeftPane::GetToolTip(IShellItemArray* psiItemArray,
                                                                LPWSTR* ppszInfotip)
{
    return GetTitle(psiItemArray, ppszInfotip);
}

IFACEMETHODIMP ContextMenuCommandRemoveFromLeftPane::Invoke(IShellItemArray* psiItemArray,
                                                            IBindCtx* pbc) noexcept
{
    UNREFERENCED_PARAMETER(pbc);

    if (GetCmdState(psiItemArray) == ECS_ENABLED)
    {
        mContextMenuData.removeFromLeftPane();
    }

    return S_OK;
}

EXPCMDSTATE ContextMenuCommandRemoveFromLeftPane::GetCmdState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        mExpCmdState = ECS_HIDDEN;
    }
    else
    {
        mState->SetState(mId, Set);

        initializeContextMenuData(psiItemArray);

        if (mContextMenuData.canRemoveFromLeftPane())
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
