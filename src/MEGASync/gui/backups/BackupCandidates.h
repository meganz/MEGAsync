#ifndef BACKUPCANDIDATES_H
#define BACKUPCANDIDATES_H

#include <QObject>

#include <memory>

class BackupCandidates: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString totalSize READ getTotalSize NOTIFY totalSizeChanged)
    Q_PROPERTY(bool totalSizeReady READ getIsTotalSizeReady NOTIFY totalSizeReadyChanged)
    Q_PROPERTY(Qt::CheckState checkAllState READ getCheckAllState NOTIFY checkAllStateChanged)
    Q_PROPERTY(QString conflictsNotificationText READ getConflictsNotificationText NOTIFY
                   globalErrorChanged)
    Q_PROPERTY(int globalError READ getGlobalError NOTIFY globalErrorChanged)

public:
    // In case you want to use this data in a model
    enum BackupFolderRoles
    {
        NAME_ROLE = Qt::UserRole + 1,
        FOLDER_ROLE,
        SIZE_ROLE,
        SIZE_READY_ROLE,
        SELECTED_ROLE,
        SELECTABLE_ROLE,
        DONE_ROLE,
        ERROR_ROLE,
    };

    enum BackupErrorCode
    {
        NONE = 0,
        DUPLICATED_NAME,
        EXISTS_REMOTE,
        SYNC_CONFLICT,
        PATH_RELATION,
        UNAVAILABLE_DIR,
        SDK_CREATION,
    };
    Q_ENUM(BackupErrorCode)

    class Data
    {
    public:
        Data() = default;
        Data(const Data& folder);
        Data(const QString& folder, const QString& displayName, bool selected = true);
        ~Data() = default;

        static QHash<int, QByteArray> roleNames();

    private:
        friend class BackupCandidates;
        friend class BackupCandidatesController;

        // Front (with role)
        QString mFolder;
        QString mName;
        bool mSelected;
        bool mDone;
        int mError;

        // Back (without role)
        long long mFolderSize;
        int mSdkError;
        int mSyncError;
    };

    BackupCandidates() = default;

    QString getTotalSize() const;
    bool getIsTotalSizeReady() const;
    Qt::CheckState getCheckAllState() const;
    int getGlobalError() const;
    int selectedRowsTotal() const;
    long long getBackupsTotalSize() const;
    const QString& getConflictsNotificationText() const;
    int getSDKConflictCount() const;
    int getRemoteConflictCount() const;

    std::shared_ptr<BackupCandidates::Data> getBackupCandidate(int index);
    std::shared_ptr<BackupCandidates::Data> getBackupCandidateByFolder(const QString& folder);
    QList<std::shared_ptr<BackupCandidates::Data>> getBackupCandidates();
    int getSize() const;
    int getRow(std::shared_ptr<BackupCandidates::Data> candidate) const;
    int getRow(const QString& folder) const;

signals:
    void totalSizeChanged();
    void checkAllStateChanged();
    void totalSizeReadyChanged();
    void globalErrorChanged();
    void noneSelected();

private:
    friend class BackupCandidatesController;

    void setIsTotalSizeReady(bool totalSizeReady);
    void setCheckAllState(Qt::CheckState state);
    void setGlobalError(BackupCandidates::BackupErrorCode error);
    void addBackupCandidate(std::shared_ptr<BackupCandidates::Data> backupCandidate);
    bool removeBackupCandidate(int index);
    bool removeBackupCandidate(const QString& folder);
    void setSDKConflictCount(int newSdkConflictCount);
    void setRemoteConflictCount(int newRemoteConflictCount);
    void setSelectedRowsTotal(int newSelectedRowsTotal);
    void setBackupsTotalSize(long long newBackupsTotalSize);
    void setConflictsNotificationText(const QString& text);

    QList<std::shared_ptr<Data>> mBackupCandidatesList;
    int mSelectedRowsTotal;
    // In bytes
    long long mBackupsTotalSize;
    bool mTotalSizeReady;
    Qt::CheckState mCheckAllState;
    int mGlobalError;
    int mConflictsSize;
    int mSDKConflictCount;
    int mRemoteConflictCount;
    QString mConflictsNotificationText;
};

#endif // BACKUPCANDIDATES_H
