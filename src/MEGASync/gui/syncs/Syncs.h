#ifndef SYNCS_H
#define SYNCS_H

#include "SyncController.h"
#include "SyncsUtils.h"

#include <QObject>

#include <memory>
#include <optional>

namespace mega
{
class MegaApi;
}

class SyncsData;
class Syncs: public QObject
{
    Q_OBJECT

    Q_PROPERTY(SyncsUtils::SyncStatusCode syncStatus READ getSyncStatus WRITE setSyncStatus NOTIFY
                   syncStatusChanged)

public:
    Syncs(QObject* parent = nullptr);
    virtual ~Syncs() = default;

    Q_INVOKABLE void addSync(SyncInfo::SyncOrigin origin,
                             const QString& local,
                             const QString& remote = QLatin1String("/"));
    Q_INVOKABLE bool checkLocalSync(const QString& path);
    Q_INVOKABLE bool checkRemoteSync(const QString& path);
    Q_INVOKABLE void clearRemoteError();
    Q_INVOKABLE void clearLocalError();

    SyncsUtils::SyncStatusCode getSyncStatus() const;
    void setSyncStatus(SyncsUtils::SyncStatusCode status);

    QString getLocalError() const;
    QString getRemoteError() const;

    /* dev only, remove it should be the contructor */
    void setSyncsData(SyncsData* syncsData)
    {
        mSyncsData = syncsData;
    }

signals:
    void syncSetupSuccess();
    void syncStatusChanged();
    void syncRemoved();
    void localErrorChanged();
    void remoteErrorChanged();

public slots:
    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* error);

private slots:
    void onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString name);
    void onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings);

private:
    enum class LocalErrors
    {
        EmptyPath,
        NoAccessPermissionsNoExist,
        NoAccessPermissionsCantCreate,
        CantSync
    };

    enum class RemoteErrors
    {
        EmptyPath,
        CantSync,
        CantCreateRemoteFolder,
        CantCreateRemoteFolderMsg,
        CantAddSync
    };

    struct MegaRemoteCodeError{
        int error;
        int syncError;
    };

    SyncsData* mSyncsData = nullptr;
    mega::MegaApi* mMegaApi = nullptr;
    std::unique_ptr<SyncController> mSyncController;

    bool mCreatingFolder = false;
    SyncsUtils::SyncStatusCode mSyncStatus = SyncsUtils::SyncStatusCode::NONE;
    SyncController::SyncConfig mSyncConfig;

    // vars with de command error data, used to generate error messages.
    MegaRemoteCodeError mRemoteMegaError;
    std::optional<LocalErrors> mLocalError;
    std::optional<RemoteErrors> mRemoteError;
    QString mRemoteStringMessage;

    bool checkErrorsOnSyncPaths(const QString& localPath, const QString& remotePath);
    void helperCheckLocalSync(const QString& path);
    void helperCheckRemoteSync(const QString& path);
    void cleanErrors();
};

#endif // SYNCS_H
