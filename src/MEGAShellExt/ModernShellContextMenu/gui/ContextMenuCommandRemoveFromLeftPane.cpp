#include "ContextMenuCommandRemoveFromLeftPane.h"

#include "MEGAinterface.h"

static const std::wstring ICON = L"removeleftpane.ico";

ContextMenuCommandRemoveFromLeftPane::ContextMenuCommandRemoveFromLeftPane():
    ContextMenuCommandBase(L"ContextMenuCommandRemoveFromLeftPane")
{}

IFACEMETHODIMP ContextMenuCommandRemoveFromLeftPane::GetTitle(IShellItemArray*, LPWSTR* ppszName)
{
    SetTitle(MegaInterface::STRING_REMOVE_FROM_LEFT_PANE, ppszName);

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

    if (GetState(psiItemArray) == ECS_ENABLED)
    {
        mContextMenuData.removeFromLeftPane();
    }

    return S_OK;
}

EXPCMDSTATE ContextMenuCommandRemoveFromLeftPane::GetState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        return ECS_HIDDEN;
    }

    if (mContextMenuData.canRemoveFromLeftPane())
    {
        return ECS_ENABLED;
    }

    return ECS_HIDDEN;
}

std::wstring ContextMenuCommandRemoveFromLeftPane::GetIcon() const
{
    return ICON;
}
