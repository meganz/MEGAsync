#ifndef BACKUPFOLDERMODEL_H
#define BACKUPFOLDERMODEL_H

#include "syncs/control/SyncController.h"
#include "BackupsController.h"

#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QTimer>
#include "control/FileFolderAttributes.h"

class BackupFolder : public QObject
{
    Q_OBJECT

public:
    // Front (with role)
    QString mName;
    QString mSize;
    bool mSelected;
    bool mDone;
    bool mFolderSizeReady;
    int mError;

    // Back (without role)
    quint64 folderSize;
    QString sdkError;

    BackupFolder();

    BackupFolder(const BackupFolder& folder);

    BackupFolder(const QString& folder,
                 const QString& displayName,
                 bool selected = true, QObject* parent = nullptr);

    LocalFileFolderAttributes* mFolderAttr;
    void setSize(qint64 size);
    void setFolder(const QString& folder);
    QString getFolder() const {return mFolder;}
    void calculateFolderSize();

private:
    bool createFileFolderAttributes();
    QString mFolder;
};

class BackupsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString totalSize
               READ getTotalSize
               NOTIFY totalSizeChanged)
    Q_PROPERTY(bool totalSizeReady
               READ getIsTotalSizeReady
               NOTIFY totalSizeReadyChanged)
    Q_PROPERTY(Qt::CheckState checkAllState
               READ getCheckAllState
               WRITE setCheckAllState
               NOTIFY checkAllStateChanged)
    Q_PROPERTY(QString conflictsNotificationText
               READ getConflictsNotificationText
               NOTIFY existConflictsChanged)
    Q_PROPERTY(int globalError
               READ getGlobalError
               NOTIFY globalErrorChanged)
    Q_PROPERTY(bool existsOnlyGlobalError
               READ existsOnlyGlobalError
               NOTIFY existsOnlyGlobalErrorChanged)

public:

    enum BackupFolderRoles
    {
        NameRole = Qt::UserRole + 1,
        FolderRole,
        SizeRole,
        SizeReadyRole,
        SelectedRole,
        SelectableRole,
        DoneRole,
        ErrorRole
    };

    enum BackupErrorCode
    {
        None = 0,
        DuplicatedName = 1,
        ExistsRemote = 2,
        SyncConflict = 3,
        PathRelation = 4,
        UnavailableDir = 5,
        SDKCreation = 6
    };
    Q_ENUM(BackupErrorCode)

    explicit BackupsModel(QObject* parent = nullptr);
    ~BackupsModel();
    QHash<int,QByteArray> roleNames() const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant data(const QModelIndex & index, int role = NameRole) const override;
    QString getTotalSize() const;
    bool getIsTotalSizeReady() const;
    Qt::CheckState getCheckAllState() const;
    void setCheckAllState(Qt::CheckState state, bool fromModel = false);
    BackupsController* backupsController() const;
    bool getExistConflicts() const;
    QString getConflictsNotificationText() const;
    int getGlobalError() const;
    bool existsOnlyGlobalError() const;
    int getRow(const QString& folder);
    void calculateFolderSizes();
    void updateSelectedAndTotalSize();

public slots:
    void insert(const QString& folder);
    void check();
    int rename(const QString& folder, const QString& name);
    void remove(const QString& folder);
    void change(const QString& oldFolder, const QString& newFolder);
    bool checkDirectories();
    void clean(bool resetErrors = false);

signals:
    void totalSizeChanged();
    void checkAllStateChanged();
    void existConflictsChanged();
    void noneSelected();
    void globalErrorChanged();
    void existsOnlyGlobalErrorChanged();
    void totalSizeReadyChanged();
    void backupsCreationFinished(bool succes);

private:
    const QString getFolderUnavailableErrorMsg();
    static int CHECK_DIRS_TIME;

    QList<BackupFolder*> mBackupFolderList;
    int mSelectedRowsTotal;
    unsigned long long mBackupsTotalSize;
    bool mTotalSizeReady;
    SyncController mSyncController;
    std::unique_ptr<BackupsController> mBackupsController;
    int mConflictsSize;
    QString mConflictsNotificationText;
    Qt::CheckState mCheckAllState;
    int mGlobalError;
    QTimer mCheckDirsTimer;
    bool mExistsOnlyGlobalError;
    void populateDefaultDirectoryList();
    void checkSelectedAll();
    bool isLocalFolderSyncable(const QString& inputPath);
    bool selectIfExistsInsertion(const QString& inputPath);
    bool folderContainsOther(const QString& folder,
                             const QString& other) const;
    bool isRelatedFolder(const QString& folder,
                         const QString& existingPath) const;
    QModelIndex getModelIndex(QList<BackupFolder*>::iterator item);
    void setAllSelected(bool selected);
    bool checkPermissions(const QString& inputPath);
    void checkRemoteDuplicatedBackups(const QSet<QString>& candidateSet);
    void checkDuplicatedBackupNames(const QSet<QString>& candidateSet,
                                    const QStringList& candidateList);
    void reviewConflicts();
    void changeConflictsNotificationText(const QString& text);
    bool existOtherRelatedFolder(const int currentRow);
    bool existsFolder(const QString& inputPath);
    void setGlobalError(BackupErrorCode error);
    void setTotalSizeReady(bool ready);

private slots:
    void onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings);
    void onBackupsCreationFinished(bool success);
    void onBackupFinished(const QString& folder,
                          bool done,
                          const QString& sdkError = QString());

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
    void backupsCreationFinished(bool success);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    bool mSelectedFilterEnabled;
    BackupsModel* backupsModel();
};

#endif // BACKUPFOLDERMODEL_H
