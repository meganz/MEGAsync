#include "ContextMenuCommandBase.h"

#include <windows.storage.h>

#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>

ContextMenuData ContextMenuCommandBase::mContextMenuData;

void ContextMenuCommandBase::log(const std::wstring& content) const
{
    const std::wstring fileName = L"c:\\temp\\winextension.log";

    // Create or open the file
    HANDLE hFile = CreateFile(fileName.c_str(), // File name
                              FILE_APPEND_DATA, // Desired access
                              0, // Share mode
                              NULL, // Security attributes
                              OPEN_EXISTING, // Creation disposition
                              FILE_ATTRIBUTE_NORMAL, // Flags and attributes
                              NULL); // Template file

    // Move the file pointer to the end of the file
    SetFilePointer(hFile, 0, NULL, FILE_END);

    // Write the content to the file
    std::wstringstream wss;
    wss << content;
    wss << std::endl;

    std::wstring linedContent = wss.str();

    DWORD bytesWritten;
    WriteFile(hFile, // File handle
              linedContent.c_str(), // Buffer to write
              static_cast<DWORD>(linedContent.size() * sizeof(wchar_t)),
              &bytesWritten, // Number of bytes written
              NULL); // Overlapped

    // Close the file handle
    CloseHandle(hFile);
}

ContextMenuCommandBase::ContextMenuCommandBase(const std::wstring& id):
    mId(id)
{
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

    auto package = winrt::Windows::ApplicationModel::Package::Current();
    auto appFolder = package.InstalledPath();

    std::wstring icon(appFolder + L"\\assets\\" + GetIcon());

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
