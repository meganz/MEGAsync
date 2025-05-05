#include "ContextMenuCommand.h"

static const std::wstring ICON = L"mega.ico";
extern winrt::com_ptr<SubCommandEnumerator> g_enumCommands;

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
    if (g_enumCommands)
    {
        *ppEnum = g_enumCommands.get();
    }
    else
    {
        *ppEnum = nullptr;
    }

    return S_OK;
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

        return ECS_ENABLED;
    }

    return ECS_HIDDEN;
}

std::wstring ContextMenuCommand::GetIcon() const
{
    return ICON;
}
