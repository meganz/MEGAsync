#include "SyncsCandidates.h"

#include "megaapi.h"
#include "MegaApplication.h"
#include "RequestListenerManager.h"
#include "StatsEventHandler.h"
#include "SyncsData.h"

SyncsCandidates::SyncsCandidates(QObject* parent):
    Syncs(parent),
    mSyncsCandidatesModel(std::make_unique<SyncsCandidatesModel>()),
    mCurrentModelConfirmationIndex(-1)
{
    connect(&SyncController::instance(),
            &SyncController::syncPrevalidateStatus,
            this,
            &SyncsCandidates::onSyncPrevalidateRequestStatus);

    connect(getSyncsData(),
            &SyncsData::syncSetupSuccess,
            this,
            &SyncsCandidates::onSyncSetupSuccess);

    connect(getSyncsData(), &SyncsData::syncSetupFailed, this, &SyncsCandidates::onSyncSetupFailed);

    updateDefaultFolders();
}

void SyncsCandidates::onSyncSetupSuccess()
{
    moveNextCandidateSyncModel(false);
}

void SyncsCandidates::onSyncSetupFailed()
{
    moveNextCandidateSyncModel(true);
}

void SyncsCandidates::addSyncCandidate(const QString& localFolder, const QString& megaFolder)
{
    mEditSyncCandidate = false;

    candidatePrevalidateHelper(localFolder, megaFolder);
}

void SyncsCandidates::editSyncCandidate(const QString& localFolder,
                                        const QString& megaFolder,
                                        const QString& originalLocalFolder,
                                        const QString& originalMegaFolder)
{
    mEditSyncCandidate = true;
    mEditOriginalLocalFolder = originalLocalFolder;
    mEditOriginalMegaFolder = originalMegaFolder;

    candidatePrevalidateHelper(localFolder, megaFolder);
}

void SyncsCandidates::removeSyncCandidate(const QString& localFolder, const QString& megaFolder)
{
    mSyncsCandidatesModel->remove(localFolder, megaFolder);
}

void SyncsCandidates::confirmSyncCandidates()
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

void SyncsCandidates::moveNextCandidateSyncModel(bool errorOnCurrent)
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

void SyncsCandidates::candidatePrevalidateHelper(const QString& localFolder,
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

bool SyncsCandidates::checkCandidateAlreadyInModel(const QString& localPath,
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

bool SyncsCandidates::checkExistInModel(const QString& path,
                                        SyncsCandidatesModel::SyncsCandidadteModelRole pathRole)
{
    return mSyncsCandidatesModel->exist(path, pathRole);
}

void SyncsCandidates::onSyncPrevalidateRequestStatus(int errorCode, int syncErrorCode)
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

SyncsCandidatesModel* SyncsCandidates::getSyncsCandidadtesModel() const
{
    return mSyncsCandidatesModel.get();
}

void SyncsCandidates::directoryCreatedNextTask()
{
    SyncController::instance().prevalidateSync(mSyncConfig);
}

void SyncsCandidates::setRemoteFolderCandidate(const QString& remoteFolderCandidate)
{
    mSyncsData->setRemoteFolderCandidate(remoteFolderCandidate);
}

void SyncsCandidates::setLocalFolderCandidate(const QString& localFolderCandidate)
{
    mSyncsData->setLocalFolderCandidate(localFolderCandidate);
}
