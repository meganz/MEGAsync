#ifndef BACKUPFOLDERMODEL_H
#define BACKUPFOLDERMODEL_H

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

struct BackupFolder
{
    QString folder;
    bool selected;
    QString size;
    long long folderSize;

    BackupFolder();

    BackupFolder(const QString& folder,
                 bool selected = true);
};

class BackupFolderModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum BackupFolderRoles {
        FolderRole = Qt::UserRole + 1,
        SelectedRole,
        SizeRole,
        FolderSizeRole
    };

    explicit BackupFolderModel(QObject* parent = nullptr);

    QHash<int,QByteArray> roleNames() const override;

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

public slots:

    void insertFolder(const QString& folder);

    void setAllSelected(bool selected);

    // TODO: Change by property??
    int getNumSelectedRows() const;

    QString getTotalSize() const;

signals:

    void rowSelectedChanged(bool selectedRow, bool selectedAll);

private:

    QList<BackupFolder> mBackupFolderList;
    QHash<int, QByteArray> mRoleNames;
    int mSelectedRowsTotal;
    long long mTotalSize;

    void populateDefaultDirectoryList();

    void checkSelectedAll(const BackupFolder& item);

};

class BackupFolderFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool selectedFilterEnabled READ selectedFilterEnabled
               WRITE setSelectedFilterEnabled NOTIFY selectedFilterEnabledChanged)

public:
    explicit BackupFolderFilterProxyModel(QObject* parent = nullptr);

    bool selectedFilterEnabled() const;

public slots:
    void setSelectedFilterEnabled(bool enabled);

signals:
    void selectedFilterEnabledChanged();

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    bool mSelectedFilterEnabled;

};

#endif // BACKUPFOLDERMODEL_H
