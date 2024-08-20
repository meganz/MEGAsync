#ifndef SYNCS_H
#define SYNCS_H

#include "SyncController.h"

#include "megaapi.h"
#include "mega/bindings/qt/QTMegaRequestListener.h"

#include <QObject>

#include <memory>
#include <optional>

class SyncController;
class Syncs : public QObject, public mega::MegaRequestListener
{
    Q_OBJECT

    Q_PROPERTY(QString defaultMegaFolder READ getDefaultMegaFolder CONSTANT FINAL)
    Q_PROPERTY(QString defaultMegaPath READ getDefaultMegaPath CONSTANT FINAL)
    Q_PROPERTY(SyncStatusCode syncStatus READ getSyncStatus WRITE setSyncStatus NOTIFY syncStatusChanged)
    Q_PROPERTY(QString localError READ getLocalError WRITE setLocalError NOTIFY localErrorChanged)
    Q_PROPERTY(QString remoteError READ getRemoteError WRITE setRemoteError NOTIFY remoteErrorChanged)

public:
    enum SyncStatusCode
    {
        NONE = 0,
        FULL,
        SELECTIVE
    };
    Q_ENUM(SyncStatusCode)

    static const QString DEFAULT_MEGA_FOLDER;
    static const QString DEFAULT_MEGA_PATH;

    Syncs(QObject* parent = nullptr);
    virtual ~Syncs() = default;

    Q_INVOKABLE void addSync(SyncInfo::SyncOrigin origin,
                             const QString& local,
                             const QString& remote = QLatin1String("/"));
    Q_INVOKABLE bool checkLocalSync(const QString& path);
    Q_INVOKABLE bool checkRemoteSync(const QString& path);
    Q_INVOKABLE void clearRemoteError();
    Q_INVOKABLE void clearLocalError();

    QString getDefaultMegaFolder() const;
    QString getDefaultMegaPath() const;

    SyncStatusCode getSyncStatus() const;
    void setSyncStatus(SyncStatusCode status);

    QString getLocalError() const;
    void setLocalError(QString){}
    QString getRemoteError() const;
    void setRemoteError(QString){}

signals:
    void syncSetupSuccess();
    void syncStatusChanged();
    void syncRemoved();
    void localErrorChanged();
    void remoteErrorChanged();

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

    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::unique_ptr<SyncController> mSyncController;
    MegaRemoteCodeError mRemoteMegaError;
    bool mCreatingFolder;
    SyncStatusCode mSyncStatus;
    QString mRemoteFolder;
    std::optional<LocalErrors> mLocalError;
    std::optional<RemoteErrors> mRemoteError;
    QString mRemoteStringMessage;

    bool checkErrorsOnSyncPaths(const QString& localPath, const QString& remotePath);
    void helperCheckLocalSync(const QString& path);
    void helperCheckRemoteSync(const QString& path);
    void cleanErrors();

private slots:
    void onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString name);
    void onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* e) override;
    void onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings);

private:
    SyncController::SyncConfig mSyncConfig;
};

#endif // SYNCS_H
