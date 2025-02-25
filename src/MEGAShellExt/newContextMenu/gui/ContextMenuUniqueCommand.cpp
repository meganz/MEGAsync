#include "ContextMenuUniqueCommand.h"

#include "ContextMenuCommandGetLink.h"
#include "ContextMenuCommandRemoveFromLeftPane.h"
#include "ContextMenuCommandUpload.h"
#include "ContextMenuCommandView.h"
#include "ContextMenuCommandViewVersions.h"

ContextMenuUniqueCommand::ContextMenuUniqueCommand():
    ContextMenuCommandBase(L"ContextMenuCommand", false)
{
    mEnumCommands = winrt::make_self<SubCommandEnumerator>();

    {
        winrt::com_ptr<ContextMenuCommandGetLink> comPointer =
            winrt::make_self<ContextMenuCommandGetLink>(true);

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }

    {
        winrt::com_ptr<ContextMenuCommandView> comPointer =
            winrt::make_self<ContextMenuCommandView>(true);

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }

    {
        winrt::com_ptr<ContextMenuCommandViewVersions> comPointer =
            winrt::make_self<ContextMenuCommandViewVersions>(true);

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }

    {
        winrt::com_ptr<ContextMenuCommandUpload> comPointer =
            winrt::make_self<ContextMenuCommandUpload>(true);

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }

    {
        winrt::com_ptr<ContextMenuCommandRemoveFromLeftPane> comPointer =
            winrt::make_self<ContextMenuCommandRemoveFromLeftPane>(true);

        mEnumCommands.get()->subCommands.push_back(comPointer);
    }
}

IFACEMETHODIMP ContextMenuUniqueCommand::GetFlags(EXPCMDFLAGS* flags)
{
    *flags = ECF_DEFAULT;

    return S_OK;
}

IFACEMETHODIMP ContextMenuUniqueCommand::GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName)
{
    if (mEnumCommands->enabledSubCommandItems(psiItemArray) == 1)
    {
        auto uniqueCommand = mEnumCommands->getEnabledCommand(psiItemArray);
        if (uniqueCommand.has_value())
        {
            return uniqueCommand.value()->GetTitle(psiItemArray, ppszName);
        }
    }

    return S_OK;
}

IFACEMETHODIMP ContextMenuUniqueCommand::GetToolTip(IShellItemArray* psiItemArray,
                                                    LPWSTR* ppszInfotip)
{
    return GetTitle(psiItemArray, ppszInfotip);
}

IFACEMETHODIMP ContextMenuUniqueCommand::Invoke(IShellItemArray* psiItemArray,
                                                IBindCtx* pbc) noexcept
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

HRESULT ContextMenuUniqueCommand::EnumSubCommands(IEnumExplorerCommand** ppEnum)
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

EXPCMDSTATE ContextMenuUniqueCommand::GetCmdState(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
    {
        mExpCmdState = ECS_HIDDEN;
    }
    else
    {
        mState->SetState(mId, Set);

        initializeContextMenuData(psiItemArray);

        if (mContextMenuData.isMEGASyncOpen() &&
            mEnumCommands->enabledSubCommandItems(psiItemArray) == 1)
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
