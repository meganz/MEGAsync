#include "BackupCandidates.h"

#include "BackupsController.h"
#include "Utilities.h"

const char* BackupCandidates::Data::SIZE_READY = "size_ready";

BackupCandidates::Data::Data(const QString& folder, const QString& displayName, bool selected):
    mFolder(folder),
    mName(displayName),
    mSize(),
    mSelected(selected),
    mDone(false),
    mFolderSizeReady(false),
    mError(0),
    mFolderSize(FileFolderAttributes::NOT_READY),
    mSdkError(-1),
    mSyncError(-1),
    mFolderAttrContext(new QObject()),
    mFolderAttr(nullptr)
{}

BackupCandidates::Data::~Data()
{
    mFolderAttrContext->deleteLater();
}

void BackupCandidates::Data::setSize(long long size)
{
    if (mFolderSize != size)
    {
        mFolderSizeReady = (size != FileFolderAttributes::NOT_READY);

        if (size > FileFolderAttributes::NOT_READY)
        {
            mFolderSize = size;
            mSize = Utilities::getSizeStringLocalized(mFolderSize);
            // Swap the value to emit "dynamic_property" change signal
            mFolderAttrContext->setProperty(SIZE_READY, mFolder);
        }
        else
        {
            mFolderAttrContext->setProperty(SIZE_READY, QString());
        }
    }
}

void BackupCandidates::Data::setFolder(const QString& folder)
{
    mFolder = folder;
    if (!createFileFolderAttributes())
    {
        mFolderAttr->setPath(mFolder);
    }

    calculateFolderSize();
}

void BackupCandidates::Data::setError(int error)
{
    if (mSelected && !mDone && mError == BackupErrorCode::NONE)
    {
        mError = error;
    }
}

void BackupCandidates::Data::calculateFolderSize()
{
    createFileFolderAttributes();
    mFolderAttr->requestSize(mFolderAttrContext,
                             [&](long long size)
                             {
                                 setSize(size);
                             });
}

QHash<int, QByteArray> BackupCandidates::Data::roleNames()
{
    static QHash<int, QByteArray> roles{
        {BackupCandidates::NAME_ROLE,       "name"     },
        {BackupCandidates::FOLDER_ROLE,     "folder"   },
        {BackupCandidates::SIZE_ROLE,       "size"     },
        {BackupCandidates::SIZE_READY_ROLE, "sizeReady"},
        {BackupCandidates::SELECTED_ROLE,   "selected" },
        {BackupCandidates::DONE_ROLE,       "done"     },
        {BackupCandidates::ERROR_ROLE,      "error"    }
    };

    return roles;
}

