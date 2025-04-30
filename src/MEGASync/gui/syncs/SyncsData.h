#ifndef SYNCS_DATA_H
#define SYNCS_DATA_H

#include "SyncInfo.h"

#include <QObject>

class Syncs;
class SyncsData: public QObject
{
    Q_OBJECT

    Q_PROPERTY(
        QString defaultLocalFolder READ getDefaultLocalFolder NOTIFY defaultLocalFolderChanged)
    Q_PROPERTY(
        QString defaultRemoteFolder READ getDefaultRemoteFolder NOTIFY defaultRemoteFolderChanged)
    Q_PROPERTY(SyncInfo::SyncOrigin syncOrigin READ getSyncOrigin NOTIFY syncOriginChanged)
    Q_PROPERTY(QString localError READ getLocalError NOTIFY localErrorChanged)
    Q_PROPERTY(QString remoteError READ getRemoteError NOTIFY remoteErrorChanged)
    Q_PROPERTY(QString remoteFolderCandidate READ getRemoteFolderCandidate NOTIFY
                   remoteFolderCandidateChanged)
    Q_PROPERTY(QString localFolderCandidate READ getLocalFolderCandidate NOTIFY
                   localFolderCandidateChanged)

    friend class Syncs;

public:
    explicit SyncsData(QObject* parent = nullptr);
    virtual ~SyncsData() = default;

    QString getRemoteFolderCandidate() const;
    QString getLocalFolderCandidate() const;

signals:
    void localErrorChanged();
    void remoteErrorChanged();
    void syncSetupSuccess(bool isFullSync);
    void syncRemoved();
    void syncOriginChanged();
    void defaultLocalFolderChanged(QString localPath);
    void defaultRemoteFolderChanged(QString remotePath);
    void remoteFolderCandidateChanged();
    void localFolderCandidateChanged();

private:
    QString getLocalError() const;
    QString getRemoteError() const;
    SyncInfo::SyncOrigin getSyncOrigin() const;
    QString getDefaultLocalFolder() const;
    QString getDefaultRemoteFolder() const;
    void setLocalError(const QString& error);
    void setRemoteError(const QString& error);

    void setRemoteFolderCandidate(const QString& remoteFolderCandidate);
    void setLocalFolderCandidate(const QString& localFolderCandidate);

    QString mLocalError;
    QString mRemoteError;
    SyncInfo::SyncOrigin mSyncOrigin = SyncInfo::SyncOrigin::MAIN_APP_ORIGIN;
    QString mDefaultLocalFolder;
    QString mDefaultRemoteFolder;
    QString mRemoteFolderCandidate;
    QString mLocalFolderCandidate;
};

#endif
