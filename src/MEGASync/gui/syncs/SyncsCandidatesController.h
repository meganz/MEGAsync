#ifndef SYNCS_CANDIDATES_CONTROLLER_H
#define SYNCS_CANDIDATES_CONTROLLER_H

#include "Syncs.h"
#include "SyncsCandidatesModel.h"

#include <QObject>

#include <memory>

class SyncsCandidatesController: public Syncs
{
    Q_OBJECT

public:
    SyncsCandidatesController(QObject* parent = nullptr);
    virtual ~SyncsCandidatesController() = default;

    void addSyncCandidate(const QString& localFolder, const QString& megaFolder);
    void removeSyncCandidate(const QString& localFolder, const QString& megaFolder);
    void editSyncCandidate(const QString& localFolder,
                           const QString& megaFolder,
                           const QString& originalLocalFolder,
                           const QString& originalMegaFolder);
    SyncsCandidatesModel* getSyncsCandidadtesModel() const;
    void confirmSyncCandidates();
    void setRemoteFolderCandidate(const QString& remoteFolderCandidate);
    void setLocalFolderCandidate(const QString& localFolderCandidate);

private slots:
    void onSyncPrevalidateRequestStatus(int errorCode, int syncErrorCode);
    void onSyncSetupSuccess();
    void onSyncSetupFailed();

private:
    std::unique_ptr<SyncsCandidatesModel> mSyncsCandidatesModel;
    int mCurrentModelConfirmationIndex;
    bool mCurrentModelConfirmationWithError;
    bool mCurrentModelConfirmationFull;
    bool mEditSyncCandidate = false;
    QString mEditOriginalLocalFolder;
    QString mEditOriginalMegaFolder;

    void directoryCreatedNextTask() override;
    bool checkCandidateAlreadyInModel(const QString& localPath, const QString& remotePath);
    void candidatePrevalidateHelper(const QString& localFolder, const QString& megaFolder);
    void moveNextCandidateSyncModel(bool errorOnCurrent);
    bool checkExistInModel(const QString& path,
                           SyncsCandidatesModel::SyncsCandidadteModelRole pathRole);
};

#endif // SYNCS_CANDIDATES_CONTROLLER_H
