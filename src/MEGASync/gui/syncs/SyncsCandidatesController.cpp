#include "SyncsCandidatesController.h"

#include "megaapi.h"
#include "MegaApplication.h"
#include "RequestListenerManager.h"
#include "StatsEventHandler.h"
#include "SyncsData.h"

SyncsCandidatesController::SyncsCandidatesController(QObject* parent):
    Syncs(parent),
    mSyncsCandidatesModel(std::make_unique<SyncsCandidatesModel>()),
    mCurrentModelConfirmationIndex(-1)
{
    connect(&SyncController::instance(),
            &SyncController::syncPrevalidateStatus,
            this,
            &SyncsCandidatesController::onSyncPrevalidateRequestStatus);

    connect(getSyncsData(),
            &SyncsData::syncSetupSuccess,
            this,
            &SyncsCandidatesController::onSyncSetupSuccess);

    connect(getSyncsData(),
            &SyncsData::syncSetupFailed,
            this,
            &SyncsCandidatesController::onSyncSetupFailed);

    updateDefaultFolders();
}

void SyncsCandidatesController::onSyncSetupSuccess()
{
    moveNextCandidateSyncModel(false);
}

void SyncsCandidatesController::onSyncSetupFailed()
{
    moveNextCandidateSyncModel(true);
}

void SyncsCandidatesController::addSyncCandidate(const QString& localFolder,
                                                 const QString& megaFolder)
{
    mEditSyncCandidate = false;

    candidatePrevalidateHelper(localFolder, megaFolder);
}

void SyncsCandidatesController::editSyncCandidate(const QString& localFolder,
                                                  const QString& megaFolder,
                                                  const QString& originalLocalFolder,
                                                  const QString& originalMegaFolder)
{
    mEditSyncCandidate = true;
    mEditOriginalLocalFolder = originalLocalFolder;
    mEditOriginalMegaFolder = originalMegaFolder;

    candidatePrevalidateHelper(localFolder, megaFolder);
}

void SyncsCandidatesController::removeSyncCandidate(const QString& localFolder,
                                                    const QString& megaFolder)
{
    mSyncsCandidatesModel->remove(localFolder, megaFolder);
}

void SyncsCandidatesController::confirmSyncCandidates()
{
    mCurrentModelConfirmationIndex = -1;
    mCurrentModelConfirmationWithError = false;
    mCurrentModelConfirmationFull = false;

    auto syncCandidates = mSyncsCandidatesModel->rowCount();

    if (syncCandidates > 0)
    {
        // we want to know when we create more than 1 syncs at once.
        if (syncCandidates > 1)
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::SYNC_CANDIDATE_PACK_CONFIRMED);
        }

        mCurrentModelConfirmationIndex = 0;

        auto localPath = mSyncsCandidatesModel->data(
            mSyncsCandidatesModel->index(mCurrentModelConfirmationIndex, 0),
            SyncsCandidatesModel::SyncsCandidadteModelRole::LOCAL_FOLDER);

        auto megaPath = mSyncsCandidatesModel->data(
            mSyncsCandidatesModel->index(mCurrentModelConfirmationIndex, 0),
            SyncsCandidatesModel::SyncsCandidadteModelRole::MEGA_FOLDER);

        addSync(localPath.toString(), megaPath.toString());
    }
}

void SyncsCandidatesController::moveNextCandidateSyncModel(bool errorOnCurrent)
{
    mCurrentModelConfirmationWithError |= errorOnCurrent;

    // have we succesfully synced a fullpath?
    if (!mCurrentModelConfirmationWithError)
    {
        mCurrentModelConfirmationFull &= (mSyncConfig.remoteFolder == FULL_SYNC_PATH);
    }

    ++mCurrentModelConfirmationIndex;
    if (mCurrentModelConfirmationIndex < mSyncsCandidatesModel->rowCount())
    {
        auto localPath = mSyncsCandidatesModel->data(
            mSyncsCandidatesModel->index(mCurrentModelConfirmationIndex, 0),
            SyncsCandidatesModel::SyncsCandidadteModelRole::LOCAL_FOLDER);

        auto megaPath = mSyncsCandidatesModel->data(
            mSyncsCandidatesModel->index(mCurrentModelConfirmationIndex, 0),
            SyncsCandidatesModel::SyncsCandidadteModelRole::MEGA_FOLDER);

        addSync(localPath.toString(), megaPath.toString());
    }
    else
    {
        if (mCurrentModelConfirmationWithError)
        {
            emit mSyncsData->syncCandidatesSetupFailed();
        }
        else
        {
            emit mSyncsData->syncCandidatesSetupSuccess(mCurrentModelConfirmationFull);
        }

        mSyncsCandidatesModel->reset();
        mCurrentModelConfirmationIndex = -1;
        mCurrentModelConfirmationWithError = false;
        mCurrentModelConfirmationFull = false;

        updateDefaultFolders();
    }
}

