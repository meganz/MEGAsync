#include "ContextMenuCommand.h"

static const std::wstring ICON = L"mega.ico";

ContextMenuCommand::ContextMenuCommand():
    ContextMenuCommandBase(L"ContextMenuCommand")
{}

IFACEMETHODIMP ContextMenuCommand::GetFlags(EXPCMDFLAGS* flags)
{
    *flags = ECF_HASSUBCOMMANDS;

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommand::GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName)
{
    static const std::wstring title = L"MEGA";

    SHStrDup(title.data(), ppszName);

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommand::GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip)
{
    return GetTitle(psiItemArray, ppszInfotip);
}

IFACEMETHODIMP ContextMenuCommand::Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept
{
    return S_OK;
}

HRESULT ContextMenuCommand::EnumSubCommands(IEnumExplorerCommand** ppEnum)
{
    *ppEnum = nullptr;

    try
    {
        auto e = winrt::make<SubCommandEnumerator>();
        return e->QueryInterface(IID_PPV_ARGS(ppEnum));
    }
    catch (...)
    {
        return winrt::to_hresult();
    }
}

EXPCMDSTATE ContextMenuCommand::GetState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        return ECS_HIDDEN;
    }

    if (mContextMenuData.isMEGASyncOpen())
    {
        initializeContextMenuData(psiItemArray);

        if (mContextMenuData.hasAnyOptionAvailable())
        {
            return ECS_ENABLED;
        }
    }

    return ECS_HIDDEN;
}

std::wstring ContextMenuCommand::GetIcon() const
{
    return ICON;
}
