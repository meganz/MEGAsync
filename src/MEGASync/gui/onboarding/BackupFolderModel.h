#ifndef BACKUPFOLDERMODEL_H
#define BACKUPFOLDERMODEL_H

#include "syncs/control/SyncController.h"

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

struct BackupFolder
{
    // Front (with role)
    QString display;
    QString tooltip;
    QString folder;
    QString size;
    bool selected;
    bool selectable;
    bool confirmed;
    bool done;
    int error;

    // Back (without role)
    long long folderSize;

    BackupFolder();

    BackupFolder(const QString& folder,
                 const QString& displayName,
                 bool selected = true);
};

class BackupFolderModel : public QAbstractListModel
{
    Q_OBJECT

public:

    enum BackupFolderRoles {
        FolderRole = Qt::UserRole + 1,
        SizeRole,
        SelectedRole,
        SelectableRole,
        ConfirmedRole,
        DoneRole,
        ErrorRole
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

    void updateConfirmed();

    QStringList getConfirmedDirs() const;

    void clean();

    void update(const QString& path, int errorCode);

signals:

    void rowSelectedChanged(bool selectedRow, bool selectedAll);

    void disableRow(int index);

private:

    QList<BackupFolder> mBackupFolderList;
    QHash<int, QByteArray> mRoleNames;
    int mSelectedRowsTotal;
    long long mTotalSize;
    SyncController mSyncController;

    void populateDefaultDirectoryList();

    void checkSelectedAll(const BackupFolder& item);

    bool isValidBackupFolder(const QString& inputPath);

    bool isValidInsertion(const QString& inputPath);

    bool folderContainsOther(const QString& folder,
                             const QString& other) const;

    int calculateNumSelectedRows() const;

    void changeOtherBackupFolders(const QString& folder, bool enable);

    bool canBackupFolderEnabled(const QString& folder,
                                const QString& selectedFolder) const;

    bool isBackupFolderValid(const QString& folder,
                             const QString& existingPath) const;

    QString getToolTipErrorText(const QString& folder,
                                const QString& existingPath) const;

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
