#ifndef SYNCS_H
#define SYNCS_H

#include "SyncController.h"
#include "SyncsCandidatesModel.h"

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
    void addSync(const QString& localFolder, const QString& megaFolder);
    void addSyncCandidate(const QString& localFolder, const QString& megaFolder);
    void removeSyncCandidate(const QString& localFolder, const QString& megaFolder);
    void editSyncCandidate(const QString& localFolder,
                           const QString& megaFolder,
                           const QString& originalLocalFolder,
                           const QString& originalMegaFolder);
    void clearRemoteError();
    void clearLocalError();
    SyncsData* getSyncsData() const;
    SyncsCandidatesModel* getSyncsCandidadtesModel() const;
    void confirmSyncCandidates();
    void setSyncOrigin(SyncInfo::SyncOrigin origin);

    void setRemoteFolder(const QString& remoteFolder);
    void setRemoteFolderCandidate(const QString& remoteFolderCandidate);
    void setLocalFolderCandidate(const QString& localFolderCandidate);

    static QString getDefaultMegaFolder();
    static QString getDefaultMegaPath();

public slots:
    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* error);

private slots:
    void onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString name);
    void onSyncPrevalidateRequestStatus(int errorCode, int syncErrorCode);
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
    std::unique_ptr<SyncsCandidatesModel> mSyncsCandidatesModel;

    bool mCreatingFolder = false;
    bool mOnlyPrevalidateSync = false;
    bool mEditSyncCandidate = false;
    QString mEditOriginalLocalFolder;
    QString mEditOriginalMegaFolder;
    SyncController::SyncConfig mSyncConfig;

    // vars with de command error data, used to generate error messages.
    MegaRemoteCodeError mRemoteMegaError;
    std::optional<LocalErrors> mLocalError;
    std::optional<RemoteErrors> mRemoteError;
    QString mRemoteStringMessage;
    QString mRemoteFolder;

    bool checkErrorsOnSyncPaths(const QString& localPath, const QString& remotePath);
    void helperCheckLocalSync(const QString& path);
    void helperCheckRemoteSync(const QString& path);
    void cleanErrors();
    QString getLocalError() const;
    QString getRemoteError() const;
    void setDefaultLocalFolder();
    void setDefaultRemoteFolder();
    bool checkLocalSync(const QString& path);
    bool checkRemoteSync(const QString& path);
    bool setErrorIfExist(int errorCode, int syncErrorCode);
    void syncHelper(bool onlyPrevalidate, const QString& localFolder, const QString& megaFolder);
};

#endif // SYNCS_H