bool BackupCandidates::Data::createFileFolderAttributes()
{
    if (!mFolderAttr)
    {
        mFolderAttr = new LocalFileFolderAttributes(mFolder, mFolderAttrContext);
        mFolderAttr->setValueUpdatesDisable();
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////

BackupCandidates::BackupCandidates() {}

QString BackupCandidates::getTotalSize() const
{
    return Utilities::getSizeStringLocalized(mBackupsTotalSize);
}

bool BackupCandidates::getIsTotalSizeReady() const
{
    return mTotalSizeReady;
}

void BackupCandidates::setIsTotalSizeReady(bool totalSizeReady)
{
    mTotalSizeReady = totalSizeReady;
    emit totalSizeReadyChanged();
}

Qt::CheckState BackupCandidates::getCheckAllState() const
{
    return mCheckAllState;
}

void BackupCandidates::setCheckAllState(Qt::CheckState state)
{
    if (mCheckAllState != state)
    {
        mCheckAllState = state;
        emit checkAllStateChanged();
    }
}

bool BackupCandidates::getExistConflicts() const
{
    return mConflictsSize > 0;
}

void BackupCandidates::setGlobalError(BackupErrorCode error)
{
    if (mGlobalError != error)
    {
        mGlobalError = error;
        emit globalErrorChanged();
    }
}

int BackupCandidates::getGlobalError() const
{
    return mGlobalError;
}

std::shared_ptr<BackupCandidates::Data> BackupCandidates::getBackupCandidate(int index) const
{
    return mBackupCandidatesList.at(index);
}

std::shared_ptr<BackupCandidates::Data>
    BackupCandidates::getBackupCandidateByFolder(const QString& folder) const
{
    foreach(auto& candidate, mBackupCandidatesList)
    {
        if (candidate->mFolder == folder)
        {
            return candidate;
        }
    }

    return nullptr;
}

QList<std::shared_ptr<BackupCandidates::Data>> BackupCandidates::getBackupCandidates() const
{
    return mBackupCandidatesList;
}

int BackupCandidates::size() const
{
    return mBackupCandidatesList.size();
}

int BackupCandidates::getRow(std::shared_ptr<BackupCandidates::Data> candidate) const
{
    return mBackupCandidatesList.indexOf(candidate);
}

int BackupCandidates::getRow(const QString& folder) const
{
    foreach(auto& candidate, mBackupCandidatesList)
    {
        if (candidate->mFolder == folder)
        {
            return getRow(candidate);
        }
    }

    return -1;
}

void BackupCandidates::addBackupCandidate(std::shared_ptr<BackupCandidates::Data> backupCandidate)
{
    mBackupCandidatesList.append(backupCandidate);
}

void BackupCandidates::removeBackupCandidate(int index)
{
    mBackupCandidatesList.removeAt(index);
}

bool BackupCandidates::removeBackupCandidate(const QString& folder)
{
    auto row(getRow(folder));
    if (row >= 0)
    {
        mBackupCandidatesList.removeAt(row);
        return true;
    }

    return false;
}

int BackupCandidates::conflictsSize() const
{
    return mConflictsSize;
}

void BackupCandidates::setConflictsSize(int newConflictsSize)
{
    mConflictsSize = newConflictsSize;
}

int BackupCandidates::SDKConflictCount() const
{
    return mSDKConflictCount;
}

void BackupCandidates::setSDKConflictCount(int newSdkConflictCount)
{
    mSDKConflictCount = newSdkConflictCount;
}

int BackupCandidates::remoteConflictCount() const
{
    return mRemoteConflictCount;
}

void BackupCandidates::setRemoteConflictCount(int newRemoteConflictCount)
{
    mRemoteConflictCount = newRemoteConflictCount;
}

int BackupCandidates::selectedRowsTotal() const
{
    return mSelectedRowsTotal;
}

void BackupCandidates::setSelectedRowsTotal(int newSelectedRowsTotal)
{
    mSelectedRowsTotal = newSelectedRowsTotal;

    if (mSelectedRowsTotal == 0)
    {
        emit noneSelected();
    }
}

long long BackupCandidates::backupsTotalSize() const
{
    return mBackupsTotalSize;
}

void BackupCandidates::setBackupsTotalSize(long long newBackupsTotalSize)
{
    if (mBackupsTotalSize != newBackupsTotalSize)
    {
        mBackupsTotalSize = newBackupsTotalSize;
        emit totalSizeChanged();
    }
}

QString BackupCandidates::getSdkErrorString() const
{
    QString message = tr("Folder wasn't backed up. Try again.", "", SDKConflictCount());
    auto candidateList(getBackupCandidates());
    auto itFound = std::find_if(candidateList.cbegin(),
                                candidateList.cend(),
                                [](const std::shared_ptr<BackupCandidates::Data> backupFolder)
                                {
                                    return (backupFolder->mSelected &&
                                            backupFolder->mError ==
                                                BackupCandidates::BackupErrorCode::SDK_CREATION);
                                });

    if (itFound != candidateList.cend())
    {
        message = BackupsController::instance().getErrorString((*itFound)->mSdkError,
                                                               (*itFound)->mSyncError);
    }

    return message;
}

QString BackupCandidates::getSyncErrorString() const
{
    QString message;
    auto candidateList(getBackupCandidates());
    auto itFound = std::find_if(candidateList.cbegin(),
                                candidateList.cend(),
                                [](const std::shared_ptr<BackupCandidates::Data> backupFolder)
                                {
                                    return (backupFolder->mSelected &&
                                            backupFolder->mError ==
                                                BackupCandidates::BackupErrorCode::SYNC_CONFLICT);
                                });

    if (itFound != candidateList.cend())
    {
        BackupsController::instance().isLocalFolderSyncable((*itFound)->getFolder(),
                                                            mega::MegaSync::TYPE_BACKUP,
                                                            message);
    }

    return message;
}

QString BackupCandidates::getConflictsNotificationText() const
{
    switch (getGlobalError())
    {
        case BackupCandidates::BackupErrorCode::DUPLICATED_NAME:
            return tr("You can't back up folders with the same name. "
                      "Rename them to continue with the backup. "
                      "Folder names won't change on your computer.");
        case BackupCandidates::BackupErrorCode::EXISTS_REMOTE:
            return tr("A folder with the same name already exists in your Backups. "
                      "Rename the new folder to continue with the backup. "
                      "Folder name will not change on your computer.",
                      "",
                      remoteConflictCount());
        case BackupCandidates::BackupErrorCode::SYNC_CONFLICT:
            return getSyncErrorString();
        case BackupCandidates::BackupErrorCode::PATH_RELATION:
            return tr("Backup folders can't contain or be contained by other backup folder");
        case BackupCandidates::BackupErrorCode::UNAVAILABLE_DIR:
            return tr("Folder can't be backed up as it can't be located. "
                      "It may have been moved or deleted, or you might not have access.");
        case BackupCandidates::BackupErrorCode::SDK_CREATION:
            return getSdkErrorString();
        default:
            return QString::fromUtf8("");
    }
}
