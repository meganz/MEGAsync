#include "ContextMenuCommandSeparator.h"

#include "SharedState.h"

IFACEMETHODIMP ContextMenuCommandSeparator::GetFlags(EXPCMDFLAGS* flags)
{
    *flags = ECF_ISSEPARATOR;
    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandSeparator::GetTitle(IShellItemArray* psiItemArray,
                                                     LPWSTR* ppszName)
{
    UNREFERENCED_PARAMETER(psiItemArray);
    UNREFERENCED_PARAMETER(ppszName);

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandSeparator::GetToolTip(IShellItemArray* psiItemArray,
                                                       LPWSTR* ppszInfotip)
{
    return GetTitle(psiItemArray, ppszInfotip);
}

IFACEMETHODIMP ContextMenuCommandSeparator::Invoke(IShellItemArray* psiItemArray,
                                                   IBindCtx* pbc) noexcept
{
    UNREFERENCED_PARAMETER(psiItemArray);
    UNREFERENCED_PARAMETER(pbc);

    return S_OK;
}

const EXPCMDSTATE ContextMenuCommandSeparator::GetState(IShellItemArray* psiItemArray)
{
    UNREFERENCED_PARAMETER(psiItemArray);
    mState->SetState(L"ContextMenuCommandSeparator", Set);

    return ECS_ENABLED;
}
