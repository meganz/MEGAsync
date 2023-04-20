#include "BackupFolderModel.h"

#include "MegaApi.h"
#include "Utilities.h"
#include "syncs/control/SyncController.h"

BackupFolder::BackupFolder()
    : display(QString::fromUtf8(""))
    , tooltip(QString::fromUtf8(""))
    , folder(QString::fromUtf8(""))
    , size(QString::fromUtf8(""))
    , selected(false)
    , selectable(true)
    , confirmed(false)
    , done(false)
    , error(0)
    , folderSize(0)
{
}

BackupFolder::BackupFolder(const QString& folder,
                           const QString& displayName,
                           bool selected)
    : display(displayName)
    , tooltip(folder)
    , folder(folder)
    , size(QString::fromUtf8(""))
    , selected(selected)
    , selectable(true)
    , confirmed(false)
    , done(false)
    , error(0)
    , folderSize(0)
{
    Utilities::getFolderSize(folder, &folderSize);
    size = Utilities::getSizeString(folderSize);
}

BackupFolderModel::BackupFolderModel(QObject* parent)
    : QAbstractListModel(parent)
    , mRoleNames(QAbstractItemModel::roleNames())
    , mSelectedRowsTotal(0)
    , mTotalSize(0)
{
    mRoleNames[FolderRole] = "folder";
    mRoleNames[SizeRole] = "size";
    mRoleNames[SelectedRole] = "selected";
    mRoleNames[SelectableRole] = "selectable";
    mRoleNames[ConfirmedRole] = "confirm";
    mRoleNames[DoneRole] = "done";
    mRoleNames[ErrorRole] = "error";

    // Append mBackupFolderList with the default dirs
    populateDefaultDirectoryList();
}

void BackupFolderModel::populateDefaultDirectoryList()
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
        if(dir.exists() && dir != QDir::home() && isValidBackupFolder(path))
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

void BackupFolderModel::checkSelectedAll(const BackupFolder& item)
{
    if(item.selected)
    {
        mSelectedRowsTotal++;
        mTotalSize += item.folderSize;
    }
    else
    {
        mSelectedRowsTotal--;
        mTotalSize -= item.folderSize;
    }

    emit rowSelectedChanged(mSelectedRowsTotal > 0 && mSelectedRowsTotal < mBackupFolderList.size(),
                            mSelectedRowsTotal == mBackupFolderList.size());
}

QHash<int, QByteArray> BackupFolderModel::roleNames() const
{
    return mRoleNames;
}

int BackupFolderModel::rowCount(const QModelIndex& parent) const
{
    // When implementing a table based model, rowCount() should return 0 when the parent is valid.
    return parent.isValid() ? 0 : mBackupFolderList.size();
}

