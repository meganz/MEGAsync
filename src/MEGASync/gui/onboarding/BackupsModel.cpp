
#include "BackupsModel.h"

#include "megaapi.h"
#include "Utilities.h"
#include "syncs/control/SyncController.h"
#include "syncs/control/SyncInfo.h"
#include "MegaApplication.h"

#include <QQmlContext>

BackupFolder::BackupFolder()
    : mName(QString::fromUtf8(""))
    , mTooltip(QString::fromUtf8(""))
    , mFolder(QString::fromUtf8(""))
    , mSize(QString::fromUtf8(""))
    , mSelected(false)
    , mSelectable(true)
    , mDone(false)
    , mError(0)
    , mErrorVisible(false)
    , folderSize(0)
{
}

BackupFolder::BackupFolder(const QString& folder,
                           const QString& displayName,
                           bool selected)
    : mName(displayName)
    , mTooltip(folder)
    , mFolder(folder)
    , mSize(QString::fromUtf8(""))
    , mSelected(selected)
    , mSelectable(true)
    , mDone(false)
    , mError(0)
    , mErrorVisible(false)
    , folderSize(0)
{
    Utilities::getFolderSize(folder, &folderSize);
    mSize = Utilities::getSizeString(folderSize);
}

BackupsModel::BackupsModel(QObject* parent)
    : QAbstractListModel(parent)
    , mRoleNames(QAbstractItemModel::roleNames())
    , mSelectedRowsTotal(0)
    , mBackupsTotalSize(0)
    , mBackupsController(new BackupsController(this))
    , mConflictsSize(0)
    , mCheckAllState(Qt::CheckState::Unchecked)
{
    mRoleNames[NameRole] = "mName";
    mRoleNames[FolderRole] = "mFolder";
    mRoleNames[SizeRole] = "mSize";
    mRoleNames[SelectedRole] = "mSelected";
    mRoleNames[SelectableRole] = "mSelectable";
    mRoleNames[DoneRole] = "mDone";
    mRoleNames[ErrorRole] = "mError";
    mRoleNames[ErrorVisibleRole] = "mErrorVisible";

    // Append mBackupFolderList with the default dirs
    populateDefaultDirectoryList();

    connect(SyncInfo::instance(), &SyncInfo::syncRemoved, this, &BackupsModel::onSyncRemoved);
    connect(SyncInfo::instance(), &SyncInfo::syncStateChanged, this, &BackupsModel::onSyncChanged);
    connect(mBackupsController, &BackupsController::backupsCreationFinished, this, &BackupsModel::clean);

    MegaSyncApp->qmlEngine()->rootContext()->setContextProperty(QString::fromUtf8("BackupsModel"), this);
    MegaSyncApp->qmlEngine()->rootContext()->setContextProperty(QString::fromUtf8("BackupsController"), mBackupsController);
}

QHash<int, QByteArray> BackupsModel::roleNames() const
{
    return mRoleNames;
}

int BackupsModel::rowCount(const QModelIndex& parent) const
{
    // When implementing a table based model, rowCount() should return 0 when the parent is valid.
    return parent.isValid() ? 0 : mBackupFolderList.size();
}

bool BackupsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool result = hasIndex(index.row(), index.column(), index.parent()) && value.isValid();

    if (result)
    {
        BackupFolder& item = mBackupFolderList[index.row()];
        switch (role)
        {
            case Qt::ToolTipRole:
                item.mTooltip = value.toString();
                break;
            case NameRole:
                item.mName = value.toString();
                break;
            case FolderRole:
                item.mFolder = value.toString();
                break;
            case SizeRole:
                item.mSize = value.toInt();
                break;
            case SelectedRole:
            {
                bool selected = value.toBool();
                if(!selected || (selected && isLocalFolderSyncable(item.mFolder)))
                {
                    item.mSelected = selected;
                    reviewOthers(item.mFolder, !selected);
                    checkSelectedAll();
                }
                break;
            }
            case SelectableRole:
                item.mSelectable = value.toBool();
                break;
            case DoneRole:
                item.mDone = value.toBool();
                break;
            case ErrorRole:
                item.mError = value.toInt();
                break;
            case ErrorVisibleRole:
                item.mErrorVisible = value.toBool();
                break;
            default:
                result = false;
                break;
        }

        if(result)
        {
            emit dataChanged(index, index, { role } );
        }
    }

    return result;
}