void SyncsCandidatesController::candidatePrevalidateHelper(const QString& localFolder,
                                                           const QString& megaFolder)
{
    cleanErrors();

    mSyncConfig.localFolder = localFolder;
    mSyncConfig.remoteFolder = megaFolder;

    setLocalFolderCandidate(localFolder);
    setRemoteFolderCandidate(megaFolder);

    if (checkErrorsOnSyncPaths(mSyncConfig.localFolder, mSyncConfig.remoteFolder) ||
        checkCandidateAlreadyInModel(mSyncConfig.localFolder, mSyncConfig.remoteFolder))
    {
        emit mSyncsData->syncPrevalidationFailed();

        return;
    }

    auto remoteHandle = mega::INVALID_HANDLE;
    auto megaNode = std::unique_ptr<mega::MegaNode>(
        mMegaApi->getNodeByPath(mSyncConfig.remoteFolder.toUtf8().constData()));
    if (megaNode != nullptr)
    {
        remoteHandle = megaNode->getHandle();
    }

    mSyncConfig.origin = SyncInfo::SyncOrigin::ONBOARDING_ORIGIN;
    mSyncConfig.remoteHandle = remoteHandle;

    if (remoteHandle == mega::INVALID_HANDLE)
    {
        mCreatingFolder = true;

        /*
         *  need to remove the first / from the remote path,
         *  we already state in createFolder the origin point.
         */
        if (mSyncConfig.remoteFolder.indexOf(QLatin1Char('/')) == 0)
        {
            mSyncConfig.remoteFolder.remove(0, 1);
        }

        auto listener = RequestListenerManager::instance().registerAndGetFinishListener(this, true);
        mMegaApi->createFolder(mSyncConfig.remoteFolder.toUtf8().constData(),
                               MegaSyncApp->getRootNode().get(),
                               listener.get());
    }
    else
    {
        SyncController::instance().prevalidateSync(mSyncConfig);
    }
}

bool SyncsCandidatesController::checkCandidateAlreadyInModel(const QString& localPath,
                                                             const QString& remotePath)
{
    std::optional<LocalErrors> localError = mLocalError;
    if (!localError.has_value() &&
        (mEditOriginalLocalFolder.isEmpty() || localPath != mEditOriginalLocalFolder) &&
        checkExistInModel(localPath, SyncsCandidatesModel::SyncsCandidadteModelRole::LOCAL_FOLDER))
    {
        localError = LocalErrors::ALREADY_SYNC_CANDIDATE;
    }

    if (mLocalError != localError)
    {
        mLocalError.swap(localError);
        mSyncsData->setLocalError(getLocalError());
    }

    std::optional<RemoteErrors> remoteError = mRemoteError;
    if (!remoteError.has_value() &&
        (mEditOriginalMegaFolder.isEmpty() || remotePath != mEditOriginalMegaFolder) &&
        checkExistInModel(remotePath, SyncsCandidatesModel::SyncsCandidadteModelRole::MEGA_FOLDER))
    {
        remoteError = RemoteErrors::ALREADY_SYNC_CANDIDATE;
    }

    if (mRemoteError != remoteError)
    {
        mRemoteError.swap(remoteError);
        mSyncsData->setRemoteError(getRemoteError());
    }

    return mLocalError.has_value() || mRemoteError.has_value();
}

bool SyncsCandidatesController::checkExistInModel(
    const QString& path,
    SyncsCandidatesModel::SyncsCandidadteModelRole pathRole)
{
    return mSyncsCandidatesModel->exist(path, pathRole);
}

void SyncsCandidatesController::onSyncPrevalidateRequestStatus(int errorCode, int syncErrorCode)
{
    auto foundErrors = setErrorIfExist(errorCode, syncErrorCode);

    if (!foundErrors)
    {
        if (mEditSyncCandidate)
        {
            mSyncsCandidatesModel->edit(mEditOriginalLocalFolder,
                                        mEditOriginalMegaFolder,
                                        mSyncsData->getLocalFolderCandidate(),
                                        mSyncsData->getRemoteFolderCandidate());
        }
        else
        {
            mSyncsCandidatesModel->add(mSyncsData->getLocalFolderCandidate(),
                                       mSyncsData->getRemoteFolderCandidate());
        }

        emit mSyncsData->syncPrevalidationSuccess();
    }
    else
    {
        emit mSyncsData->syncPrevalidationFailed();
    }

    mEditOriginalLocalFolder.clear();
    mEditOriginalMegaFolder.clear();
}

SyncsCandidatesModel* SyncsCandidatesController::getSyncsCandidadtesModel() const
{
    return mSyncsCandidatesModel.get();
}

void SyncsCandidatesController::directoryCreatedNextTask()
{
    SyncController::instance().prevalidateSync(mSyncConfig);
}

void SyncsCandidatesController::setRemoteFolderCandidate(const QString& remoteFolderCandidate)
{
    mSyncsData->setRemoteFolderCandidate(remoteFolderCandidate);
}

void SyncsCandidatesController::setLocalFolderCandidate(const QString& localFolderCandidate)
{
    mSyncsData->setLocalFolderCandidate(localFolderCandidate);
}