bool BackupFolderModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool result = hasIndex(index.row(), index.column(), index.parent()) && value.isValid();

    if (result)
    {
        BackupFolder& item = mBackupFolderList[index.row()];
        switch (role)
        {
            case Qt::DisplayRole:
                item.display = value.toString();
                break;
            case Qt::ToolTipRole:
                item.tooltip = value.toString();
                break;
            case FolderRole:
                item.folder = value.toString();
                break;
            case SizeRole:
                item.size = value.toInt();
                break;
            case SelectedRole:
            {
                bool selected = value.toBool();
                bool isCkecked = (selected && isValidBackupFolder(item.folder));
                if(isCkecked || !selected) {
                    item.selected = selected;
                    if(isCkecked) {
                        changeOtherBackupFolders(item.folder, false);
                    } else if(!selected) {
                        changeOtherBackupFolders(item.folder, true);
                    }
                    checkSelectedAll(item);
                }
                break;
            }
            case SelectableRole:
                item.selectable = value.toBool();
                break;
            case ConfirmedRole:
                item.confirmed = value.toBool();
                break;
            case DoneRole:
                item.done = value.toBool();
                break;
            case ErrorRole:
                item.error = value.toInt();
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

QVariant BackupFolderModel::data(const QModelIndex &index, int role) const
{
    QVariant field;

    if (hasIndex(index.row(), index.column(), index.parent()))
    {
        const BackupFolder& item = mBackupFolderList.at(index.row());
        switch (role)
        {
            case Qt::DisplayRole:
                field = item.display;
                break;
            case Qt::ToolTipRole:
                field = item.tooltip;
                break;
            case FolderRole:
                field = item.folder;
                break;
            case SizeRole:
                field = item.size;
                break;
            case SelectedRole:
                field = item.selected;
                break;
            case SelectableRole:
                field = item.selectable;
                break;
            case ConfirmedRole:
                field = item.confirmed;
                break;
            case DoneRole:
                field = item.done;
                break;
            case ErrorRole:
                field = item.error;
                break;
            default:
                break;
        }
    }

    return field;
}

bool BackupFolderModel::isValidBackupFolder(const QString& inputPath)
{
    QString message;
    auto syncability = SyncController::isLocalFolderSyncable(inputPath, mega::MegaSync::TYPE_BACKUP, message);
    if(syncability != SyncController::CANT_SYNC)
    {
        int row = 0;
        while (syncability != SyncController::CANT_SYNC && row < rowCount())
        {
            QString existingPath(mBackupFolderList[row].folder);
            if(mBackupFolderList[row].selected && isBackupFolderValid(inputPath, existingPath))
            {
                syncability = SyncController::CANT_SYNC;
            }
            row++;
        }
    }

    return (syncability != SyncController::CANT_SYNC);
}

bool BackupFolderModel::isValidInsertion(const QString& inputPath)
{
    QString message;
    auto syncability = SyncController::isLocalFolderSyncable(inputPath, mega::MegaSync::TYPE_BACKUP, message);
    if(syncability != SyncController::CANT_SYNC)
    {
        int row = 0;
        while (syncability != SyncController::CANT_SYNC && row < rowCount())
        {
            QString existingPath(mBackupFolderList[row].folder);
            if (inputPath == existingPath)
            {
                if(!mBackupFolderList[row].selected)
                {
                    setData(index(row, 0), QVariant(true), SelectedRole);
                }
                syncability = SyncController::CANT_SYNC;
            }
            row++;
        }
    }

    return (syncability != SyncController::CANT_SYNC);
}

QString BackupFolderModel::getToolTipErrorText(const QString& folder,
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

bool BackupFolderModel::folderContainsOther(const QString& folder,
                                            const QString& other) const
{
    return folder.startsWith(other) && folder[other.size()] == QDir::separator();
}

void BackupFolderModel::changeOtherBackupFolders(const QString& folder,
                                                 bool enable)
{
    QVector<int> roles = { SelectableRole, Qt::ToolTipRole };
    if(!enable)
    {
        roles.append(SelectedRole);
    }

    QModelIndex modelIndex;
    for (int row = 0; row < rowCount(); row++)
    {
        QString existingPath(mBackupFolderList[row].folder);
        if ((folder != existingPath) && isBackupFolderValid(folder, existingPath))
        {
            modelIndex = index(row, 0);
            if(enable)
            {
                if(canBackupFolderEnabled(existingPath, folder))
                {
                    mBackupFolderList[row].selectable = true;
                    mBackupFolderList[row].tooltip = mBackupFolderList[row].folder;
                    emit dataChanged(modelIndex, modelIndex, roles);
                }
            }
            else
            {
                if(mBackupFolderList[row].selected)
                {
                    mSelectedRowsTotal--;
                    mTotalSize -= mBackupFolderList[row].folderSize;
                }
                mBackupFolderList[row].selectable = false;
                mBackupFolderList[row].selected = false;
                mBackupFolderList[row].tooltip = getToolTipErrorText(existingPath, folder);
                emit dataChanged(modelIndex, modelIndex, roles);
            }
        }
    }
}

bool BackupFolderModel::isBackupFolderValid(const QString& folder,
                                            const QString& existingPath) const
{
    return folderContainsOther(folder, existingPath) || folderContainsOther(existingPath, folder);
}

bool BackupFolderModel::canBackupFolderEnabled(const QString& folder,
                                               const QString& selectedFolder) const {
    bool success = true;
    int row = 0;
    while(success && row < rowCount())
    {
        QString existingPath(mBackupFolderList[row].folder);
        if (folder != existingPath && selectedFolder != existingPath)
        {
            success = !isBackupFolderValid(folder, existingPath)
                      || (isBackupFolderValid(folder, existingPath) && !mBackupFolderList[row].selected);
        }
        row++;
    }
    return success;
}

int BackupFolderModel::calculateNumSelectedRows() const
{
    int selectedCount = 0;
    for (int row = 0; row < rowCount(); row++)
    {
        if(mBackupFolderList[row].selected)
        {
            selectedCount++;
        }
    }
    return selectedCount;
}

void BackupFolderModel::insertFolder(const QString &folder)
{
    QString inputPath(QDir::toNativeSeparators(QDir(folder).absolutePath()));
    if(!isValidInsertion(inputPath))
    {
        return;
    }

    beginInsertRows(QModelIndex(), 0, 0);
    BackupFolder data(inputPath, mSyncController.getSyncNameFromPath(inputPath));
    mBackupFolderList.prepend(data);
    endInsertRows();
    changeOtherBackupFolders(inputPath, false);
    checkSelectedAll(data);
}

void BackupFolderModel::setAllSelected(bool selected)
{
    QModelIndex modelIndex;

    for (int row = 0; row < rowCount(); row++)
    {
        modelIndex = index(row, 0);
        if(selected)
        {
            if(mBackupFolderList[row].selectable && !mBackupFolderList[row].selected)
            {
                mBackupFolderList[row].selected = true;
                emit dataChanged(modelIndex, modelIndex, { SelectedRole } );
                changeOtherBackupFolders(mBackupFolderList[row].folder, false);
                mTotalSize += mBackupFolderList[row].folderSize;
            }
        }
        else
        {
            if(mBackupFolderList[row].selected)
            {
                mTotalSize -= mBackupFolderList[row].folderSize;
            }
            mBackupFolderList[row].selected = false;
            mBackupFolderList[row].selectable = true;
            emit dataChanged(modelIndex, modelIndex, { SelectedRole, SelectableRole } );
        }
    }

    mSelectedRowsTotal = (selected ? calculateNumSelectedRows() : 0);
}

int BackupFolderModel::getNumSelectedRows() const
{
    return mSelectedRowsTotal;
}

QString BackupFolderModel::getTotalSize() const
{
    return Utilities::getSizeString(mTotalSize);
}

void BackupFolderModel::updateConfirmed()
{
    for (int row = 0; row < rowCount(); row++)
    {
        mBackupFolderList[row].confirmed = mBackupFolderList[row].selected;
    }
}

QStringList BackupFolderModel::getConfirmedDirs() const
{
    QStringList dirs;
    for (int row = 0; row < rowCount(); row++)
    {
        if (mBackupFolderList[row].confirmed)
        {
            dirs.append(mBackupFolderList[row].folder);
        }
    }
    return dirs;
}

void BackupFolderModel::clean()
{
    QStringList removedFolders;
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    while (item != mBackupFolderList.end())
    {
        if(item->done)
        {
            removedFolders.append(item->folder);
            const auto row = std::distance(mBackupFolderList.begin(), item);
            beginRemoveRows(QModelIndex(), row, row);
            mBackupFolderList.erase(item);
            endRemoveRows();
        }
        item++;
    }

    std::for_each(removedFolders.begin(), removedFolders.end(), [this](const QString& folder) {
        changeOtherBackupFolders(folder, true);
    });
    setAllSelected(false);
    emit rowSelectedChanged(mSelectedRowsTotal > 0 && mSelectedRowsTotal < mBackupFolderList.size(),
                            mSelectedRowsTotal == mBackupFolderList.size());
}

void BackupFolderModel::update(const QString& path, int errorCode)
{
    QModelIndex modelIndex;
    for (int row = 0; row < rowCount(); row++)
    {
        if(mBackupFolderList[row].folder == path)
        {
            modelIndex = index(row, 0);
            if(errorCode == mega::MegaError::API_OK)
            {
                mBackupFolderList[row].done = true;
                emit dataChanged(modelIndex, modelIndex, { DoneRole } );
            }
            else
            {
                mBackupFolderList[row].error = errorCode;
                emit dataChanged(modelIndex, modelIndex, { ErrorRole } );
            }
        }
    }
}

BackupFolderFilterProxyModel::BackupFolderFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
    , mSelectedFilterEnabled(false)
{
    setDynamicSortFilter(true);
}

bool BackupFolderFilterProxyModel::selectedFilterEnabled() const
{
    return mSelectedFilterEnabled;
}

void BackupFolderFilterProxyModel::setSelectedFilterEnabled(bool enabled)
{
    if(mSelectedFilterEnabled == enabled) {
        return;
    }

    mSelectedFilterEnabled = enabled;
    emit selectedFilterEnabledChanged();

    invalidateFilter();
}

bool BackupFolderFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if(!mSelectedFilterEnabled) {
        return true;
    }

    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    return index.data(BackupFolderModel::BackupFolderRoles::ConfirmedRole).toBool();
}

