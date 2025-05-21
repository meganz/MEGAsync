#include "ContextMenuData.h"

#include "MEGAinterface.h"
#include "RegUtils.h"
#include "Utilities.h"

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
    WCHAR longPath[MAX_LONG_PATH];

    std::wstring prefixedPath = L"\\\\?\\" + path;

    DWORD result = GetLongPathNameW(prefixedPath.c_str(), longPath, MAX_LONG_PATH);

    if (result <= 0 || result >= MAX_LONG_PATH)
    {
        return;
    }

    WIN32_FILE_ATTRIBUTE_DATA fad;
    int type = MegaInterface::TYPE_UNKNOWN;
    if (GetFileAttributesExW(longPath, GetFileExInfoStandard, (LPVOID)&fad))
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

    int state = MegaInterface::getPathState(longPath, false);
    mSelectedPaths.push_back(longPath);
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
        if (CheckLeftPaneIcon(const_cast<wchar_t*>(longPath), false))
        {
            mInLeftPane = path;
        }
    }

    // Check if we can still sync with the new path
    //  If there is at least one non-syncable path, all of them are non-syncable
    if (type != MegaInterface::TYPE_FOLDER)
    {
        mCanSync = false;
    }
    else
    {
        mCanSync &= state == MegaInterface::FILE_NOTFOUND_SYNCABLE;
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

    mCanSync = true;
}

// Only if all items are not synced
bool ContextMenuData::canRequestUpload() const
{
    if (!isThereAnySyncedItem() && isThereAnyUnsyncedItem())
    {
        return true;
    }

    return false;
}

// Only if all items are synced
bool ContextMenuData::canRequestGetLinks() const
{
    if (isThereAnySyncedItem() && !isThereAnyUnsyncedItem())
    {
        return true;
    }

    return false;
}

// Only if all items are in the left pane
bool ContextMenuData::canRemoveFromLeftPane() const
{
    if (mInLeftPane.size() == mSelectedPaths.size())
    {
        return true;
    }

    return false;
}

// Only for synced folders
bool ContextMenuData::canViewOnMEGA() const
{
    if (mSelectedPaths.size() == 1 && mSyncedFolders)
    {
        return true;
    }

    return false;
}

// Only for synced files
bool ContextMenuData::canViewVersions() const
{
    if (mSelectedPaths.size() == 1 && mSyncedFiles)
    {
        return true;
    }

    return false;
}

// Only if all items are syncable
bool ContextMenuData::canSync(MegaInterface::SyncType type) const
{
    // We only allow syncing a single folder (for the moment)
    if (type == MegaInterface::SyncType::TYPE_TWOWAY && mSelectedPaths.size() > 1)
    {
        return false;
    }

    return mCanSync;
}

bool ContextMenuData::isThereAnyUnsyncedItem() const
{
    return mUnsyncedFiles || mUnsyncedFolders || mUnsyncedUnknowns || mSyncedUnknowns;
}

bool ContextMenuData::isThereAnySyncedItem() const
{
    return mSyncedFiles || mSyncedFolders;
}

bool ContextMenuData::isSynced(int type, int state)
{
    return (state == MegaInterface::FILE_SYNCED) ||
           ((type == MegaInterface::TYPE_FOLDER) && state == MegaInterface::FILE_SYNCING);
}

bool ContextMenuData::isUnsynced(int state)
{
    return (state == MegaInterface::FILE_NOTFOUND_NON_SYNCABLE ||
            state == MegaInterface::FILE_NOTFOUND_SYNCABLE);
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

void ContextMenuData::requestSync(MegaInterface::SyncType type)
{
    if (mSelectedPaths.size() > 0)
    {
        MegaInterface::sync(mSelectedPaths, type);

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

bool ContextMenuData::hasAnyOptionAvailable() const
{
    return (canRequestGetLinks() || canRemoveFromLeftPane() ||
            canSync(MegaInterface::SyncType::TYPE_TWOWAY) ||
            canSync(MegaInterface::SyncType::TYPE_BACKUP) || canRequestUpload() ||
            canViewOnMEGA() || canViewVersions());
}
