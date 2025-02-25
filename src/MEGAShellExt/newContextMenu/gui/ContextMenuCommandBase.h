#ifndef CONTEXTMENUCOMMANDBASE_H
#define CONTEXTMENUCOMMANDBASE_H

#include "../data/ContextMenuData.h"
#include "../utilities/SharedState.h"

#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>

class ContextMenuCommandBase: public winrt::implements<ContextMenuCommandBase, IExplorerCommand>
{
public:
    ContextMenuCommandBase(const std::wstring& id, bool isSubCommand);

    IFACEMETHODIMP GetCanonicalName(GUID* pguidCommandName) override;
    IFACEMETHODIMP GetFlags(EXPCMDFLAGS* flags) override;
    IFACEMETHODIMP GetIcon(IShellItemArray* psiItemArray, LPWSTR* ppszIcon) override;
    IFACEMETHODIMP GetState(IShellItemArray* psiItemArray,
                            BOOL fOkToBeSlow,
                            EXPCMDSTATE* pCmdState) override;
    IFACEMETHODIMP EnumSubCommands(IEnumExplorerCommand** ppEnum) override;

    virtual EXPCMDSTATE GetCmdState(IShellItemArray* psiItemArray)
    {
        return ECS_ENABLED;
    };

    std::wstring GetId() const
    {
        return mId;
    }

protected:
    void initializeContextMenuData(IShellItemArray* psiItemArray);
    void log(const std::wstring& content) const;

    std::wstring mId;
    bool mIsSubCommand;
    std::unique_ptr<SharedState> mState;
    _EXPCMDSTATE mExpCmdState = ECS_DISABLED;
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

    uint64_t enabledSubCommandItems(IShellItemArray* psiItemArray)
    {
        auto index = 0u;
        auto counter = 0u;

        log(L"enabledSubCommandItems()");

        while (index < subCommands.size())
        {
            auto subcommand = subCommands[index];
            auto menuCommand = reinterpret_cast<ContextMenuCommandBase*>(subcommand.get());

            if (menuCommand->GetCmdState(psiItemArray) == ECS_ENABLED)
            {
                log(menuCommand->GetId());
                log(L"ENABLED");
                ++counter;
            }
            else
            {
                log(menuCommand->GetId());
                log(L"HIDDEN");
            }

            ++index;
        }

        /*
        return (std::count_if(subCommands.cbegin(), subCommands.cend(),
                      [](const auto& command){
                            return dynamic_cast<const
        ContextMenuCommandBase*>(command.get())->isEnabled();
                      }));
        */

        std::wstringstream wss;
        wss << L"counter : " << counter;

        log(wss.str());

        return counter;
    }

    std::optional<IExplorerCommand*> getEnabledCommand(IShellItemArray* psiItemArray)
    {
        /*
        auto it = std::find_if(subCommands.cbegin(),
                               subCommands.cend(),
                               [](const auto& command)
                               {
                                   return reinterpret_cast<ContextMenuCommandBase*>(
                                              subCommands[index].get())
                                              ->GetCmdState(psiItemArray) == ECS_ENABLED;
                               });

        if (it != subCommands.cend())
        {
            return it->get();
        }
        */

        auto index = 0u;

        while (index < subCommands.size())
        {
            auto subcommand = subCommands[index];
            auto menuCommand = reinterpret_cast<ContextMenuCommandBase*>(subcommand.get());

            if (menuCommand->GetCmdState(psiItemArray) == ECS_ENABLED)
            {
                return subcommand.get();
            }

            ++index;
        }

        return std::nullopt;
    }

    void log(const std::wstring& content) const
    {
        const std::wstring fileName = L"c:\\temp\\winextension.log";

        HANDLE hFile = CreateFile(fileName.c_str(),
                                  FILE_APPEND_DATA,
                                  0,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);

        SetFilePointer(hFile, 0, NULL, FILE_END);

        std::wstringstream wss;
        wss << content;
        wss << std::endl;

        std::wstring linedContent = wss.str();

        DWORD bytesWritten;
        WriteFile(hFile,
                  linedContent.c_str(),
                  static_cast<DWORD>(linedContent.size() * sizeof(wchar_t)),
                  &bytesWritten,
                  NULL);

        CloseHandle(hFile);
    }
};

#endif
