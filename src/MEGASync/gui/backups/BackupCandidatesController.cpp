#include "BackupCandidatesController.h"

#include <BackupCandidates.h>
#include <BackupCandidatesFolderSizeRequester.h>
#include <BackupCandidatesModel.h>
#include <BackupsController.h>
#include <FileFolderAttributes.h>
#include <QApplication>
#include <QmlManager.h>
#include <StandardIconProvider.h>
#include <SyncSettings.h>
#include <Utilities.h>

static const int CHECK_DIRS_TIME = 1000;

BackupCandidatesController::BackupCandidatesController():
    DataController(),
    mBackupCandidates(std::make_shared<BackupCandidates>()),
    mBackupCandidatesSizeRequester(new BackupCandidatesFolderSizeRequester(this))
{
    connect(SyncInfo::instance(),
            &SyncInfo::syncRemoved,
            this,
            &BackupCandidatesController::onSyncRemoved);
    connect(&BackupsController::instance(),
            &BackupsController::backupsCreationFinished,
            this,
            &BackupCandidatesController::onBackupsCreationFinished);
    connect(&BackupsController::instance(),
            &BackupsController::backupFinished,
            this,
            &BackupCandidatesController::onBackupFinished);
    connect(&mCheckDirsTimer,
            &QTimer::timeout,
            this,
            &BackupCandidatesController::checkDirectories);

    QmlManager::instance()->setRootContextProperty(mBackupCandidates.get());

    mCheckDirsTimer.setInterval(CHECK_DIRS_TIME);
    mCheckDirsTimer.start();

    connect(mBackupCandidatesSizeRequester,
            &BackupCandidatesFolderSizeRequester::sizeReceived,
            this,
            &BackupCandidatesController::onFolderSizeReceived);
}

std::shared_ptr<BackupCandidates::Data>
    BackupCandidatesController::createData(const QString& folder,
                                           const QString& displayName,
                                           bool selected)
{
    auto data = std::make_shared<BackupCandidates::Data>(folder, displayName, selected);
    // Used to detect size changes
    mBackupCandidatesSizeRequester->addFolder(folder);
    return data;
}

void BackupCandidatesController::init()
{
    // Default directories definition
    static QVector<QStandardPaths::StandardLocation> defaultPaths = {
        QStandardPaths::DesktopLocation,
        QStandardPaths::DocumentsLocation,
        QStandardPaths::MusicLocation,
        QStandardPaths::PicturesLocation};

    // Iterate defaultPaths to add to mBackupFolderList if the path is not empty
    for (auto type: qAsConst(defaultPaths))
    {
        const auto standardPaths(QStandardPaths::standardLocations(type));
        QDir dir(QDir::cleanPath(standardPaths.first()));
        QString path(QDir::toNativeSeparators(dir.canonicalPath()));
        if (dir.exists() && dir != QDir::home() && isLocalFolderSyncable(path))
        {
            auto data =
                createData(path, BackupsController::instance().getSyncNameFromPath(path), false);
            auto size(mBackupCandidates->getSize());
            emit beginInsertRows(size, size);
            mBackupCandidates->addBackupCandidate(data);
            emit endInsertRows();
        }
        else
        {
            mega::MegaApi::log(
                mega::MegaApi::LOG_LEVEL_WARNING,
                QString::fromUtf8("Default path %1 is not valid.").arg(path).toUtf8().constData());
        }
    }
}

void BackupCandidatesController::setCheckAllState(Qt::CheckState state, bool fromModel)
{
    auto checkAllState(mBackupCandidates->getCheckAllState());

    if (!fromModel && checkAllState == Qt::CheckState::Unchecked &&
        state == Qt::CheckState::PartiallyChecked)
    {
        state = Qt::CheckState::Checked;
    }

    if (checkAllState == state)
    {
        return;
    }

    if (checkAllState != Qt::CheckState::Checked && state == Qt::CheckState::Checked)
    {
        setAllSelected(true);
    }
    else if (checkAllState != Qt::CheckState::Unchecked && state == Qt::CheckState::Unchecked)
    {
        setAllSelected(false);
    }

    mBackupCandidates->setCheckAllState(state);
}

