#include "ContextMenuCommandView.h"

#include "MEGAinterface.h"

static const std::wstring ICON = L"view.ico";

ContextMenuCommandView::ContextMenuCommandView():
    ContextMenuCommandBase(L"ContextMenuCommandView")
{}

IFACEMETHODIMP ContextMenuCommandView::GetTitle(IShellItemArray*, LPWSTR* ppszName)
{
    SetTitle(MegaInterface::STRING_VIEW_ON_MEGA, ppszName);

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

    mContextMenuData.viewOnMEGA();

    return S_OK;
}

EXPCMDSTATE ContextMenuCommandView::GetState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        return ECS_HIDDEN;
    }

    if (mContextMenuData.canViewOnMEGA())
    {
        return ECS_ENABLED;
    }

    return ECS_HIDDEN;
}

std::wstring ContextMenuCommandView::GetIcon() const
{
    return ICON;
}