QVariant BackupsModel::data(const QModelIndex &index, int role) const
{
    QVariant field;

    if (hasIndex(index.row(), index.column(), index.parent()))
    {
        const BackupFolder& item = mBackupFolderList.at(index.row());
        switch (role)
        {
            case Qt::ToolTipRole:
                field = item.mTooltip;
                break;
            case NameRole:
                field = item.mName;
                break;
            case FolderRole:
                field = item.mFolder;
                break;
            case SizeRole:
                field = item.mSize;
                break;
            case SelectedRole:
                field = item.mSelected;
                break;
            case SelectableRole:
                field = item.mSelectable;
                break;
            case DoneRole:
                field = item.mDone;
                break;
            case ErrorRole:
                field = item.mError;
                break;
            case ErrorVisibleRole:
                field = item.mErrorVisible;
                break;
            default:
                break;
        }
    }

    return field;
}

QString BackupsModel::getTotalSize() const
{
    return Utilities::getSizeString(mBackupsTotalSize);
}

Qt::CheckState BackupsModel::getCheckAllState() const
{
    return mCheckAllState;
}

void BackupsModel::setCheckAllState(Qt::CheckState state, bool fromModel)
{
    if(!fromModel && mCheckAllState == Qt::CheckState::Unchecked && state == Qt::CheckState::PartiallyChecked)
    {
        state = Qt::CheckState::Checked;
    }

    if(mCheckAllState == state)
    {
        return;
    }

    if(mCheckAllState != Qt::CheckState::Checked && state == Qt::CheckState::Checked)
    {
        setAllSelected(true);
    }
    else if(mCheckAllState != Qt::CheckState::Unchecked && state == Qt::CheckState::Unchecked)
    {
        setAllSelected(false);
    }
    mCheckAllState = state;
    emit checkAllStateChanged();
}

BackupsController* BackupsModel::backupsController() const
{
    return mBackupsController;
}

bool BackupsModel::getExistConflicts() const
{
    return mConflictsSize > 0;
}

QString BackupsModel::getConflictsNotificationText() const
{
    return mConflictsNotificationText;
}

void BackupsModel::insertFolder(const QString &folder)
{
    QString inputPath(QDir::toNativeSeparators(QDir(folder).absolutePath()));
    if(selectIfExistsInsertion(inputPath))
    {
        // If the folder exists in the table then select the item and return
        return;
    }

    QString message;
    auto syncability = SyncController::isLocalFolderSyncable(inputPath, mega::MegaSync::TYPE_BACKUP, message);
    if(syncability == SyncController::CANT_SYNC)
    {
        // This element can not be selected
        BackupFolder data(inputPath, mSyncController.getSyncNameFromPath(inputPath), false);
        data.mSelectable = false;
        data.mTooltip = message;

        beginInsertRows(QModelIndex(), 0, 0);
        mBackupFolderList.prepend(data);
        endInsertRows();
    }
    else
    {
        BackupFolder data(inputPath, mSyncController.getSyncNameFromPath(inputPath));
        beginInsertRows(QModelIndex(), 0, 0);
        mBackupFolderList.prepend(data);
        endInsertRows();
        reviewOthers(inputPath, false);
        checkSelectedAll();
    }
}

void BackupsModel::setAllSelected(bool selected)
{
    QList<BackupFolder>::iterator item = mBackupFolderList.end()-1;
    while (item != mBackupFolderList.begin()-1)
    {
        if(!selected || (selected && isLocalFolderSyncable(item->mFolder)))
        {
            item->mSelected = selected;
            reviewOthers(item->mFolder, !selected);
            emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectedRole } );
        }
        item--;
    }

    updateSelectedAndTotalSize();
}

void BackupsModel::populateDefaultDirectoryList()
{
    // Default directories definition
    QVector<QStandardPaths::StandardLocation> defaultPaths =
    {
        QStandardPaths::DesktopLocation,
        QStandardPaths::DocumentsLocation,
        QStandardPaths::MusicLocation,
        QStandardPaths::PicturesLocation
    };

    // Iterate defaultPaths to add to mBackupFolderList if the path is not empty
    for (auto type : defaultPaths)
    {
        const auto standardPaths (QStandardPaths::standardLocations(type));
        QDir dir (QDir::cleanPath(standardPaths.first()));
        QString path (QDir::toNativeSeparators(dir.canonicalPath()));
        if(dir.exists() && dir != QDir::home() && isLocalFolderSyncable(path))
        {
            mBackupFolderList.append(BackupFolder(path, mSyncController.getSyncNameFromPath(path), false));
        }
        else
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                               QString::fromUtf8("Default path %1 is not valid.").arg(path).toUtf8().constData());
        }
    }
}

