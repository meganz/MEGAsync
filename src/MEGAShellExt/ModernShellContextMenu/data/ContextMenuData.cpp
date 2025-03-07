#include "ContextMenuData.h"

#include "MEGAinterface.h"
#include "RegUtils.h"

void ContextMenuData::initialize(IShellItemArray* psiItemArray)
{
    reset();

    DWORD count = 0UL;
    HRESULT hr = psiItemArray->GetCount(&count);
    if (FAILED(hr))
    {
        return;
    }

    std::vector<std::wstring> selectedPaths;

    for (DWORD fileIndex = 0; fileIndex < count; ++fileIndex)
    {
        LPWSTR selectedPath;
        IShellItem* psi = nullptr;

        psiItemArray->GetItemAt(fileIndex, &psi);
        hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &selectedPath);
        if (FAILED(hr))
        {
            return;
        }

        psi->Release();

        if (selectedPath != nullptr)
        {
            selectedPaths.emplace_back(selectedPath);
            CoTaskMemFree(selectedPath);
        }
    }

    for (const std::wstring& selectedPath: selectedPaths)
    {
        processPath(selectedPath);
    }
}

void ContextMenuData::processPath(const std::wstring& path)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
    int type = MegaInterface::TYPE_UNKNOWN;
    if (GetFileAttributesExW(path.data(), GetFileExInfoStandard, (LPVOID)&fad))
    {
        type = (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? MegaInterface::TYPE_FOLDER :
                                                                   MegaInterface::TYPE_FILE;
    }
    else
    {
        DWORD error = GetLastError();
        if (error == ERROR_PATH_NOT_FOUND || error == ERROR_INVALID_NAME)
        {
            type = MegaInterface::TYPE_NOTFOUND;
        }
    }

    int state = MegaInterface::getPathState(path.data(), false);
    mSelectedPaths.push_back(path);
    mPathStates.push_back(state);
    mPathTypes.push_back(type);

    if (isSynced(type, state))
    {
        if (type == MegaInterface::TYPE_FOLDER)
        {
            mSyncedFolders++;
        }
        else if (type == MegaInterface::TYPE_FILE)
        {
            mSyncedFiles++;
        }
        else if (type == MegaInterface::TYPE_UNKNOWN)
        {
            mSyncedUnknowns++;
        }
    }
    else if (isUnsynced(state))
    {
        if (type == MegaInterface::TYPE_FOLDER)
        {
            mUnsyncedFolders++;
        }
        else if (type == MegaInterface::TYPE_FILE)
        {
            mUnsyncedFiles++;
        }
        else if (type == MegaInterface::TYPE_UNKNOWN)
        {
            mUnsyncedUnknowns++;
        }
    }

    if ((type == MegaInterface::TYPE_FOLDER || type == MegaInterface::TYPE_NOTFOUND) &&
        !mInLeftPane.size())
    {
        if (CheckLeftPaneIcon(const_cast<wchar_t*>(path.data()), false))
        {
            mInLeftPane = path;
        }
    }
}

void ContextMenuData::reset()
{
    mInLeftPane.clear();
    mSelectedPaths.clear();
    mPathStates.clear();
    mPathTypes.clear();

    mSyncedFolders = 0;
    mSyncedFiles = 0;
    mSyncedUnknowns = 0;
    mUnsyncedFolders = 0;
    mUnsyncedFiles = 0;
    mUnsyncedUnknowns = 0;
}

bool ContextMenuData::canRequestUpload() const
{
    if (mUnsyncedFolders || mUnsyncedFiles)
    {
        return true;
    }

    return false;
}

bool ContextMenuData::canRequestGetLinks() const
{
    if (mSyncedFolders || mSyncedFiles)
    {
        return true;
    }

    return false;
}

bool ContextMenuData::canRemoveFromLeftPane() const
{
    if (mInLeftPane.size())
    {
        return true;
    }

    return false;
}

bool ContextMenuData::canViewOnMEGA() const
{
    if (!mUnsyncedFiles && !mUnsyncedFolders && !mUnsyncedUnknowns && !mSyncedUnknowns &&
        mSelectedPaths.size() == 1 && (mSyncedFiles + mSyncedFolders) == 1 && mSyncedFolders)
    {
        return true;
    }

    return false;
}

bool ContextMenuData::canViewVersions() const
{
    if (!mUnsyncedFiles && !mUnsyncedFolders && !mUnsyncedUnknowns && !mSyncedUnknowns &&
        mSelectedPaths.size() == 1 && (mSyncedFiles + mSyncedFolders) == 1 && !mSyncedFolders)
    {
        return true;
    }

    return false;
}

bool ContextMenuData::isSynced(int type, int state)
{
    return (state == MegaInterface::FILE_SYNCED) ||
           ((type == MegaInterface::TYPE_FOLDER) && state == MegaInterface::FILE_SYNCING);
}

bool ContextMenuData::isUnsynced(int state)
{
    return (state == MegaInterface::FILE_NOTFOUND);
}

bool ContextMenuData::isMEGASyncOpen() const
{
    return MegaInterface::isMEGASyncOpen();
}

void ContextMenuData::requestUpload()
{
    for (auto fileIndex = 0u; fileIndex < mSelectedPaths.size(); ++fileIndex)
    {
        if (isUnsynced(mPathStates[fileIndex]))
        {
            MegaInterface::upload(mSelectedPaths[fileIndex].data());
        }
    }

    MegaInterface::endRequest();
}

void ContextMenuData::requestGetLinks()
{
    for (auto fileIndex = 0u; fileIndex < mSelectedPaths.size(); ++fileIndex)
    {
        if (isSynced(mPathTypes[fileIndex], mPathStates[fileIndex]))
        {
            MegaInterface::pasteLink(mSelectedPaths[fileIndex].data());
        }
    }

    MegaInterface::endRequest();
}

void ContextMenuData::removeFromLeftPane()
{
    if (!mInLeftPane.empty())
    {
        MegaInterface::removeFromLeftPane(mInLeftPane.data());
        MegaInterface::endRequest();
    }
}

void ContextMenuData::viewOnMEGA()
{
    if (mSelectedPaths.size())
    {
        MegaInterface::viewOnMEGA(mSelectedPaths.cbegin()->data());
        MegaInterface::endRequest();
    }
}

void ContextMenuData::viewVersions()
{
    if (mSelectedPaths.size())
    {
        MegaInterface::viewVersions(mSelectedPaths.cbegin()->data());
        MegaInterface::endRequest();
    }
}

int ContextMenuData::getUnsyncedFolders() const
{
    return mUnsyncedFolders;
}

int ContextMenuData::getUnsyncedFiles() const
{
    return mUnsyncedFiles;
}

int ContextMenuData::getSyncedFolders() const
{
    return mSyncedFolders;
}

int ContextMenuData::getSyncedFiles() const
{
    return mSyncedFiles;
}
