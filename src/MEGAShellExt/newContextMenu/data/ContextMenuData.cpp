#include "ContextMenuData.h"

#include "../../MEGAinterface.h"
#include "../../RegUtils.h"

ContextMenuData::ContextMenuData(void)
{
    reset();
}

ContextMenuData::~ContextMenuData(void)
{
    reset();
}

void ContextMenuData::initialize(const std::vector<std::wstring>& selectedFiles,
                                 bool forceInitialize)
{
    if (forceInitialize || isReset())
    {
        reset();

        for (const std::wstring& selectedFile: selectedFiles)
        {
            processFile(selectedFile);
        }
    }
}

void ContextMenuData::reset()
{
    mIsReset = true;
    mInLeftPane.clear();
    mSelectedFiles.clear();
    mPathStates.clear();
    mPathTypes.clear();
    mSyncedFolders = mSyncedFiles = mSyncedUnknowns = 0;
    mUnsyncedFolders = mUnsyncedFiles = mUnsyncedUnknowns = 0;
}

bool ContextMenuData::isReset() const
{
    return mIsReset;
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
        return true;

    return false;
}

bool ContextMenuData::canViewOnMEGA() const
{
    if (!mUnsyncedFiles && !mUnsyncedFolders && !mUnsyncedUnknowns && !mSyncedUnknowns &&
        mSelectedFiles.size() == 1 && (mSyncedFiles + mSyncedFolders) == 1 && mSyncedFolders)
    {
        return true;
    }

    return false;
}

bool ContextMenuData::canViewVersions() const
{
    if (!mUnsyncedFiles && !mUnsyncedFolders && !mUnsyncedUnknowns && !mSyncedUnknowns &&
        mSelectedFiles.size() == 1 && (mSyncedFiles + mSyncedFolders) == 1 && !mSyncedFolders)
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

void ContextMenuData::processFile(const std::wstring& fileOrDirPath)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
    int type = MegaInterface::TYPE_UNKNOWN;
    if (GetFileAttributesExW(fileOrDirPath.data(), GetFileExInfoStandard, (LPVOID)&fad))
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

    int state = MegaInterface::getPathState(fileOrDirPath.data(), false);
    mSelectedFiles.push_back(fileOrDirPath);
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
        if (CheckLeftPaneIcon(const_cast<wchar_t*>(fileOrDirPath.data()), false))
        {
            mInLeftPane = fileOrDirPath;
        }
    }
}

void ContextMenuData::requestUpload()
{
    for (unsigned int i = 0; i < mSelectedFiles.size(); i++)
    {
        if (isUnsynced(mPathStates[i]))
            MegaInterface::upload(mSelectedFiles[i].data());
    }

    MegaInterface::endRequest();
}

void ContextMenuData::requestGetLinks()
{
    for (unsigned int i = 0; i < mSelectedFiles.size(); i++)
    {
        if (isSynced(mPathTypes[i], mPathStates[i]))
            MegaInterface::pasteLink(mSelectedFiles[i].data());
    }

    MegaInterface::endRequest();
}

void ContextMenuData::removeFromLeftPane()
{
    if (!mInLeftPane.size())
        return;

    CheckLeftPaneIcon(mInLeftPane.data(), true);
    MegaInterface::endRequest();
}

void ContextMenuData::viewOnMEGA()
{
    if (mSelectedFiles.size())
    {
        MegaInterface::viewOnMEGA(mSelectedFiles[0].data());
        MegaInterface::endRequest();
    }
}

void ContextMenuData::viewVersions()
{
    if (mSelectedFiles.size())
    {
        MegaInterface::viewVersions(mSelectedFiles[0].data());
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
