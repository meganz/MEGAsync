#ifndef CONTEXTMENUCOMMANDBASE_H
#define CONTEXTMENUCOMMANDBASE_H

#include "../data/ContextMenuData.h"
#include "../utilities/SharedState.h"

class ContextMenuCommandBase: public winrt::implements<ContextMenuCommandBase, IExplorerCommand>
{
public:
    ContextMenuCommandBase();

    IFACEMETHODIMP GetCanonicalName(GUID* pguidCommandName) override;
    IFACEMETHODIMP GetFlags(EXPCMDFLAGS* flags) override;
    IFACEMETHODIMP GetIcon(IShellItemArray* psiItemArray, LPWSTR* ppszIcon) override;
    IFACEMETHODIMP GetState(IShellItemArray* psiItemArray,
                            BOOL fOkToBeSlow,
                            EXPCMDSTATE* pCmdState) override;
    IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand** ppEnum) override;

protected:
    void initializeContextMenuData(IShellItemArray* psiItemArray);
    virtual const EXPCMDSTATE GetState(IShellItemArray* psiItemArray) = 0;

protected:
    std::unique_ptr<SharedState> mState;
    static ContextMenuData mContextMenuData;
};

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

    HRESULT STDMETHODCALLTYPE Skip(ULONG celt)
    {
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE Reset(void)
    {
        subCommands.clear();
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Clone(
        /* [out] */ __RPC__deref_out_opt IEnumExplorerCommand** ppenum)
    {
        return S_OK;
    }
};

#endif
