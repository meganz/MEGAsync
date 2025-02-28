#include "ContextMenuCommandRemoveFromLeftPane.h"

#include "MEGAinterface.h"

static const std::wstring ICON = L"cmd2.ico";

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
