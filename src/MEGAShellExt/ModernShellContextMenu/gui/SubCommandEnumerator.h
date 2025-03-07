#ifndef SUBCOMMAND_ENUMERATOR_H
#define SUBCOMMAND_ENUMERATOR_H

#include <winrt/base.h>

#include <shobjidl_core.h>
#include <windows.h>

class SubCommandEnumerator: public winrt::implements<SubCommandEnumerator, IEnumExplorerCommand>
{
private:
    size_t currentIndex;

public:
    SubCommandEnumerator():
        currentIndex(0)
    {}

    std::vector<winrt::com_ptr<IExplorerCommand>> subCommands;

    // Enumerar los subcomandos
    HRESULT STDMETHODCALLTYPE Next(ULONG celt,
                                   IExplorerCommand** rgelt,
                                   ULONG* pceltFetched) override
    {
        ULONG fetched = 0;
        while (currentIndex < subCommands.size() && fetched < celt)
        {
            rgelt[fetched] = subCommands[currentIndex].get();
            currentIndex++;
            fetched++;
        }

        if (pceltFetched != nullptr)
        {
            *pceltFetched = fetched;
        }

        return (fetched == celt) ? S_OK : S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE Skip(ULONG celt) override
    {
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE Reset(void) override
    {
        subCommands.clear();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Clone(
        /* [out] */ __RPC__deref_out_opt IEnumExplorerCommand** ppenum) override
    {
        return S_OK;
    }
};

#endif