void BackupCandidatesController::setCheckState(int row, bool state)
{
    setData(row, state, BackupCandidates::SELECTED_ROLE);
    checkSelectedAll();
}

int BackupCandidatesController::insert(const QString& folder)
{
    QString inputPath(QDir::toNativeSeparators(QDir(folder).absolutePath()));
    auto existingRow(selectIfExistsInsertion(inputPath));
    if (existingRow >= 0)
    {
        // If the folder exists in the table then select the item and return
        return existingRow;
    }

    auto last(mBackupCandidates->getSize());
    emit beginInsertRows(last, last);

    auto data =
        createData(inputPath, BackupsController::instance().getSyncNameFromPath(inputPath), true);
    mBackupCandidates->addBackupCandidate(data);
    emit endInsertRows();

    checkSelectedAll();

    return (mBackupCandidates->getSize() - 1);
}

void BackupCandidatesController::setAllSelected(bool selected)
{
    QListIterator<std::shared_ptr<BackupCandidates::Data>> it(
        mBackupCandidates->getBackupCandidates());

    while (it.hasNext())
    {
        auto backupFolder = it.next();
        if (backupFolder->mSelected != selected &&
            (!selected || checkPermissions(backupFolder->mFolder)))
        {
            setData(backupFolder, selected, BackupCandidates::SELECTED_ROLE);
        }
    }

    updateSelectedAndTotalSize();
}

bool BackupCandidatesController::checkPermissions(const QString& inputPath)
{
    QDir dir(inputPath);
    dir.isEmpty(); // this triggers permission request on macOS, don´t remove
    if (dir.exists() && !dir.isReadable()) // this didn´t trigger permission request
    {
        return false;
    }
    return true;
}

void BackupCandidatesController::updateSelectedAndTotalSize()
{
    auto newSelectedRowsTotal(0);
    auto lastTotalSize = mBackupCandidates->getBackupsTotalSize();
    long long totalSize = 0;

    int selectedAndSizeReadyFolders = 0;

    foreach(auto candidate, mBackupCandidates->getBackupCandidates())
    {
        if (candidate->mSelected)
        {
            ++newSelectedRowsTotal;

            if (data(candidate, BackupCandidates::SIZE_READY_ROLE).toBool())
            {
                ++selectedAndSizeReadyFolders;
                totalSize += candidate->mFolderSize;
            }
        }
    }

    if (selectedAndSizeReadyFolders == newSelectedRowsTotal)
    {
        if (totalSize != lastTotalSize)
        {
            mBackupCandidates->setBackupsTotalSize(totalSize);
        }

        mBackupCandidates->setIsTotalSizeReady(true);
    }
    else
    {
        mBackupCandidates->setIsTotalSizeReady(false);
    }

    mBackupCandidates->setSelectedRowsTotal(newSelectedRowsTotal);
}

void BackupCandidatesController::checkSelectedAll()
{
    updateSelectedAndTotalSize();

    Qt::CheckState state = Qt::CheckState::PartiallyChecked;
    if (mBackupCandidates->selectedRowsTotal() == 0)
    {
        state = Qt::CheckState::Unchecked;
    }
    else if (mBackupCandidates->selectedRowsTotal() == mBackupCandidates->getSize())
    {
        state = Qt::CheckState::Checked;
    }

    setCheckAllState(state, true);
}

bool BackupCandidatesController::isLocalFolderSyncable(const QString& inputPath)
{
    QString message;
    return (BackupsController::instance().isLocalFolderSyncable(inputPath,
                                                                mega::MegaSync::TYPE_BACKUP,
                                                                message) !=
            SyncController::CANT_SYNC);
}

int BackupCandidatesController::selectIfExistsInsertion(const QString& inputPath)
{
    auto backupCandidate = mBackupCandidates->getBackupCandidateByFolder(inputPath);
    if (backupCandidate)
    {
        if (!backupCandidate->mSelected)
        {
            setData(backupCandidate, true, BackupCandidates::SELECTED_ROLE);
        }

        return mBackupCandidates->getRow(backupCandidate);
    }

    return -1;
}

