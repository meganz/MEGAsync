#ifndef SYNCS_H
#define SYNCS_H

#include "megaapi.h"
#include "SyncController.h"

#include <QObject>

#include <memory>
#include <optional>

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

    static inline const QString DEFAULT_MEGA_FOLDER = QString::fromUtf8("MEGA");
    static inline const QString DEFAULT_MEGA_PATH =
        QString::fromUtf8("/") + Syncs::DEFAULT_MEGA_FOLDER;

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
    QString getRemoteError() const;
    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* error);

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
    std::unique_ptr<SyncController> mSyncController;

    bool mCreatingFolder;
    SyncStatusCode mSyncStatus;

    // vars with de command error return data.
    MegaRemoteCodeError mRemoteMegaError;
    std::optional<LocalErrors> mLocalError;
    std::optional<RemoteErrors> mRemoteError;
    QString mRemoteStringMessage;

    bool checkErrorsOnSyncPaths(const QString& localPath, const QString& remotePath);
    void helperCheckLocalSync(const QString& path);
    void helperCheckRemoteSync(const QString& path);
    void cleanErrors();

private slots:
    void onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString name);
    void onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings);

private:
    SyncController::SyncConfig mSyncConfig;
};

#endif // SYNCS_H