void BackupsModel::updateSelectedAndTotalSize()
{
    mSelectedRowsTotal = 0;
    auto lastTotalSize = mBackupsTotalSize;
    mBackupsTotalSize = 0;
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    while (item != mBackupFolderList.end())
    {
        if(item->mSelected)
        {
            mSelectedRowsTotal++;
            mBackupsTotalSize += item->folderSize;
        }
        item++;
    }

    if(mBackupsTotalSize != lastTotalSize)
    {
        emit totalSizeChanged();
    }
}

void BackupsModel::checkSelectedAll()
{
    updateSelectedAndTotalSize();

    Qt::CheckState state = Qt::CheckState::PartiallyChecked;
    if(mSelectedRowsTotal == 0)
    {
        state = Qt::CheckState::Unchecked;
    }
    else if(mSelectedRowsTotal == mBackupFolderList.size())
    {
        state = Qt::CheckState::Checked;
    }
    setCheckAllState(state, true);
}

bool BackupsModel::isLocalFolderSyncable(const QString& inputPath)
{
    QString message;
    return (SyncController::isLocalFolderSyncable(inputPath, mega::MegaSync::TYPE_BACKUP, message) != SyncController::CANT_SYNC);
}

bool BackupsModel::selectIfExistsInsertion(const QString& inputPath)
{
    bool exists = false;
    int row = 0;

    while (!exists && row < rowCount())
    {
        QString existingPath(mBackupFolderList[row].mFolder);
        if ((exists = (inputPath == existingPath)))
        {
            if(!mBackupFolderList[row].mSelected)
            {
                setData(index(row, 0), QVariant(true), SelectedRole);
            }
        }
        row++;
    }

    return exists;
}

QString BackupsModel::getToolTipErrorText(const QString& folder,
                                          const QString& existingPath) const
{
    QString message(QString::fromUtf8(""));
    if (folderContainsOther(folder, existingPath))
    {
        message = SyncController::getErrStrCurrentBackupInsideExistingBackup();
    }
    else if (folderContainsOther(existingPath, folder))
    {
        message = SyncController::getErrStrCurrentBackupOverExistingBackup();
    }
    return message;
}

bool BackupsModel::folderContainsOther(const QString& folder,
                                            const QString& other) const
{
    return folder.startsWith(other) && folder[other.size()] == QDir::separator();
}

bool BackupsModel::isRelatedFolder(const QString& folder,
                                        const QString& existingPath) const
{
    return folderContainsOther(folder, existingPath) || folderContainsOther(existingPath, folder);
}

QModelIndex BackupsModel::getModelIndex(QList<BackupFolder>::iterator item)
{
    const auto row = std::distance(mBackupFolderList.begin(), item);
    return QModelIndex(index(row, 0));
}

void BackupsModel::reviewOthers(const QString& folder,
                                     bool selectable)
{
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    while (item != mBackupFolderList.end())
    {
        QString existingPath(item->mFolder);
        if ((folder != existingPath) && isRelatedFolder(folder, existingPath))
        {
            QString message;
            auto syncability = SyncController::isLocalFolderSyncable(existingPath, mega::MegaSync::TYPE_BACKUP, message);
            if(!item->mSelectable && selectable && (syncability != SyncController::CANT_SYNC)
                && !existAnotherBackupFolderRelated(existingPath, folder))
            {
                item->mSelectable = true;
                item->mTooltip = item->mFolder;
                emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectableRole, Qt::ToolTipRole });
            }
            else if(item->mSelectable && !selectable)
            {
                item->mSelectable = false;
                item->mSelected = false;
                item->mTooltip = message.isEmpty() ? getToolTipErrorText(existingPath, folder) : message;
                emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectableRole, Qt::ToolTipRole, SelectedRole });
                reviewOthers(existingPath, true);
            }
        }
        item++;
    }
}

