#ifndef BACKUPCANDIDATES_H
#define BACKUPCANDIDATES_H

#include "FileFolderAttributes.h"

#include <QObject>

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
        ERROR_ROLE
    };

    enum BackupErrorCode
    {
        NONE = 0,
        DUPLICATED_NAME = 1,
        EXISTS_REMOTE = 2,
        SYNC_CONFLICT = 3,
        PATH_RELATION = 4,
        UNAVAILABLE_DIR = 5,
        SDK_CREATION = 6
    };
    Q_ENUM(BackupErrorCode)

    class Data
    {
    public:
        static const char* SIZE_READY;

        // Front (with role)
        QString mFolder;
        QString mName;
        QString mSize;
        bool mSelected;
        bool mDone;
        bool mFolderSizeReady;
        int mError;

        // Back (without role)
        long long mFolderSize;
        int mSdkError;
        int mSyncError;

        Data() = default;
        Data(const Data& folder);
        Data(const QString& folder, const QString& displayName, bool selected = true);
        ~Data();

        QObject* mFolderAttrContext = nullptr;
        LocalFileFolderAttributes* mFolderAttr;
        void setSize(long long size);
        void setFolder(const QString& folder);
        void setError(int error);

        QString getFolder() const
        {
            return mFolder;
        }

        void calculateFolderSize();

        static QHash<int, QByteArray> roleNames();

    private:
        bool createFileFolderAttributes();
    };

    BackupCandidates();

    QString getTotalSize() const;
    bool getIsTotalSizeReady() const;
    void setIsTotalSizeReady(bool totalSizeReady);

    Qt::CheckState getCheckAllState() const;
    void setCheckAllState(Qt::CheckState state);

    bool getExistConflicts() const;

    void setGlobalError(BackupCandidates::BackupErrorCode error);
    int getGlobalError() const;

    std::shared_ptr<BackupCandidates::Data> getBackupCandidate(int index) const;
    std::shared_ptr<BackupCandidates::Data> getBackupCandidateByFolder(const QString& folder) const;
    QList<std::shared_ptr<BackupCandidates::Data>> getBackupCandidates() const;
    int size() const;
    int getRow(std::shared_ptr<BackupCandidates::Data> candidate) const;
    int getRow(const QString& folder) const;

    void addBackupCandidate(std::shared_ptr<BackupCandidates::Data> backupCandidate);
    void removeBackupCandidate(int index);
    bool removeBackupCandidate(const QString& folder);

    int conflictsSize() const;
    void setConflictsSize(int newConflictsSize);

    int SDKConflictCount() const;
    void setSDKConflictCount(int newSdkConflictCount);

    int remoteConflictCount() const;
    void setRemoteConflictCount(int newRemoteConflictCount);

    int selectedRowsTotal() const;
    void setSelectedRowsTotal(int newSelectedRowsTotal);

    long long backupsTotalSize() const;
    void setBackupsTotalSize(long long newBackupsTotalSize);

    QString getConflictsNotificationText() const;
    QString getSdkErrorString() const;
    QString getSyncErrorString() const;

signals:
    void totalSizeChanged();
    void checkAllStateChanged();
    void totalSizeReadyChanged();
    void globalErrorChanged();
    void noneSelected();

private:
    QList<std::shared_ptr<Data>> mBackupCandidatesList;

    int mSelectedRowsTotal;
    long long mBackupsTotalSize;
    bool mTotalSizeReady;
    Qt::CheckState mCheckAllState;
    int mGlobalError;
    int mConflictsSize;
    int mSDKConflictCount;
    int mRemoteConflictCount;
};

#endif // BACKUPCANDIDATES_H
