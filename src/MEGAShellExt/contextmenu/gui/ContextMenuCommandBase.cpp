#include "ContextMenuCommandBase.h"

#include "PathHelper.h"

ContextMenuData ContextMenuCommandBase::mContextMenuData;

ContextMenuCommandBase::ContextMenuCommandBase()
{
    mCounter = std::make_unique<SharedCounter>();
    mState = std::make_unique<SharedState>();
    mContextMenuData.reset();
}

IFACEMETHODIMP ContextMenuCommandBase::GetCanonicalName(GUID* pguidCommandName)
{
    *pguidCommandName = GUID_NULL;
    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandBase::GetFlags(EXPCMDFLAGS* flags)
{
    *flags = ECF_DEFAULT;
    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandBase::GetIcon(IShellItemArray* psiItemArray, LPWSTR* ppszIcon)
{
    UNREFERENCED_PARAMETER(psiItemArray);

    std::wstring icon(GetContextMenuPath() + L"\\MEGAsync.exe");
    SHStrDup(icon.data(), ppszIcon);

    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandBase::GetState(IShellItemArray* psiItemArray,
                                                BOOL fOkToBeSlow,
                                                EXPCMDSTATE* pCmdState)
{
    UNREFERENCED_PARAMETER(fOkToBeSlow);

    *pCmdState = GetState(psiItemArray);
    return S_OK;
}

IFACEMETHODIMP ContextMenuCommandBase::EnumSubCommands(IEnumExplorerCommand** ppEnum)
{
    *ppEnum = nullptr;
    return E_NOTIMPL;
}

void ContextMenuCommandBase::initializeContextMenuData(IShellItemArray* psiItemArray)
{
    if (!psiItemArray)
        return;

    if (!mContextMenuData.isReset())
        return;

    DWORD count = 0UL;
    HRESULT hr = psiItemArray->GetCount(&count);
    if (FAILED(hr))
        return;

    IShellItem* psi = nullptr;
    LPWSTR file2OpenPath;
    std::vector<std::wstring> selectedFiles;

    for (DWORD i = 0; i < count; ++i)
    {
        psiItemArray->GetItemAt(i, &psi);
        hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &file2OpenPath);
        if (FAILED(hr))
            return;

        // Release the IShellItem pointer, since we are done with it as well.
        psi->Release();

        selectedFiles.push_back(std::wstring(file2OpenPath));

        // Cleanup itemName, since we are done with it.
        if (file2OpenPath)
        {
            CoTaskMemFree(file2OpenPath);
        }
    }

    mContextMenuData.initialize(selectedFiles);
}
