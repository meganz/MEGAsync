#ifndef SUBCOMMAND_ENUMERATOR_H
#define SUBCOMMAND_ENUMERATOR_H

#include "ContextMenuCommandGetLink.h"
#include "ContextMenuCommandRemoveFromLeftPane.h"
#include "ContextMenuCommandSyncBackup.h"
#include "ContextMenuCommandUpload.h"
#include "ContextMenuCommandView.h"
#include "ContextMenuCommandViewVersions.h"
#include <Unknwn.h>

#include <winrt/base.h>

#include <shobjidl_core.h>
#include <windows.h>

class SubCommandEnumerator: public winrt::implements<SubCommandEnumerator, IEnumExplorerCommand>
{
private:
    std::vector<winrt::com_ptr<IExplorerCommand>> mCommands;
    std::vector<winrt::com_ptr<IExplorerCommand>>::const_iterator mCurrent;

public:
    SubCommandEnumerator()
    {
        mCommands.push_back(winrt::make_self<ContextMenuCommandGetLink>());
        mCommands.push_back(winrt::make_self<ContextMenuCommandView>());
        mCommands.push_back(winrt::make_self<ContextMenuCommandViewVersions>());
        mCommands.push_back(
            winrt::make_self<ContextMenuCommandSyncBackup>(MegaInterface::SyncType::TYPE_TWOWAY));
        mCommands.push_back(
            winrt::make_self<ContextMenuCommandSyncBackup>(MegaInterface::SyncType::TYPE_BACKUP));
        mCommands.push_back(winrt::make_self<ContextMenuCommandUpload>());
        mCommands.push_back(winrt::make_self<ContextMenuCommandRemoveFromLeftPane>());

        mCurrent = mCommands.cbegin();
    }

    // Enumerar los subcomandos
    HRESULT STDMETHODCALLTYPE Next(ULONG celt,
                                   IExplorerCommand** rgelt,
                                   ULONG* pceltFetched) override
    {
        ULONG fetched{0};
        *pceltFetched = 0ul;

        for (ULONG index = 0; (index < celt) && (mCurrent != mCommands.cend()); index++)
        {
            mCurrent->copy_to(&rgelt[0]);
            mCurrent++;
            fetched++;
        }

        *pceltFetched = fetched;

        return (fetched == celt) ? S_OK : S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE Skip(ULONG celt) override
    {
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE Reset(void) override
    {
        mCurrent = mCommands.cbegin();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Clone(__RPC__deref_out_opt IEnumExplorerCommand** ppenum) override
    {
        return S_OK;
    }
};

#endif