void BackupsModel::reviewOthersWhenRemoved(const QString& folder)
{
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    while (item != mBackupFolderList.end())
    {
        QString existingPath(item->mFolder);
        if (!item->mSelectable
                && isRelatedFolder(folder, existingPath)
                && !existAnotherBackupFolderRelated(existingPath, folder)
                && isLocalFolderSyncable(existingPath))
        {
            item->mSelectable = true;
            item->mTooltip = item->mFolder;
            emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectableRole, Qt::ToolTipRole });
        }
        item++;
    }
}

bool BackupsModel::existAnotherBackupFolderRelated(const QString& folder,
                                                   const QString& selectedFolder) const {
    bool exists = false;
    int row = 0;
    while(!exists && row < rowCount())
    {
        QString existingPath(mBackupFolderList[row].mFolder);
        if (folder != existingPath && selectedFolder != existingPath)
        {
            exists = isRelatedFolder(folder, existingPath) && mBackupFolderList[row].mSelected;
        }
        row++;
    }
    return exists;
}

void BackupsModel::updateBackupFolder(QList<BackupFolder>::iterator item,
                                      bool selectable,
                                      const QString& message)
{
    bool toSelectable = !item->mSelectable && selectable;
    bool toUnselectable = item->mSelectable && !selectable;
    if(!toSelectable && !toUnselectable)
    {
        return;
    }

    if(toSelectable)
    {
        item->mTooltip = item->mFolder;
    }
    else
    {
        item->mTooltip = message;
    }
    item->mSelectable = selectable;

    emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectableRole, Qt::ToolTipRole } );
}

void BackupsModel::reviewAllBackupFolders()
{
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    while (item != mBackupFolderList.end())
    {
        QString message;
        auto syncability = SyncController::isLocalFolderSyncable(item->mFolder, mega::MegaSync::TYPE_BACKUP, message);
        updateBackupFolder(item, syncability != SyncController::CANT_SYNC, message);
        item++;
    }
}

void BackupsModel::checkDuplicatedBackupNames(const QSet<QString>& candidateSet,
                                              const QStringList& candidateList)
{
    // Search the repeated strings (backup names)
    int repeatedStrings = candidateList.count() - candidateSet.count();
    int repeatedCounter = 0;
    int row = -1;
    while(repeatedCounter < repeatedStrings && ++row < rowCount())
    {
        if (mBackupFolderList[row].mSelected)
        {
            QString current(mBackupFolderList[row].mName);
            int i = row;
            bool isFirst = true;
            while(repeatedCounter < repeatedStrings && ++i < rowCount())
            {
                if (mBackupFolderList[i].mSelected
                        && mBackupFolderList[i].mName == current)
                {
                    if(isFirst)
                    {
                        setData(index(row, 0), QVariant(BackupErrorCode::DuplicatedName), ErrorRole);
                        isFirst = false;
                    }
                    setData(index(i, 0), QVariant(BackupErrorCode::DuplicatedName), ErrorRole);
                    repeatedCounter++;
                }
            }
        }
    }
}

void BackupsModel::reviewConflicts()
{
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    QString firstRemoteNameConflict = QString::fromUtf8("");
    int remoteCount = 0;
    int duplicatedCount = 0;

    while (item != mBackupFolderList.end())
    {
        if(item->mSelected)
        {
            if(item->mError == DuplicatedName)
            {
                duplicatedCount++;
            }
            else if(item->mError == ExistsRemote)
            {
                if(firstRemoteNameConflict.isEmpty())
                {
                    firstRemoteNameConflict = item->mName;
                }
                remoteCount++;
            }
        }
        item++;
    }

    mConflictsSize = duplicatedCount + remoteCount;

    mConflictsNotificationText = QString::fromUtf8("");
    if(duplicatedCount > 0)
    {
        mConflictsNotificationText = tr("You can't back up folders with the same name. Rename them to continue with the backup. Folder names won't change on your computer.");
    }
    else if(remoteCount == 1)
    {
        mConflictsNotificationText = tr("A folder named \"%1\" already exists in your Backups. Rename the new folder to continue with the backup. Folder name will not change on your computer.").arg(firstRemoteNameConflict);
    }
    else if(remoteCount > 1)
    {
        mConflictsNotificationText = tr("Some folders with the same name already exist in your Backups. Rename the new folders to continue with the backup. Folder names will not change on your computer.");
    }

    emit existConflictsChanged();
}

