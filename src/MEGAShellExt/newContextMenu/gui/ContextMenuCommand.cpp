#include "ContextMenuCommand.h"

#include "ContextMenuCommandGetLink.h"
#include "ContextMenuCommandRemoveFromLeftPane.h"
#include "ContextMenuCommandSeparator.h"
#include "ContextMenuCommandUpload.h"
#include "ContextMenuCommandView.h"
#include "ContextMenuCommandViewVersions.h"

ContextMenuCommand::ContextMenuCommand():
    ContextMenuCommandBase(L"ContextMenuCommand")
{
    mEnumCommands = winrt::make_self<SubCommandEnumerator>();

    {
        winrt::com_ptr<ContextMenuCommandGetLink> comPointer =
            winrt::make_self<ContextMenuCommandGetLink>();

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }

    /*
    {
        winrt::com_ptr<ContextMenuCommandSeparator> comPointer =
            winrt::make_self<ContextMenuCommandSeparator>();

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }
*/

    {
        winrt::com_ptr<ContextMenuCommandView> comPointer =
            winrt::make_self<ContextMenuCommandView>();

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }

    {
        winrt::com_ptr<ContextMenuCommandViewVersions> comPointer =
            winrt::make_self<ContextMenuCommandViewVersions>();

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }

    {
        winrt::com_ptr<ContextMenuCommandUpload> comPointer =
            winrt::make_self<ContextMenuCommandUpload>();

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }

    {
        winrt::com_ptr<ContextMenuCommandRemoveFromLeftPane> comPointer =
            winrt::make_self<ContextMenuCommandRemoveFromLeftPane>();

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }
}

IFACEMETHODIMP ContextMenuCommand::GetFlags(EXPCMDFLAGS* flags)
{
    *flags = ECF_HASSUBCOMMANDS;

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommand::GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName)
{
    static const std::wstring title = L"MEGA";

    if (mEnumCommands->enabledSubCommandItems(psiItemArray) == 1)
    {
        auto uniqueCommand = mEnumCommands->getEnabledCommand(psiItemArray);
        if (uniqueCommand.has_value())
        {
            return uniqueCommand.value()->GetTitle(psiItemArray, ppszName);
        }
    }

    SHStrDup(title.data(), ppszName);

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommand::GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip)
{
    return GetTitle(psiItemArray, ppszInfotip);
}

IFACEMETHODIMP ContextMenuCommand::Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) noexcept
{
    if (mEnumCommands->enabledSubCommandItems(psiItemArray) == 1)
    {
        auto uniqueCommand = mEnumCommands->getEnabledCommand(psiItemArray);
        if (uniqueCommand.has_value())
        {
            return uniqueCommand.value()->Invoke(psiItemArray, pbc);
        }
    }

    return S_OK;
}

HRESULT ContextMenuCommand::EnumSubCommands(IEnumExplorerCommand** ppEnum)
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

EXPCMDSTATE ContextMenuCommand::GetCmdState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        mExpCmdState = ECS_HIDDEN;
    }
    else
    {
        mState->SetState(mId, Set);

        initializeContextMenuData(psiItemArray);

        if (mContextMenuData.isMEGASyncOpen())
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