bool BackupCandidatesController::folderContainsOther(const QString& folder,
                                                     const QString& other) const
{
    if (folder == other)
    {
        return true;
    }
    return folder.startsWith(other) && folder[other.size()] == QDir::separator();
}

bool BackupCandidatesController::isRelatedFolder(const QString& folder,
                                                 const QString& existingPath) const
{
    return folderContainsOther(folder, existingPath) || folderContainsOther(existingPath, folder);
}

QList<QList<std::shared_ptr<BackupCandidates::Data>>::const_iterator>
    BackupCandidatesController::getRepeatedNameItList(const QString& name)
{
    QList<QList<std::shared_ptr<BackupCandidates::Data>>::const_iterator> ret;
    auto candidateList(mBackupCandidates->getBackupCandidates());
    for (auto it = candidateList.cbegin(); it != candidateList.cend(); ++it)
    {
        if ((*it)->mName == name)
        {
            ret.append(it);
        }
    }
    return ret;
}

void BackupCandidatesController::reviewConflicts()
{
    int duplicatedCount = 0;
    int syncConflictCount = 0;
    int pathRelationCount = 0;
    int unavailableCount = 0;

    auto SDKConflictCount = 0;
    auto remoteConflictCount = 0;

    foreach(auto candidate, mBackupCandidates->getBackupCandidates())
    {
        if (candidate->mSelected)
        {
            switch (candidate->mError)
            {
                case BackupCandidates::BackupErrorCode::DUPLICATED_NAME:
                    duplicatedCount++;
                    break;
                case BackupCandidates::BackupErrorCode::EXISTS_REMOTE:
                    remoteConflictCount++;
                    break;
                case BackupCandidates::BackupErrorCode::SYNC_CONFLICT:
                    syncConflictCount++;
                    break;
                case BackupCandidates::BackupErrorCode::PATH_RELATION:
                    pathRelationCount++;
                    break;
                case BackupCandidates::BackupErrorCode::UNAVAILABLE_DIR:
                    unavailableCount++;
                    break;
                case BackupCandidates::BackupErrorCode::SDK_CREATION:
                    SDKConflictCount++;
                    break;
                default:
                    break;
            }
        }
    }

    mBackupCandidates->setSDKConflictCount(SDKConflictCount);
    mBackupCandidates->setRemoteConflictCount(remoteConflictCount);

    BackupCandidates::BackupErrorCode error(BackupCandidates::BackupErrorCode::NONE);

    if (SDKConflictCount > 0)
    {
        error = BackupCandidates::BackupErrorCode::SDK_CREATION;
    }
    else if (syncConflictCount > 0)
    {
        error = BackupCandidates::BackupErrorCode::SYNC_CONFLICT;
    }
    else if (duplicatedCount > 0)
    {
        error = BackupCandidates::BackupErrorCode::DUPLICATED_NAME;
    }
    else if (remoteConflictCount > 0)
    {
        error = BackupCandidates::BackupErrorCode::EXISTS_REMOTE;
    }
    else if (pathRelationCount > 0)
    {
        error = BackupCandidates::BackupErrorCode::PATH_RELATION;
    }
    else if (unavailableCount > 0)
    {
        error = BackupCandidates::BackupErrorCode::UNAVAILABLE_DIR;
    }

    createConflictsNotificationText(error);
    mBackupCandidates->setGlobalError(error);
}

bool BackupCandidatesController::existOtherRelatedFolder(
    std::shared_ptr<BackupCandidates::Data> backupCandidate)
{
    auto backupCandidates(mBackupCandidates->getBackupCandidates());
    auto foundBackupCandidate = std::find_if(
        backupCandidates.cbegin(),
        backupCandidates.cend(),
        [this, backupCandidate](std::shared_ptr<BackupCandidates::Data> conflictBackupCandidate)
        {
            return conflictBackupCandidate->mSelected &&
                   backupCandidate != conflictBackupCandidate &&
                   isRelatedFolder(backupCandidate->mFolder, conflictBackupCandidate->mFolder);
        });

    if (foundBackupCandidate == backupCandidates.cend())
    {
        return false;
    }

    setData(backupCandidate,
            BackupCandidates::BackupErrorCode::PATH_RELATION,
            BackupCandidates::ERROR_ROLE);
    setData((*foundBackupCandidate),
            BackupCandidates::BackupErrorCode::PATH_RELATION,
            BackupCandidates::ERROR_ROLE);

    return true;
}