void BackupsModel::checkRemoteDuplicatedBackups(const QSet<QString>& candidateSet)
{
    QSet<QString> duplicatedSet = mBackupsController->getRemoteFolders();
    duplicatedSet.intersect(candidateSet);
    if(!duplicatedSet.isEmpty())
    {
        auto backup = duplicatedSet.begin();
        while(backup != duplicatedSet.end())
        {
            int row = -1;
            bool found = false;
            while(!found && ++row < rowCount())
            {
                if ((found = mBackupFolderList[row].mSelected
                                && mBackupFolderList[row].mName == *backup))
                {
                    setData(index(row, 0), QVariant(BackupErrorCode::ExistsRemote), ErrorRole);
                }
            }
            backup++;
        }
    }
}

void BackupsModel::checkBackups()
{
    QStringList candidateList;
    for (int row = 0; row < rowCount(); row++)
    {
        if (mBackupFolderList[row].mSelected)
        {
            candidateList.append(mBackupFolderList[row].mName);
        }
    }

    QSet<QString> candidateSet = QSet<QString>::fromList(candidateList);
    checkRemoteDuplicatedBackups(candidateSet);

    if (candidateSet.count() < candidateList.count())
    {
        checkDuplicatedBackupNames(candidateSet, candidateList);
    }

    reviewConflicts();
}

int BackupsModel::getRow(const QString& folder)
{
    int row = 0;
    bool found = false;
    while(!found && row < rowCount())
    {
        found = mBackupFolderList[row].mFolder == folder;
        if (!found)
        {
            row++;
        }
    }
    return row;
}

void BackupsModel::reviewOtherBackupErrors(const QString& name)
{
    bool error = false;
    int row = -1;
    while(!error && ++row < rowCount())
    {
        if(mBackupFolderList[row].mSelected
            && name == mBackupFolderList[row].mName
            && (mBackupFolderList[row].mError == BackupErrorCode::ExistsRemote
                || mBackupFolderList[row].mError == BackupErrorCode::DuplicatedName))
        {
            int i = row;
            bool found = false;
            while(!found && ++i < rowCount())
            {
                found = mBackupFolderList[i].mName == mBackupFolderList[row].mName;
            }
            if(!found)
            {
                setData(index(row, 0), QVariant(0), ErrorRole);
                setData(index(row, 0), QVariant(false), ErrorVisibleRole);
            }
        }
    }
}

bool BackupsModel::renameBackup(const QString& folder, const QString& name)
{
    int row = getRow(folder);
    QString originalName = mBackupFolderList[row].mName;
    bool hasError = false;
    QSet<QString> candidateSet;
    candidateSet.insert(name);
    QSet<QString> duplicatedSet = mBackupsController->getRemoteFolders();
    duplicatedSet.intersect(candidateSet);

    if(!duplicatedSet.isEmpty())
    {
        setData(index(row, 0), QVariant(BackupErrorCode::ExistsRemote), ErrorRole);
        setData(index(row, 0), QVariant(true), ErrorVisibleRole);
        hasError = true;
    }

    if(!hasError)
    {
        int i = -1;
        while(!hasError && ++i < rowCount())
        {
            hasError = (i != row && mBackupFolderList[row].mSelected && name == mBackupFolderList[i].mName);
            if (hasError)
            {
                setData(index(row, 0), QVariant(BackupErrorCode::DuplicatedName), ErrorRole);
                setData(index(row, 0), QVariant(true), ErrorVisibleRole);
            }
        }
    }

    if(!hasError)
    {
        setData(index(row, 0), QVariant(name), NameRole);
        setData(index(row, 0), QVariant(0), ErrorRole);
        setData(index(row, 0), QVariant(false), ErrorVisibleRole);
        reviewOtherBackupErrors(originalName);
        checkBackups();
    }
    else
    {
        reviewConflicts();
    }

    return !hasError;
}

void BackupsModel::remove(const QString& folder)
{
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    bool found = false;
    QString name;
    while (!found && item != mBackupFolderList.end())
    {
        if((found = item->mFolder == folder))
        {
            name = item->mName;
            const auto row = std::distance(mBackupFolderList.begin(), item);
            beginRemoveRows(QModelIndex(), row, row);
            item = mBackupFolderList.erase(item);
            endRemoveRows();
        }
        else
        {
            item++;
        }
    }

    if(found)
    {
        reviewConflicts();
        reviewOtherBackupErrors(name);
        reviewAllBackupFolders();
        checkSelectedAll();
    }
}

