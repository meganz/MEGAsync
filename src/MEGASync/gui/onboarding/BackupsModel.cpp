
#include "BackupsModel.h"

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

BackupsModel::BackupsModel(QObject* parent)
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

    connect(SyncInfo::instance(), &SyncInfo::syncRemoved, this, &BackupsModel::onSyncRemoved);
    connect(SyncInfo::instance(), &SyncInfo::syncStateChanged, this, &BackupsModel::onSyncChanged);
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
    auto lastTotalSize = mTotalSize;
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

    if(mTotalSize != lastTotalSize)
    {
        emit totalSizeChanged();
    }
}

void BackupsModel::checkSelectedAll()
{
    updateSelectedAndTotalSize();

    emit rowSelectedChanged(mSelectedRowsTotal > 0 && mSelectedRowsTotal < mBackupFolderList.size(),
                            mSelectedRowsTotal == mBackupFolderList.size());
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

QVariant BackupsModel::data(const QModelIndex &index, int role) const
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

void BackupsModel::onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings)
{
    QString localFolder (syncSettings->getLocalFolder());
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    bool found = false;
    while (!found && item != mBackupFolderList.end())
    {
        if((found = (item->folder == localFolder)))
        {
            item->selectable = true;
            item->tooltip = item->folder;
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
        if((found = (item->folder == localFolder)))
        {
            QString message;
            auto syncability = SyncController::isLocalFolderSyncable(localFolder, mega::MegaSync::TYPE_BACKUP, message);
            if(syncability == SyncController::CANT_SYNC && item->selectable)
            {
                // This element can not be selected
                item->selected = false;
                item->selectable = false;
                item->tooltip = message;
                emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectedRole, SelectableRole, Qt::ToolTipRole });
                reviewOthers(localFolder, false);
                checkSelectedAll();
            }
        }
        item++;
    }
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
        QString existingPath(item->folder);
        if ((folder != existingPath) && isRelatedFolder(folder, existingPath))
        {
            QString message;
            auto syncability = SyncController::isLocalFolderSyncable(existingPath, mega::MegaSync::TYPE_BACKUP, message);
            if(!item->selectable && selectable && (syncability != SyncController::CANT_SYNC)
                && !existAnotherBackupFolderRelated(existingPath, folder))
            {
                item->selectable = true;
                item->tooltip = item->folder;
                emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectableRole, Qt::ToolTipRole });
            }
            else if(item->selectable && !selectable)
            {
                item->selectable = false;
                item->selected = false;
                item->tooltip = message.isEmpty() ? getToolTipErrorText(existingPath, folder) : message;
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
        QString existingPath(item->folder);
        if (!item->selectable
                && isRelatedFolder(folder, existingPath)
                && !existAnotherBackupFolderRelated(existingPath, folder)
                && isLocalFolderSyncable(existingPath))
        {
            item->selectable = true;
            item->tooltip = item->folder;
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
        QString existingPath(mBackupFolderList[row].folder);
        if (folder != existingPath && selectedFolder != existingPath)
        {
            exists = isRelatedFolder(folder, existingPath) && mBackupFolderList[row].selected;
        }
        row++;
    }
    return exists;
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

void BackupsModel::setAllSelected(bool selected)
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

int BackupsModel::getNumSelectedRows() const
{
    return mSelectedRowsTotal;
}

QString BackupsModel::getTotalSize() const
{
    return Utilities::getSizeString(mTotalSize);
}

void BackupsModel::updateConfirmed()
{
    for (int row = 0; row < rowCount(); row++)
    {
        mBackupFolderList[row].confirmed = mBackupFolderList[row].selected;
    }
}

QStringList BackupsModel::getConfirmedDirs() const
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

void BackupsModel::updateBackupFolder(QList<BackupFolder>::iterator item,
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

    emit dataChanged(getModelIndex(item), getModelIndex(item), { SelectableRole, Qt::ToolTipRole } );
}

void BackupsModel::reviewAllBackupFolders()
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

void BackupsModel::clean()
{
    QList<BackupFolder>::iterator item = mBackupFolderList.begin();
    while (item != mBackupFolderList.end())
    {
        if(item->done)
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
        if((found = (mBackupFolderList[row].folder == path)))
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
        row++;
    }
}

BackupsProxyModel::BackupsProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
    , mSelectedFilterEnabled(false)
{
    setSourceModel(new BackupsModel(this));
    setDynamicSortFilter(true);

    connect(backupsModel(), &BackupsModel::rowSelectedChanged, this, &BackupsProxyModel::onRowSelectedChanged);
    connect(backupsModel(), &BackupsModel::disableRow, this, &BackupsProxyModel::disableRow);
    connect(backupsModel(), &BackupsModel::totalSizeChanged, this, &BackupsProxyModel::totalSizeChanged);
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
    return index.data(BackupsModel::BackupFolderRoles::ConfirmedRole).toBool();
}

QString BackupsProxyModel::getTotalSize()
{
    return backupsModel()->getTotalSize();
}

void BackupsProxyModel::setAllSelected(bool selected)
{
    backupsModel()->setAllSelected(selected);
}

int BackupsProxyModel::getNumSelectedRows()
{
    return backupsModel()->getNumSelectedRows();
}

void BackupsProxyModel::insertFolder(const QString &folder)
{
    backupsModel()->insertFolder(folder);
}

void BackupsProxyModel::updateConfirmed()
{
    backupsModel()->updateConfirmed();
}

QStringList BackupsProxyModel::getConfirmedDirs()
{
    return backupsModel()->getConfirmedDirs();
}

void BackupsProxyModel::clean()
{
    backupsModel()->clean();
}

void BackupsProxyModel::update(const QString& path, int errorCode)
{
    backupsModel()->update(path, errorCode);
}

BackupsModel* BackupsProxyModel::backupsModel()
{
    return dynamic_cast<BackupsModel*>(sourceModel());
}

void BackupsProxyModel::onRowSelectedChanged(bool selectedRow, bool selectedAll)
{
    emit rowSelectedChanged(selectedRow, selectedAll);
}

void BackupsProxyModel::onDisableRowChanged(int index)
{
    emit disableRow(index);
}

void BackupsProxyModel::onTotalSizeChanged()
{
    emit totalSizeChanged();
}