void BackupCandidatesController::refreshBackupCandidatesErrors()
{
    auto candidateList(checkIfFoldersAreSyncable());

    checkDuplicatedBackups(candidateList);

    reviewConflicts();
}

QStringList BackupCandidatesController::checkIfFoldersAreSyncable()
{
    QStringList candidateList;
    foreach(const auto& backupCandidate, mBackupCandidates->getBackupCandidates())
    {
        if (backupCandidate->mSelected)
        {
            if (backupCandidate->mError == BackupCandidates::BackupErrorCode::SDK_CREATION)
            {
                continue;
            }

            // Clean error
            setData(backupCandidate,
                    BackupCandidates::BackupErrorCode::NONE,
                    BackupCandidates::ERROR_ROLE);

            if (!backupCandidate->mDone)
            {
                candidateList.append(backupCandidate->mName);

                if (!existOtherRelatedFolder(backupCandidate) &&
                    BackupsController::instance().isLocalFolderSyncable(
                        backupCandidate->mFolder,
                        mega::MegaSync::TYPE_BACKUP) != SyncController::CAN_SYNC)
                {
                    handleDirectoryStatus(backupCandidate);
                }
            }
        }
    }

    return candidateList;
}

void BackupCandidatesController::handleDirectoryStatus(
    std::shared_ptr<BackupCandidates::Data> candidate)
{
    QDir dir(candidate->mFolder);
    setData(candidate,
            dir.exists() ? BackupCandidates::BackupErrorCode::SYNC_CONFLICT :
                           BackupCandidates::BackupErrorCode::UNAVAILABLE_DIR,
            BackupCandidates::ERROR_ROLE);
}

void BackupCandidatesController::checkDuplicatedBackups(const QStringList& candidateList)
{
    QSet<QString> remoteSet = BackupsController::instance().getRemoteFolders();
    QSet<QString> localSet;
    QStringListIterator it(candidateList);
    while (it.hasNext())
    {
        auto name = it.next();

        BackupCandidates::BackupErrorCode error = BackupCandidates::BackupErrorCode::NONE;

        if (remoteSet.contains(name))
        {
            error = BackupCandidates::BackupErrorCode::EXISTS_REMOTE;
        }
        else if (localSet.contains(name))
        {
            error = BackupCandidates::BackupErrorCode::DUPLICATED_NAME;
        }
        else
        {
            localSet.insert(name);
        }

        if (error != BackupCandidates::BackupErrorCode::NONE)
        {
            foreach(auto foldIt, getRepeatedNameItList(name))
            {
                setData((*foldIt), error, BackupCandidates::ERROR_ROLE);
            }
        }
    }
}

bool BackupCandidatesController::setData(int row, const QVariant& value, int role)
{
    return setData(mBackupCandidates->getBackupCandidate(row), value, role);
}

bool BackupCandidatesController::setData(std::shared_ptr<BackupCandidates::Data> candidate,
                                         QVariant value,
                                         int role)
{
    auto result(true);

    switch (role)
    {
        case BackupCandidates::NAME_ROLE:
            candidate->mName = value.toString();
            break;
        case BackupCandidates::SIZE_ROLE:
            candidate->mFolderSize = value.toLongLong();
            break;
        case BackupCandidates::FOLDER_ROLE:
            candidate->mFolder = value.toString();
            break;
        case BackupCandidates::SELECTED_ROLE:
        {
            if (checkPermissions(candidate->mFolder))
            {
                candidate->mSelected = value.toBool();
                checkSelectedAll();
            }
            break;
        }
        case BackupCandidates::DONE_ROLE:
            candidate->mDone = value.toBool();
            break;
        case BackupCandidates::ERROR_ROLE:
            candidate->mError = value.toInt();
            break;
        default:
            result = false;
            break;
    }

    if (result)
    {
        updateModel(role, candidate);
    }

    return result;
}

