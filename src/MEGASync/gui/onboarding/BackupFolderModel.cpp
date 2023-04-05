#include "BackupFolderModel.h"

#include "MegaApi.h"
#include "Utilities.h"

BackupFolder::BackupFolder()
    : folder(QString::fromUtf8(""))
    , selected(false)
    , size(QString::fromUtf8(""))
    , folderSize(0)
{
}

BackupFolder::BackupFolder(const QString& folder, bool selected)
    : folder(folder)
    , selected(selected)
    , folderSize(0)
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
        { FolderSizeRole, "folderSize" }
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
    for (QStandardPaths::StandardLocation path : defaultPaths)
    {
        QString fullPath(QStandardPaths::writableLocation(path));
        if(!fullPath.isEmpty())
        {
            mBackupFolderList.append(BackupFolder(fullPath, false));
        }
        else
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                               QString::fromUtf8("Default path with enum %1 is empty.").arg(path).toUtf8().constData());
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
                item.selected = value.toBool();
                checkSelectedAll(item);
                break;
            case SizeRole:
                item.size = value.toInt();
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
            default:
                break;
        }
    }

    return field;
}

void BackupFolderModel::insertFolder(const QString &folder)
{
    beginInsertRows(QModelIndex(), 0, 0);
    BackupFolder data(folder);
    mBackupFolderList.prepend(data);
    checkSelectedAll(data);
    endInsertRows();
}

void BackupFolderModel::setAllSelected(bool selected)
{
    QModelIndex modelIndex;
    for (int row = 0; row < rowCount(); row++) {
        modelIndex = index(row, 0);
        mBackupFolderList[row].selected = selected;
        emit dataChanged(modelIndex, modelIndex, { SelectedRole } );
    }

    mSelectedRowsTotal = (selected ? mBackupFolderList.size() : 0);
}

int BackupFolderModel::getNumSelectedRows() const
{
    return mSelectedRowsTotal;
}

QString BackupFolderModel::getTotalSize() const
{
    return Utilities::getSizeString(mTotalSize);
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

