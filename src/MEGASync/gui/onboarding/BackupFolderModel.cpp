
#include "BackupFolderModel.h"

#include "MegaApi.h"
#include "Utilities.h"
#include "syncs/control/SyncController.h"
#include "syncs/control/SyncInfo.h"

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
    , syncable(true)
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
    , syncable(true)
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

    connect(SyncInfo::instance(), &SyncInfo::syncRemoved, this, &BackupFolderModel::onSyncRemoved);
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

void BackupFolderModel::updateSelectedAndTotalSize()
{
    mSelectedRowsTotal = 0;
    mTotalSize = 0;
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    while (item != mBackupFolderList.end())
    {
        if(item->selected)
        {
            mSelectedRowsTotal++;
            mTotalSize += item->folderSize;
        }
        item++;
    }
}

void BackupFolderModel::checkSelectedAll()
{
    updateSelectedAndTotalSize();

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
                if(!selected || (selected && isLocalFolderSyncable(item.folder)))
                {
                    item.selected = selected;
                    reviewOthers(item.folder, !selected);
                    checkSelectedAll();
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

bool BackupFolderModel::isLocalFolderSyncable(const QString& inputPath)
{
    QString message;
    return (SyncController::isLocalFolderSyncable(inputPath, mega::MegaSync::TYPE_BACKUP, message) != SyncController::CANT_SYNC);
}

bool BackupFolderModel::selectIfExistsInsertion(const QString& inputPath)
{
    bool exists = false;
    int row = 0;

    while (!exists && row < rowCount())
    {
        QString existingPath(mBackupFolderList[row].folder);
        if ((exists = (inputPath == existingPath)))
        {
            if(!mBackupFolderList[row].selected)
            {
                setData(index(row, 0), QVariant(true), SelectedRole);
            }
        }
        row++;
    }

    return exists;
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

void BackupFolderModel::onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings)
{
    QString localFolder (syncSettings->getLocalFolder());
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    while (item != mBackupFolderList.end())
    {
        if(item->folder == localFolder)
        {
            const auto row = std::distance(mBackupFolderList.begin(), item);
            QModelIndex modelIndex(index(row, 0));
            item->syncable = true;
            item->selectable = true; // TODO: change by real value
            item->tooltip = item->folder;
            emit dataChanged(modelIndex, modelIndex, { SelectableRole, Qt::ToolTipRole });
        }
        item++;
    }
}

bool BackupFolderModel::folderContainsOther(const QString& folder,
                                            const QString& other) const
{
    return folder.startsWith(other) && folder[other.size()] == QDir::separator();
}

bool BackupFolderModel::isRelatedFolder(const QString& folder,
                                            const QString& existingPath) const
{
    return folderContainsOther(folder, existingPath) || folderContainsOther(existingPath, folder);
}

QModelIndex BackupFolderModel::getModelIndex(QList<BackupFolder>::iterator item)
{
    const auto row = std::distance(mBackupFolderList.begin(), item);
    return QModelIndex(index(row, 0));
}

void BackupFolderModel::reviewOthers(const QString& folder,
                                     bool selectable,
                                     bool force)
{
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    while (item != mBackupFolderList.end())
    {
        QString existingPath(item->folder);
        if ((folder != existingPath) && isRelatedFolder(folder, existingPath) && isLocalFolderSyncable(existingPath))
        {
            if(!item->selectable && selectable && (!existAnotherBackupFolderRelated(existingPath, folder) || force))
            {
                item->selectable = true;
                item->tooltip = item->folder;
                emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectableRole, Qt::ToolTipRole });
            }
            else if(item->selectable && !selectable)
            {
                item->selectable = false;
                item->selected = false;
                item->tooltip = getToolTipErrorText(existingPath, folder);
                emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectableRole, Qt::ToolTipRole, SelectedRole });
                reviewOthers(existingPath, true, true);
            }
        }
        item++;
    }
}

bool BackupFolderModel::existAnotherBackupFolderRelated(const QString& folder,
                                                        const QString& selectedFolder) const {
    bool exists = false;
    int row = 0;
    while(!exists && row < rowCount())
    {
        QString existingPath(mBackupFolderList[row].folder);
        if (folder != existingPath && selectedFolder != existingPath)
        {
            exists = isRelatedFolder(folder, existingPath) && mBackupFolderList[row].selected;
        }
        row++;
    }
    return exists;
}

void BackupFolderModel::insertFolder(const QString &folder)
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
        data.syncable = false;
        data.selectable = false;
        data.tooltip = message;

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

void BackupFolderModel::setAllSelected(bool selected)
{
    QList<BackupFolder>::iterator item = mBackupFolderList.end()-1;
    while (item != mBackupFolderList.begin()-1)
    {
        if(!selected || (selected && isLocalFolderSyncable(item->folder)))
        {
            item->selected = selected;
            reviewOthers(item->folder, !selected);
            emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectedRole } );
        }
        item--;
    }

    updateSelectedAndTotalSize();
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

void BackupFolderModel::updateBackupFolder(QList<BackupFolder>::iterator item,
                                           bool selectable,
                                           const QString& message)
{
    bool toSelectable = !item->selectable && selectable;
    bool toUnselectable = item->selectable && !selectable;
    if(!toSelectable && !toUnselectable)
    {
        return;
    }

    if(toSelectable)
    {
        item->tooltip = item->folder;
    }
    else
    {
        item->tooltip = message;
    }
    item->selectable = selectable;

    const auto row = std::distance(mBackupFolderList.begin(), item);
    QModelIndex modelIndex(index(row, 0));
    emit dataChanged(modelIndex, modelIndex, { SelectableRole, Qt::ToolTipRole } );
}

void BackupFolderModel::reviewAllBackupFolders()
{
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    while (item != mBackupFolderList.end())
    {
        QString message;
        auto syncability = SyncController::isLocalFolderSyncable(item->folder, mega::MegaSync::TYPE_BACKUP, message);
        updateBackupFolder(item, syncability != SyncController::CANT_SYNC, message);
        item++;
    }
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

    reviewAllBackupFolders();
    checkSelectedAll();
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

