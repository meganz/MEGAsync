#include "BackupFolderModel.h"

#include "MegaApi.h"
#include "Utilities.h"
#include "syncs/control/SyncController.h"
#include "QMegaMessageBox.h"

BackupFolder::BackupFolder()
    : folder(QString::fromUtf8(""))
    , selected(false)
    , size(QString::fromUtf8(""))
    , folderSize(0)
    , selectable(true)
{
}

BackupFolder::BackupFolder(const QString& folder, bool selected)
    : folder(folder)
    , selected(selected)
    , folderSize(0)
    , selectable(true)
{
    Utilities::getFolderSize(folder, &folderSize);
    size = Utilities::getSizeString(folderSize);
}

BackupFolderModel::BackupFolderModel(QObject* parent)
    : QAbstractListModel(parent)
    , mSelectedRowsTotal(0)
    , mTotalSize(0)
{
    mRoleNames = {
        { FolderRole, "folder" },
        { SelectedRole, "selected" },
        { SizeRole, "size" },
        { FolderSizeRole, "folderSize" },
        { SelectableRole, "selectable" }
    };

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
            mBackupFolderList.append(BackupFolder(path, false));
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
            case FolderRole:
                item.folder = value.toString();
                break;
            case SelectedRole:
            {
                bool selected = value.toBool();
                bool isCkecked = (selected && isValidBackupFolder(item.folder, true, true));
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
            case SizeRole:
                item.size = value.toInt();
                break;
            case SelectableRole:
                item.selectable = value.toBool();
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
            case FolderRole:
                field = item.folder;
                break;
            case SelectedRole:
                field = item.selected;
                break;
            case SizeRole:
                field = item.size;
                break;
            case SelectableRole:
                field = item.selectable;
                break;
            default:
                break;
        }
    }

    return field;
}

bool BackupFolderModel::isValidBackupFolder(const QString& inputPath,
                                            bool displayWarning,
                                            bool fromCheckAction)
{
    QString message;
    auto syncability = SyncController::isLocalFolderSyncable(inputPath, mega::MegaSync::TYPE_BACKUP, message);
    if(syncability != SyncController::CANT_SYNC)
    {
        int row = 0;
        while (syncability != SyncController::CANT_SYNC && row < rowCount())
        {
            QString existingPath(mBackupFolderList[row].folder);
            if (!fromCheckAction && inputPath == existingPath)
            {
                if(mBackupFolderList[row].selected) {
                    message = tr("Folder is already selected. Select a different folder.");
                } else {
                    setData(index(row, 0), QVariant(true), SelectedRole);
                    displayWarning = false;
                }
                syncability = SyncController::CANT_SYNC;
            }
            else if(mBackupFolderList[row].selected)
            {
                if (inputPath.startsWith(existingPath)
                    && inputPath[existingPath.size()] == QDir::separator())
                {
                    message = SyncController::getErrStrCurrentBackupInsideExistingBackup();
                    syncability = SyncController::CANT_SYNC;
                }
                else if (existingPath.startsWith(inputPath)
                         && existingPath[inputPath.size()] == QDir::separator())
                {
                    message = SyncController::getErrStrCurrentBackupOverExistingBackup();
                    syncability = SyncController::CANT_SYNC;
                }
            }
            row++;
        }
    }

    if (displayWarning)
    {
        if (syncability == SyncController::CANT_SYNC)
        {
            QMegaMessageBox::warning(nullptr, QString(), message, QMessageBox::Ok);
        }
        else if (syncability == SyncController::WARN_SYNC
                 && fromCheckAction
                 && (QMegaMessageBox::warning(nullptr, QString(), message
                                              + QLatin1Char('/')
                                              + tr("Do you want to continue?"),
                                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                     == QMessageBox::Yes))
        {
            syncability = SyncController::CANT_SYNC;
        }
    }

    return (syncability != SyncController::CANT_SYNC);
}

void BackupFolderModel::changeOtherBackupFolders(const QString& folder,
                                                 bool enable)
{
    QVector<int> roles = { SelectableRole };
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
                    emit dataChanged(modelIndex, modelIndex, roles);
                }
            }
            else
            {
                mBackupFolderList[row].selectable = false;
                mBackupFolderList[row].selected = false;
                emit dataChanged(modelIndex, modelIndex, roles);
            }
        }
    }
}

bool BackupFolderModel::isBackupFolderValid(const QString& folder,
                                            const QString& existingPath) const
{
    return ((folder.startsWith(existingPath) && folder[existingPath.size()] == QDir::separator())
            || (existingPath.startsWith(folder) && existingPath[folder.size()] == QDir::separator()));
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
    if(!isValidBackupFolder(inputPath, true)) {
        return;
    }
    beginInsertRows(QModelIndex(), 0, 0);
    BackupFolder data(inputPath);
    mBackupFolderList.prepend(data);
    endInsertRows();
    changeOtherBackupFolders(inputPath, false);
    checkSelectedAll(data);
}

void BackupFolderModel::setAllSelected(bool selected)
{
    QModelIndex modelIndex;

    for (int row = 0; row < rowCount(); row++) {
        modelIndex = index(row, 0);
        if(selected)
        {
            if(mBackupFolderList[row].selectable && !mBackupFolderList[row].selected) {
                mBackupFolderList[row].selected = true;
                emit dataChanged(modelIndex, modelIndex, { SelectedRole } );
                changeOtherBackupFolders(mBackupFolderList[row].folder, false);
                mTotalSize += mBackupFolderList[row].folderSize;
            }
        }
        else
        {
            if(mBackupFolderList[row].selected) {
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

QString BackupFolderModel::getTooltipText(int index) const
{
    QString message (QString::fromUtf8(""));
    QString inputPath(mBackupFolderList[index].folder);
    auto syncability = SyncController::isLocalFolderSyncable(inputPath, mega::MegaSync::TYPE_BACKUP, message);
    if(syncability != SyncController::CANT_SYNC)
    {
        int row = 0;
        while (message.isEmpty() && row < rowCount())
        {
            QString existingPath(mBackupFolderList[row].folder);
            if(mBackupFolderList[row].selected)
            {
                if (inputPath.startsWith(existingPath)
                    && inputPath[existingPath.size()] == QDir::separator())
                {
                    message = SyncController::getErrStrCurrentBackupInsideExistingBackup();
                }
                else if (existingPath.startsWith(inputPath)
                         && existingPath[inputPath.size()] == QDir::separator())
                {
                    message = SyncController::getErrStrCurrentBackupOverExistingBackup();
                }
            }
            row++;
        }
    }
    return message;
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
    return index.data(BackupFolderModel::BackupFolderRoles::SelectedRole).toBool();
}

