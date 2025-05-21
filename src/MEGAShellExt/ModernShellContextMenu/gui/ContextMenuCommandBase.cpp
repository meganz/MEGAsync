#include "ContextMenuCommandBase.h"

#include "Utilities.h"

ContextMenuData ContextMenuCommandBase::mContextMenuData;

ContextMenuCommandBase::ContextMenuCommandBase(const std::wstring& id):
    mId(id)
{}

void ContextMenuCommandBase::SetTitle(MegaInterface::StringID id, LPWSTR* ppszName)
{
    std::wstring title;
    auto menuText = MegaInterface::getString(id);
    if (menuText)
    {
        title = menuText.get();
    }

    SHStrDup(title.data(), ppszName);
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

    auto package = winrt::Windows::ApplicationModel::Package::Current();
    auto appFolder = package.InstalledPath();
    auto theme = Utilities::isDarkModeActive() ? L"dark" : L"light";

    std::wstring icon(appFolder + L"\\assets\\" + theme + L"\\" + GetIcon());

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
    if (psiItemArray == nullptr)
    {
        return;
    }

    mContextMenuData.initialize(psiItemArray);
}
