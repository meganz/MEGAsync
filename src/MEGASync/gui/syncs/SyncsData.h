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

    friend class Syncs;

public:
    explicit SyncsData(QObject* parent = nullptr);
    virtual ~SyncsData() = default;

signals:
    void localErrorChanged();
    void remoteErrorChanged();
    void syncSetupSuccess(bool isFullSync);
    void syncRemoved();
    void syncOriginChanged();
    void defaultLocalFolderChanged(QString localPath);
    void defaultRemoteFolderChanged(QString remotePath);

private:
    QString getLocalError() const;
    QString getRemoteError() const;
    SyncInfo::SyncOrigin getSyncOrigin() const;
    QString getDefaultLocalFolder() const;
    QString getDefaultRemoteFolder() const;

    void setLocalError(const QString& error);
    void setRemoteError(const QString& error);

    QString mLocalError;
    QString mRemoteError;
    SyncInfo::SyncOrigin mSyncOrigin = SyncInfo::SyncOrigin::MAIN_APP_ORIGIN;
    QString mDefaultLocalFolder;
    QString mDefaultRemoteFolder;
};

#endif
