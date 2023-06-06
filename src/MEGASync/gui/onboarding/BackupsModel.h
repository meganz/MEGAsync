#ifndef BACKUPFOLDERMODEL_H
#define BACKUPFOLDERMODEL_H

#include "syncs/control/SyncController.h"
#include "BackupsController.h"

#include <QAbstractListModel>
#include <QSortFilterProxyModel>

struct BackupFolder
{
    // Front (with role)
    QString mName;
    QString mTooltip;
    QString mFolder;
    QString mSize;
    bool mSelected;
    bool mSelectable;
    bool mDone;
    int mError;
    bool mErrorVisible;

    // Back (without role)
    long long folderSize;

    BackupFolder();

    BackupFolder(const QString& folder,
                 const QString& displayName,
                 bool selected = true);
};

class BackupsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString mTotalSize READ getTotalSize NOTIFY totalSizeChanged)
    Q_PROPERTY(Qt::CheckState mCheckAllState READ getCheckAllState WRITE setCheckAllState NOTIFY checkAllStateChanged)
    Q_PROPERTY(bool mExistConflicts READ getExistConflicts NOTIFY existConfilctsChanged)

public:

    enum BackupFolderRoles
    {
        NameRole = Qt::UserRole + 1,
        FolderRole,
        SizeRole,
        SelectedRole,
        SelectableRole,
        DoneRole,
        ErrorRole,
        ErrorVisibleRole
    };

    enum BackupErrorCode
    {
        None = 0,
        DuplicatedName = 1,
        ExistsRemote = 2
    };
    Q_ENUM(BackupErrorCode)

    explicit BackupsModel(QObject* parent = nullptr);

    QHash<int,QByteArray> roleNames() const override;

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QVariant data(const QModelIndex & index, int role = NameRole) const override;

    QString getTotalSize() const;

    Qt::CheckState getCheckAllState() const;

    void setCheckAllState(Qt::CheckState state, bool fromModel = false);

    BackupsController* backupsController() const;

    bool getExistConflicts() const;

public slots:

    void insertFolder(const QString& folder);

    void checkBackups();

    bool renameBackup(const QString& folder, const QString& name);

    void remove(const QString& folder);

signals:

    void totalSizeChanged();

    void checkAllStateChanged();

    void existConfilctsChanged();

private:

    QList<BackupFolder> mBackupFolderList;
    QHash<int, QByteArray> mRoleNames;
    int mSelectedRowsTotal;
    long long mBackupsTotalSize;
    SyncController mSyncController;
    BackupsController* mBackupsController;
    bool mExistConflicts;
    Qt::CheckState mCheckAllState;

    void populateDefaultDirectoryList();

    void updateSelectedAndTotalSize();

    void checkSelectedAll();

    bool isLocalFolderSyncable(const QString& inputPath);

    bool selectIfExistsInsertion(const QString& inputPath);

    QString getToolTipErrorText(const QString& folder,
                                const QString& existingPath) const;

    bool folderContainsOther(const QString& folder,
                             const QString& other) const;

    bool isRelatedFolder(const QString& folder,
                         const QString& existingPath) const;

    QModelIndex getModelIndex(QList<BackupFolder>::iterator item);

    void reviewOthers(const QString& folder,
                      bool enable);

    void reviewOthersWhenRemoved(const QString& folder);

    bool existAnotherBackupFolderRelated(const QString& folder,
                                         const QString& selectedFolder) const;

    void updateBackupFolder(QList<BackupFolder>::iterator item,
                            bool selectable,
                            const QString& message);

    void reviewAllBackupFolders();

    int getRow(const QString& folder);

    void setAllSelected(bool selected);

    void reviewOtherBackupErrors(const QString& name);

    void checkRemoteDuplicatedBackups(const QSet<QString>& candidateSet);

    void checkDuplicatedBackupNames(const QSet<QString>& candidateSet,
                                    const QStringList& candidateList);

private slots:

    void onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings);

    void onSyncChanged(std::shared_ptr<SyncSettings> syncSettings);

    void clean();

    void update(const QString& path, int errorCode);

};

class BackupsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

    Q_PROPERTY(bool selectedFilterEnabled READ selectedFilterEnabled
               WRITE setSelectedFilterEnabled NOTIFY selectedFilterEnabledChanged)

public:

    explicit BackupsProxyModel(QObject* parent = nullptr);

    bool selectedFilterEnabled() const;

    void setSelectedFilterEnabled(bool enabled);

public slots:

    void createBackups();

signals:

    void selectedFilterEnabledChanged();

protected:

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:

    bool mSelectedFilterEnabled;

    BackupsModel* backupsModel();

};

#endif // BACKUPFOLDERMODEL_H