QVariant BackupCandidatesController::data(int row, int role) const
{
    std::shared_ptr<BackupCandidates::Data> item = mBackupCandidates->getBackupCandidate(row);

    return data(item, role);
}

QVariant BackupCandidatesController::data(std::shared_ptr<BackupCandidates::Data> candidate,
                                          int role) const
{
    QVariant field;

    if (candidate)
    {
        switch (role)
        {
            case BackupCandidates::NAME_ROLE:
                field = candidate->mName;
                break;
            case BackupCandidates::FOLDER_ROLE:
                field = candidate->mFolder;
                break;
            case BackupCandidates::SIZE_ROLE:
                field = Utilities::getSizeStringLocalized(candidate->mFolderSize);
                break;
            case BackupCandidates::SIZE_READY_ROLE:
                field = candidate->mFolderSize != LocalFileFolderAttributes::NOT_READY;
                break;
            case BackupCandidates::SELECTED_ROLE:
                field = candidate->mSelected;
                break;
            case BackupCandidates::DONE_ROLE:
                field = candidate->mDone;
                break;
            case BackupCandidates::ERROR_ROLE:
                field = candidate->mError;
                break;
            default:
                break;
        }
    }

    return field;
}

int BackupCandidatesController::size() const
{
    return mBackupCandidates->getSize();
}

void BackupCandidatesController::calculateFolderSizes()
{
    for (auto& backupFolder: mBackupCandidates->getBackupCandidates())
    {
        if (backupFolder->mSelected)
        {
            mBackupCandidatesSizeRequester->calculateFolderSize(backupFolder->mFolder);
        }
    }
}

int BackupCandidatesController::rename(const QString& folder, const QString& name)
{
    auto candidate = mBackupCandidates->getBackupCandidateByFolder(folder);

    if (!candidate)
    {
        return BackupCandidates::BackupErrorCode::UNAVAILABLE_DIR;
    }

    QSet<QString> remoteFolders = BackupsController::instance().getRemoteFolders();
    if (remoteFolders.contains(name))
    {
        setData(candidate,
                BackupCandidates::BackupErrorCode::EXISTS_REMOTE,
                BackupCandidates::ERROR_ROLE);
        return candidate->mError;
    }

    foreach(const auto& checkCandidate, mBackupCandidates->getBackupCandidates())
    {
        if (checkCandidate != candidate && candidate->mSelected && name == checkCandidate->mName)
        {
            setData(candidate,
                    BackupCandidates::BackupErrorCode::DUPLICATED_NAME,
                    BackupCandidates::ERROR_ROLE);
            return candidate->mError;
        }
    }

    // How to let the model know that there was a changed?
    setData(candidate, name, BackupCandidates::NAME_ROLE);
    refreshBackupCandidatesErrors();

    return candidate->mError;
}

void BackupCandidatesController::remove(const QString& folder)
{
    auto row(mBackupCandidates->getRow(folder));
    if (row >= 0)
    {
        emit beginRemoveRows(row, row);

        if (mBackupCandidates->removeBackupCandidate(folder))
        {
            mBackupCandidatesSizeRequester->removeFolder(folder);
        }

        emit endRemoveRows();

        refreshBackupCandidatesErrors();
        checkSelectedAll();
    }
}

bool BackupCandidatesController::existsFolder(const QString& inputPath)
{
    return mBackupCandidates->getBackupCandidateByFolder(inputPath) != nullptr;
}

void BackupCandidatesController::change(const QString& oldFolder, const QString& newFolder)
{
    if (oldFolder == newFolder)
    {
        return;
    }

    auto candidate = mBackupCandidates->getBackupCandidateByFolder(oldFolder);
    if (candidate && candidate->mSelected)
    {
        if (existsFolder(newFolder))
        {
            remove(newFolder);
        }

        setData(candidate,
                BackupsController::instance().getSyncNameFromPath(newFolder),
                BackupCandidates::NAME_ROLE);
        setData(candidate, newFolder, BackupCandidates::FOLDER_ROLE);

        if (candidate->mError == BackupCandidates::BackupErrorCode::SDK_CREATION)
        {
            setData(candidate,
                    BackupCandidates::BackupErrorCode::NONE,
                    BackupCandidates::ERROR_ROLE);
        }

        checkSelectedAll();
        refreshBackupCandidatesErrors();
    }
}

