#ifndef SYNCS_H
#define SYNCS_H

#include "SyncController.h"

#include <QObject>

#include <memory>
#include <optional>

namespace mega
{
class MegaApi;
}

class Syncs: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString defaultMegaFolder READ getDefaultMegaFolder CONSTANT FINAL)
    Q_PROPERTY(QString defaultMegaPath READ getDefaultMegaPath CONSTANT FINAL)
    Q_PROPERTY(SyncStatusCode syncStatus READ getSyncStatus WRITE setSyncStatus NOTIFY syncStatusChanged)
    Q_PROPERTY(QString localError READ getLocalError NOTIFY localErrorChanged)
    Q_PROPERTY(QString remoteError READ getRemoteError NOTIFY remoteErrorChanged)

public:
    enum SyncStatusCode
    {
        NONE = 0,
        FULL,
        SELECTIVE
    };
    Q_ENUM(SyncStatusCode)

    Syncs(QObject* parent = nullptr);
    virtual ~Syncs() = default;

    Q_INVOKABLE void addSync(SyncInfo::SyncOrigin origin,
                             const QString& local,
                             const QString& remote = QLatin1String("/"));
    Q_INVOKABLE bool checkLocalSync(const QString& path);
    Q_INVOKABLE bool checkRemoteSync(const QString& path);
    Q_INVOKABLE void clearRemoteError();
    Q_INVOKABLE void clearLocalError();

    static QString getDefaultMegaFolder();
    static QString getDefaultMegaPath();

    SyncStatusCode getSyncStatus() const;
    void setSyncStatus(SyncStatusCode status);

    QString getLocalError() const;
    QString getRemoteError() const;

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

    mega::MegaApi* mMegaApi = nullptr;
    std::unique_ptr<SyncController> mSyncController;

    bool mCreatingFolder = false;
    SyncStatusCode mSyncStatus = SyncStatusCode::NONE;
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