void BackupsModel::onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings)
{
    QString localFolder (syncSettings->getLocalFolder());
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    bool found = false;
    while (!found && item != mBackupFolderList.end())
    {
        if((found = (item->mFolder == localFolder)))
        {
            item->mSelectable = true;
            item->mTooltip = item->mFolder;
            emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectableRole, Qt::ToolTipRole });
            reviewOthers(localFolder, true);
            checkSelectedAll();
        }
        item++;
    }

    if(!found)
    {
        reviewOthersWhenRemoved(localFolder);
        checkSelectedAll();
    }
}

void BackupsModel::onSyncChanged(std::shared_ptr<SyncSettings> syncSettings)
{
    if(syncSettings->getType() != mega::MegaSync::SyncType::TYPE_TWOWAY
        || !syncSettings->isActive())
    {
        return;
    }

    QString localFolder (syncSettings->getLocalFolder());
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    bool found = false;
    while (!found && item != mBackupFolderList.end())
    {
        if((found = (item->mFolder == localFolder)))
        {
            QString message;
            auto syncability = SyncController::isLocalFolderSyncable(localFolder, mega::MegaSync::TYPE_BACKUP, message);
            if(syncability == SyncController::CANT_SYNC && item->mSelectable)
            {
                // This element can not be selected
                item->mSelected = false;
                item->mSelectable = false;
                item->mTooltip = message;
                emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectedRole, SelectableRole, Qt::ToolTipRole });
                reviewOthers(localFolder, false);
                checkSelectedAll();
            }
        }
        item++;
    }
}

void BackupsModel::clean()
{
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    while (item != mBackupFolderList.end())
    {
        if(item->mDone)
        {
            const auto row = std::distance(mBackupFolderList.begin(), item);
            beginRemoveRows(QModelIndex(), row, row);
            item = mBackupFolderList.erase(item);
            endRemoveRows();
        }
        else
        {
            item++;
        }
    }

    reviewAllBackupFolders();
    checkSelectedAll();
}

void BackupsModel::update(const QString& path, int errorCode)
{
    QModelIndex modelIndex;
    bool found = false;
    int row = 0;
    while(!found && row < rowCount())
    {
        if((found = (mBackupFolderList[row].mFolder == path)))
        {
            modelIndex = index(row, 0);
            if(errorCode == mega::MegaError::API_OK)
            {
                setData(index(row, 0), QVariant(true), DoneRole);
            }
            else
            {
                setData(index(row, 0), QVariant(errorCode), ErrorRole);
            }
        }
        row++;
    }
}

// ************************************************************************************************
// * BackupsProxyModel
// ************************************************************************************************

BackupsProxyModel::BackupsProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
    , mSelectedFilterEnabled(false)
{
    setSourceModel(new BackupsModel(this));
    setDynamicSortFilter(true);
}

bool BackupsProxyModel::selectedFilterEnabled() const
{
    return mSelectedFilterEnabled;
}

void BackupsProxyModel::setSelectedFilterEnabled(bool enabled)
{
    if(mSelectedFilterEnabled == enabled) {
        return;
    }

    mSelectedFilterEnabled = enabled;
    emit selectedFilterEnabledChanged();

    invalidateFilter();
}

bool BackupsProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if(!mSelectedFilterEnabled) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    return index.data(BackupsModel::BackupFolderRoles::SelectedRole).toBool();
}

void BackupsProxyModel::createBackups()
{
    BackupsController::BackupInfoList candidateList;
    for (int row = 0; row < rowCount(); row++)
    {
        BackupsController::BackupInfo candidate;
        candidate.first = index(row, 0).data(BackupsModel::BackupFolderRoles::FolderRole).toString();
        candidate.second = index(row, 0).data(BackupsModel::BackupFolderRoles::NameRole).toString();
        candidateList.append(candidate);
    }
    backupsModel()->backupsController()->addBackups(candidateList);
}

BackupsModel* BackupsProxyModel::backupsModel()
{
    return dynamic_cast<BackupsModel*>(sourceModel());
}
