#ifndef SYNCS_H
#define SYNCS_H

#include "SyncController.h"
#include "SyncsData.h"

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

public:
    Syncs(QObject* parent = nullptr);
    virtual ~Syncs() = default;
    void addSync(const QString& localFolder, const QString& megaFolder);
    void clearRemoteError();
    void clearLocalError();
    SyncsData* getSyncsData() const;
    void setSyncOrigin(SyncInfo::SyncOrigin origin);
    void setRemoteFolder(const QString& remoteFolder);
    void updateDefaultFolders();

    static QString getDefaultMegaFolder();
    static QString getDefaultMegaPath();

    const static inline QString FULL_SYNC_PATH = QString::fromLatin1(R"(/)");
    const static inline QString DEFAULT_MEGA_FOLDER = QString::fromLatin1("MEGA");
    const static inline QString DEFAULT_MEGA_PATH = FULL_SYNC_PATH + DEFAULT_MEGA_FOLDER;

public slots:
    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* error);

private slots:
    void onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString name);
    void onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings);
    void onLanguageChanged();

protected:
    enum class LocalErrors
    {
        EMPTY_PATH,
        NO_ACCESS_PERMISSIONS_NO_EXIST,
        NO_ACCESS_PERMISSIONS_CANT_CREATE,
        CANT_SYNC,
        ALREADY_SYNC_CANDIDATE
    };

    enum class RemoteErrors
    {
        EMPTY_PATH,
        CANT_SYNC,
        CANT_CREATE_REMOTE_FOLDER,
        CANT_CREATE_REMOTE_FOLDER_MSG,
        CANT_ADD_SYNC,
        ALREADY_SYNC_CANDIDATE
    };

    struct MegaRemoteCodeError
    {
        int error;
        int syncError;
    };

    std::unique_ptr<SyncsData> mSyncsData;
    SyncController::SyncConfig mSyncConfig;
    mega::MegaApi* mMegaApi = nullptr;
    bool mCreatingFolder = false;
    std::optional<LocalErrors> mLocalError;
    std::optional<RemoteErrors> mRemoteError;

    void cleanErrors();
    bool checkErrorsOnSyncPaths(const QString& localPath, const QString& remotePath);
    QString getLocalError() const;
    QString getRemoteError() const;
    bool setErrorIfExist(int errorCode, int syncErrorCode);

private:
    SyncController& mSyncController;
    MegaRemoteCodeError mRemoteMegaError;
    QString mRemoteStringMessage;
    QString mRemoteFolder;

    std::optional<LocalErrors> helperCheckLocalSync(const QString& path);
    std::optional<RemoteErrors> helperCheckRemoteSync(const QString& path);
    virtual void directoryCreatedNextTask();
    void setDefaultLocalFolder();
    void setDefaultRemoteFolder();
    bool checkLocalSync(const QString& path);
    bool checkRemoteSync(const QString& path);
};

#endif // SYNCS_H