void BackupCandidatesController::onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings)
{
    Q_UNUSED(syncSettings);
    refreshBackupCandidatesErrors();
}

void BackupCandidatesController::onFolderSizeReceived(QString folder, int size)
{
    auto backupCandidate = mBackupCandidates->getBackupCandidateByFolder(folder);
    if (backupCandidate)
    {
        setData(backupCandidate, size, BackupCandidates::SIZE_ROLE);

        if (size != FileFolderAttributes::NOT_READY)
        {
            updateSelectedAndTotalSize();
            updateModel(BackupCandidates::SIZE_ROLE, backupCandidate);
            updateModel(BackupCandidates::SIZE_READY_ROLE, backupCandidate);
        }
    }
}

void BackupCandidatesController::clean(bool resetErrors)
{
    auto row(-1);
    foreach(auto candidate, mBackupCandidates->getBackupCandidates())
    {
        row++;

        if (!candidate->mSelected)
        {
            continue;
        }

        if (candidate->mDone)
        {
            emit beginRemoveRows(row, row);
            mBackupCandidates->removeBackupCandidate(candidate->mFolder);
            mBackupCandidatesSizeRequester->removeFolder(candidate->mFolder);
            emit endRemoveRows();
        }
        else
        {
            if (resetErrors)
            {
                setData(candidate,
                        BackupCandidates::BackupErrorCode::NONE,
                        BackupCandidates::FOLDER_ROLE);
            }
        }
    }
}

std::shared_ptr<BackupCandidates> BackupCandidatesController::getBackupCandidates()
{
    return mBackupCandidates;
}

void BackupCandidatesController::updateModel(
    QVector<int> roles,
    std::shared_ptr<BackupCandidates::Data> backupCandidate)
{
    auto row = mBackupCandidates->getBackupCandidates().indexOf(backupCandidate);
    DataController::updateModel(row, 0, roles);
}

void BackupCandidatesController::updateModel(
    int role,
    std::shared_ptr<BackupCandidates::Data> backupCandidate)
{
    updateModel(QVector<int>() << role, backupCandidate);
}

QStringList BackupCandidatesController::getSelectedCandidates() const
{
    QStringList selectedCandidates;

    for (int row = 0; row < mBackupCandidates->getSize(); row++)
    {
        if (data(row, BackupCandidates::SELECTED_ROLE).toBool())
        {
            auto candidate = mBackupCandidates->getBackupCandidate(row);
            if (candidate)
            {
                selectedCandidates.append(candidate->mFolder);
            }
        }
    }

    return selectedCandidates;
}

void BackupCandidatesController::createBackups(int syncOrigin)
{
    if (!checkDirectories())
    {
        return;
    }

    // All expected errors have been handled
    BackupsController::BackupInfoList candidateList;
    for (int row = 0; row < mBackupCandidates->getSize(); row++)
    {
        if (data(row, BackupCandidates::SELECTED_ROLE).toBool())
        {
            BackupsController::BackupInfo candidate;
            candidate.first = data(row, BackupCandidates::FOLDER_ROLE).toString();
            candidate.second = data(row, BackupCandidates::NAME_ROLE).toString();
            candidateList.append(candidate);
        }
    }

    BackupsController::instance().addBackups(candidateList,
                                             static_cast<SyncInfo::SyncOrigin>(syncOrigin));
}

void BackupCandidatesController::onBackupsCreationFinished(bool success)
{
    if (success)
    {
        clean();
    }
    else
    {
        reviewConflicts();
    }

    emit backupsCreationFinished(success);
}

void BackupCandidatesController::onBackupFinished(const QString& folder,
                                                  int errorCode,
                                                  int syncErrorCode)
{
    auto backupCandidate = mBackupCandidates->getBackupCandidateByFolder(folder);
    if (backupCandidate)
    {
        if (errorCode == mega::MegaError::API_OK)
        {
            setData(backupCandidate, true, BackupCandidates::DONE_ROLE);
        }
        else
        {
            backupCandidate->mSdkError = errorCode;
            backupCandidate->mSyncError = syncErrorCode;
            setData(backupCandidate,
                    BackupCandidates::BackupErrorCode::SDK_CREATION,
                    BackupCandidates::ERROR_ROLE);
        }
    }
}

