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

class SyncsData;
class Syncs: public QObject
{
    Q_OBJECT

public:
    Syncs(QObject* parent = nullptr);
    virtual ~Syncs() = default;

    void addSync(SyncInfo::SyncOrigin origin,
                 const QString& local,
                 const QString& remote = QLatin1String("/"));
    bool checkLocalSync(const QString& path);
    bool checkRemoteSync(const QString& path);
    void clearRemoteError();
    void clearLocalError();

    QString getLocalError() const;
    QString getRemoteError() const;

    SyncsData* getSyncsData() const;

public slots:
    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* error);

private slots:
    void onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString name);
    void onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings);
    void onLanguageChanged();

private:
    enum class LocalErrors
    {
        EMPTY_PATH,
        NO_ACCESS_PERMISSIONS_NO_EXIST,
        NO_ACCESS_PERMISSIONS_CANT_CREATE,
        CANT_SYNC
    };

    enum class RemoteErrors
    {
        EMPTY_PATH,
        CANT_SYNC,
        CANT_CREATE_REMOTE_FOLDER,
        CANT_CREATE_REMOTE_FOLDER_MSG,
        CANT_ADD_SYNC
    };

    struct MegaRemoteCodeError
    {
        int error;
        int syncError;
    };

    mega::MegaApi* mMegaApi = nullptr;
    SyncController& mSyncController;
    std::unique_ptr<SyncsData> mSyncsData;

    bool mCreatingFolder = false;
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