bool BackupCandidatesController::checkDirectories()
{
    bool success = true;
    bool reviewErrors = false;
    for (int index = 0; index < mBackupCandidates->getSize(); index++)
    {
        auto backupCandidate = mBackupCandidates->getBackupCandidate(index);

        if (backupCandidate->mSelected && !backupCandidate->mDone &&
            backupCandidate->mError != BackupCandidates::BackupErrorCode::SDK_CREATION)
        {
            if (!QDir(backupCandidate->mFolder).exists())
            {
                setData(backupCandidate,
                        BackupCandidates::BackupErrorCode::UNAVAILABLE_DIR,
                        BackupCandidates::ERROR_ROLE);
                success = false;
            }
            else if (backupCandidate->mError == BackupCandidates::BackupErrorCode::UNAVAILABLE_DIR)
            {
                // If actual error is UNAVAILABLE_DIR and it could be located again
                // Then, clean this error
                setData(backupCandidate,
                        BackupCandidates::BackupErrorCode::NONE,
                        BackupCandidates::ERROR_ROLE);
                reviewErrors = true;
            }
        }
    }

    if (reviewErrors)
    {
        // If one or more UNAVAILABLE_DIR errors have been reverted
        // Then we need to check all the conflicts again
        refreshBackupCandidatesErrors();
    }
    else
    {
        // Only review the global conflict
        reviewConflicts();
    }

    return success;
}

QString BackupCandidatesController::getSdkErrorString() const
{
    QString message = QCoreApplication::translate("BackupsModel",
                                                  "Folder wasn't backed up. Try again.",
                                                  "",
                                                  mBackupCandidates->getSDKConflictCount());
    auto candidateList(mBackupCandidates->getBackupCandidates());
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

QString BackupCandidatesController::getSyncErrorString() const
{
    QString message;
    auto candidateList(mBackupCandidates->getBackupCandidates());
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
        BackupsController::instance().isLocalFolderSyncable((*itFound)->mFolder,
                                                            mega::MegaSync::TYPE_BACKUP,
                                                            message);
    }

    return message;
}

void BackupCandidatesController::createConflictsNotificationText(
    BackupCandidates::BackupErrorCode error)
{
    QString errorMessage;

    switch (error)
    {
        case BackupCandidates::BackupErrorCode::DUPLICATED_NAME:
        {
            errorMessage =
                QCoreApplication::translate("BackupsModel",
                                            "You can't back up folders with the same name. "
                                            "Rename them to continue with the backup. "
                                            "Folder names won't change on your computer.");
            break;
        }
        case BackupCandidates::BackupErrorCode::EXISTS_REMOTE:
        {
            errorMessage = QCoreApplication::translate(
                "BackupsModel",
                "A folder with the same name already exists in your Backups. "
                "Rename the new folder to continue with the backup. "
                "Folder name will not change on your computer.",
                "",
                mBackupCandidates->getRemoteConflictCount());
            break;
        }
        case BackupCandidates::BackupErrorCode::SYNC_CONFLICT:
        {
            errorMessage = getSyncErrorString();
            break;
        }
        case BackupCandidates::BackupErrorCode::PATH_RELATION:
        {
            errorMessage = QCoreApplication::translate(
                "BackupsModel",
                "Backup folders can't contain or be contained by other backup folder");
            break;
        }
        case BackupCandidates::BackupErrorCode::UNAVAILABLE_DIR:
        {
            errorMessage = QCoreApplication::translate(
                "BackupsModel",
                "Folder can't be backed up as it can't be located. "
                "It may have been moved or deleted, or you might not have access.");
            break;
        }
        case BackupCandidates::BackupErrorCode::SDK_CREATION:
        {
            errorMessage = getSdkErrorString();
            break;
        }
        default:
        {
        }
    }

    mBackupCandidates->setConflictsNotificationText(errorMessage);
}
