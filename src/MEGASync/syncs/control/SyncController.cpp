#include "SyncController.h"

#include "mega/types.h"
#include "MegaApiSynchronizedRequest.h"
#include "MegaApplication.h"
#include "MyBackupsHandle.h"
#include "Platform.h"
#include "RequestListenerManager.h"
#include "SyncControllerInternal.h"

#include <QScopeGuard>
#include <QStorageInfo>
#include <QTemporaryFile>

using namespace mega;

SyncController::SyncController(QObject* parent)
    : QObject(parent)
    , mPendingBackups(QMap<QString, QString>())
    , mActiveOperations(0)
    , mApi(MegaSyncApp->getMegaApi())
    , mSyncInfo(SyncInfo::instance())
{
    // The controller shouldn't ever be instantiated before we have an API and a SyncInfo available
    assert(mApi);
    assert(mSyncInfo);
}

void SyncController::createPendingBackups()
{
    for(auto it = mPendingBackups.cbegin(); it != mPendingBackups.cend(); it++)
    {
        SyncConfig conf;
        conf.localFolder = QDir::toNativeSeparators(it.key());
        conf.syncName = it.value().isEmpty() ? getSyncNameFromPath(it.key()) : it.value();
        conf.type = mega::MegaSync::TYPE_BACKUP;
        addSync(conf);
    }
    mPendingBackups.clear();
}

void SyncController::addBackup(const QString& localFolder, const QString& syncName)
{
    mPendingBackups.insert(localFolder, syncName);

    auto request = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    connect(request.get(),
            &UserAttributes::MyBackupsHandle::attributeReady,
            this,
            [this]()
            {
                createPendingBackups();
            });
    request->createMyBackupsFolderIfNeeded();
}

QString SyncController::getErrorString(int errorCode, int syncErrorCode) const
{
    QString errorMsg;
    if (syncErrorCode != MegaSync::NO_SYNC_ERROR)
    {
        std::unique_ptr<const char[]> syncErrorText(MegaSync::getMegaSyncErrorCode(syncErrorCode));
        errorMsg = QCoreApplication::translate("MegaSyncError", syncErrorText.get());
    }
    else if (errorCode != MegaError::API_OK)
    {
        errorMsg = getSyncAPIErrorMsg(errorCode);
        if(errorMsg.isEmpty())
        {
            errorMsg = QCoreApplication::translate("MegaError", MegaError::getErrorString(errorCode));
        }
    }
    return errorMsg;
}

void SyncController::prevalidateSync(SyncConfig& sync)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                 QString::fromUtf8("prevalide Sync (%1) \"%2\" for path \"%3\"")
                     .arg(getSyncTypeString(sync.type), sync.syncName, sync.localFolder)
                     .toUtf8()
                     .constData());

    QString syncCleanName = sync.syncName;
    if (syncCleanName.isEmpty())
    {
        syncCleanName = SyncController::getSyncNameFromPath(sync.localFolder);
    }
    syncCleanName.remove(Utilities::FORBIDDEN_CHARS_RX);

    auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [sync, this](MegaRequest* request, MegaError* e)
        {
            int errorCode = e->getErrorCode();
            int syncErrorCode = request->getNumDetails();

            if (syncErrorCode != MegaSync::NO_SYNC_ERROR || errorCode != MegaError::API_OK)
            {
                QString logMsg = QString::fromUtf8("Error on prevalidate sync (%1) \"%2\" for "
                                                   "\"%3\" to \"%4\" (request error): %5")
                                     .arg(getSyncTypeString(sync.type),
                                          QString::fromUtf8(request->getName()),
                                          sync.localFolder,
                                          sync.remoteFolder,
                                          getErrorString(errorCode, syncErrorCode));
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
            }

            emit syncPrevalidateStatus(errorCode, syncErrorCode);
        });

    mApi->prevalidateSyncFolder(sync.type,
                                // The SDK wants the path in utf8, even in Windows!!
                                sync.localFolder.toStdString(),
                                syncCleanName.toStdString(),
                                sync.remoteHandle,
                                std::string(),
                                listener.get());
}

void SyncController::addSync(SyncConfig& sync)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                 QString::fromUtf8("Adding sync (%1) \"%2\" for path \"%3\"")
                     .arg(getSyncTypeString(sync.type), sync.syncName, sync.localFolder)
                     .toUtf8()
                     .constData());

    syncOperationBegins();

    QString syncCleanName = sync.syncName;
    if(syncCleanName.isEmpty())
    {
        syncCleanName = SyncController::getSyncNameFromPath(sync.localFolder);
    }
    syncCleanName.remove(Utilities::FORBIDDEN_CHARS_RX);

    auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [sync, this](MegaRequest* request, MegaError* e)
        {
            int errorCode = e->getErrorCode();
            int syncErrorCode = request->getNumDetails();

            if (syncErrorCode != MegaSync::NO_SYNC_ERROR || errorCode != MegaError::API_OK)
            {
                std::unique_ptr<MegaNode> remoteNode(MegaSyncApp->getMegaApi()->getNodeByHandle(request->getNodeHandle()));
                QString logMsg =
                    QString::fromUtf8(
                        "Error adding sync (%1) \"%2\" for \"%3\" to \"%4\" (request error): %5")
                        .arg(getSyncTypeString(sync.type),
                             QString::fromUtf8(request->getName()),
                             sync.localFolder,
                             QString::fromUtf8(
                                 MegaSyncApp->getMegaApi()->getNodePath(remoteNode.get())),
                             getErrorString(errorCode, syncErrorCode));
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
            }

            emit syncAddStatus(errorCode, syncErrorCode, sync.localFolder);
            syncOperationEnds();
        });

    mApi->syncFolder(sync.type,
                     // The SDK wants the path in utf8, even in Windows!!
                     sync.localFolder.toStdString(),
                     syncCleanName.toStdString(),
                     sync.remoteHandle,
                     std::string(),
                     listener.get());
}

void SyncController::removeSync(std::shared_ptr<SyncSettings> syncSetting, const MegaHandle& remoteHandle)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Removing invalid sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Removing sync (%1) \"%2\"")
                 .arg(getSyncTypeString(syncSetting->getType()), syncSetting->name()).toUtf8().constData());

    syncOperationBegins();
    emit syncRemoveBegins(syncSetting->backupId());

    bool isBackup = syncSetting->getType() == MegaSync::TYPE_BACKUP;
    MegaHandle backupId = syncSetting->backupId();

    auto removeSyncListener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [=](MegaRequest*, MegaError* e){
            if (e->getErrorCode() != MegaError::API_OK)
            {
                QString errorMsg = QString::fromUtf8(e->getErrorString());
                std::shared_ptr<SyncSettings> sync = mSyncInfo->getSyncSettingByTag(backupId);
                QString logMsg = QString::fromUtf8("Error removing sync (%1) (request error): %2 (sync id): %3").arg(
                                     getSyncTypeString(sync ? sync->getType() : MegaSync::SyncType::TYPE_UNKNOWN),
                                     errorMsg, QString::number(backupId));
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());

                emit syncRemoveError(std::shared_ptr<MegaError>(e->copy()));
            }
            else if (isBackup)
            {
                moveOrDeleteRemovedBackupData(syncSetting, remoteHandle);
            }

            syncOperationEnds();
            emit syncRemoveStatus(e->getErrorCode());
            emit syncRemoveEnds(syncSetting->backupId());
        });

    mApi->removeSync(backupId, removeSyncListener.get());
}

void SyncController::moveOrDeleteRemovedBackupData(std::shared_ptr<SyncSettings> syncSetting,
                                                   const mega::MegaHandle& remoteHandle)
{
    MegaHandle backupRoot = syncSetting->getMegaHandle();
    MegaHandle backupId = syncSetting->backupId();

    auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [=](MegaRequest* request, MegaError* e)
        {
            if (e->getErrorCode() != MegaError::API_OK)
            {
                QString errorMsg = QString::fromUtf8(e->getErrorString());
                QString logMsg =
                    QString::fromUtf8("Error moving or deleting remote backup folder (request "
                                      "error): %1 (sync id): %2 (Folder handle):%3")
                        .arg(errorMsg, QString::number(backupId), QString::number(backupRoot));
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());

                emit backupMoveOrRemoveRemoteFolderError(std::shared_ptr<MegaError>(e->copy()));
            }
            else
            {
                QString logMsg = QString::fromUtf8("Remote backup folder correctly moved or "
                                                   "removed. (Backup id): %1 (Folder handle): %2")
                                     .arg(QString::number(backupId), QString::number(backupRoot));
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, logMsg.toUtf8().constData());
            }
        });

    // We now have to delete or remove the remote folder
    MegaSyncApp->getMegaApi()->moveOrRemoveDeconfiguredBackupNodes(backupRoot,
                                                                   remoteHandle,
                                                                   listener.get());
}

void SyncController::setSyncToRun(std::shared_ptr<SyncSettings> syncSetting)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Trying to run null sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Running sync (%1) \"%2\" to \"%3\"")
                 .arg(getSyncTypeString(syncSetting->getType()), syncSetting->getLocalFolder(), syncSetting->getMegaFolder())
                 .toUtf8().constData());

    syncOperationBegins();

    auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [=](MegaRequest*, MegaError* e)
        {
            auto syncErrorCode(static_cast<MegaSync::Error>(e->getSyncError()));
            auto errorCode(e->getErrorCode());

            updateSyncSettings(*e, syncSetting);

            if (syncSetting && syncErrorCode != MegaSync::NO_SYNC_ERROR)
            {
                std::unique_ptr<const char[]> syncErrorText(
                    MegaSync::getMegaSyncErrorCode(syncErrorCode));
                QString errorMsg =
                    QString::fromUtf8("Error enabling sync (%1) \"%2\" for \"%3\" to \"%4\": %5")
                        .arg(getSyncTypeString(syncSetting->getType()),
                             syncSetting->name(),
                             syncSetting->getLocalFolder(),
                             syncSetting->getMegaFolder(),
                             QString::fromUtf8(syncErrorText.get()));
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, errorMsg.toUtf8().constData());

                emit signalSyncOperationError(syncSetting);
            }
            else if (!syncSetting || errorCode != mega::API_OK)
            {
                QString errorMsg = getSyncAPIErrorMsg(errorCode);
                if (errorMsg.isEmpty())
                {
                    errorMsg = QString::fromUtf8(e->getErrorString());
                }

                QString logMsg =
                    QString::fromUtf8("Error enabling sync (%1) (request reason): %2")
                        .arg(getSyncTypeString(syncSetting ? syncSetting->getType() :
                                                             MegaSync::SyncType::TYPE_UNKNOWN),
                             errorMsg);
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
            }

            syncOperationEnds();
        });

    mApi->setSyncRunState(syncSetting->backupId(), MegaSync::RUNSTATE_RUNNING, listener.get());
}

void SyncController::setSyncToPause(std::shared_ptr<SyncSettings> syncSetting)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Trying to pause invalid sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Pausing sync (%1) \"%2\" to \"%3\"")
                 .arg(getSyncTypeString(syncSetting->getType()), syncSetting->getLocalFolder(), syncSetting->getMegaFolder())
                 .toUtf8().constData());

    syncOperationBegins();

    auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [=](MegaRequest*, MegaError* e){
            updateSyncSettings(*e, syncSetting);

            if (e->getErrorCode() != MegaError::API_OK)
            {
                emit signalSyncOperationError(syncSetting);
            }

            syncOperationEnds();
        });

    mApi->setSyncRunState(syncSetting->backupId(), MegaSync::RUNSTATE_SUSPENDED, listener.get());
}

void SyncController::setSyncToSuspend(std::shared_ptr<SyncSettings> syncSetting)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Trying to suspend invalid sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Suspending sync (%1) \"%2\" to \"%3\"")
                 .arg(getSyncTypeString(syncSetting->getType()), syncSetting->getLocalFolder(), syncSetting->getMegaFolder())
                 .toUtf8().constData());

    syncOperationBegins();

    auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [=](MegaRequest*, MegaError* e){
            updateSyncSettings(*e, syncSetting);

            if (e->getErrorCode() != MegaError::API_OK)
            {
                emit signalSyncOperationError(syncSetting);
            }
            syncOperationEnds();
        });

    mApi->setSyncRunState(syncSetting->backupId(), MegaSync::RUNSTATE_SUSPENDED, listener.get());
}

void SyncController::setSyncToDisabled(std::shared_ptr<SyncSettings> syncSetting)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Trying to disable invalid sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Disabling sync (%1) \"%2\" to \"%3\"")
                 .arg(getSyncTypeString(syncSetting->getType()), syncSetting->getLocalFolder(), syncSetting->getMegaFolder())
                 .toUtf8().constData());

    syncOperationBegins();

    auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [=](MegaRequest*, MegaError* e){
            if (!syncSetting)
            {
                return;
            }

            updateSyncSettings(*e, syncSetting);

            // NOTE: As of sdk commit 94e2b9dd1db6a886e21cc1ee826bda58c8c33f99, this never fails
            // and errorCode is always MegaError::API_OK.
            auto errorCode (e->getErrorCode());
            if (errorCode != MegaError::API_OK)
            {
                emit signalSyncOperationError(syncSetting);
                auto syncErrorCode (static_cast<MegaSync::Error>(e->getSyncError()));

                if (syncErrorCode != MegaSync::NO_SYNC_ERROR)
                {
                    std::unique_ptr<const char[]> syncErrorText(
                        MegaSync::getMegaSyncErrorCode(syncErrorCode));
                    QString logMsg =
                        QString::fromUtf8(
                            "Error disabling sync (%1) \"%2\" for \"%3\" to \"%4\": %5")
                            .arg(getSyncTypeString(syncSetting->getType()),
                                 syncSetting->name(),
                                 syncSetting->getLocalFolder(),
                                 syncSetting->getMegaFolder(),
                                 QString::fromUtf8(syncErrorText.get()));
                    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
                }
                else if (!syncSetting || errorCode != mega::API_OK)
                {
                    QString errorMsg = getSyncAPIErrorMsg(errorCode);
                    if (errorMsg.isEmpty())
                    {
                        errorMsg = QString::fromUtf8(e->getErrorString());
                    }

                    QString logMsg = QString::fromUtf8("Error disabling sync (%1) (request error): %2").arg(
                        getSyncTypeString(syncSetting ? syncSetting->getType() : MegaSync::SyncType::TYPE_UNKNOWN),
                        errorMsg);
                    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
                }
            }

            syncOperationEnds();
        });

    mApi->setSyncRunState(syncSetting->backupId(), MegaSync::RUNSTATE_DISABLED, listener.get());
}

void SyncController::resetSync(std::shared_ptr<SyncSettings> syncSetting,
                               MegaSync::SyncRunningState initialState)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Trying to run null sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                 QString::fromUtf8("Resetting sync (%1) \"%2\" to \"%3\"")
                     .arg(getSyncTypeString(syncSetting->getType()),
                          syncSetting->getLocalFolder(),
                          syncSetting->getMegaFolder())
                     .toUtf8()
                     .constData());

    std::shared_ptr<mega::MegaError> error(nullptr);
    auto processResult = [this, error, syncSetting]()
    {
        if (error)
        {
            auto errorCode(error->getErrorCode());
            if (errorCode != MegaError::API_OK)
            {
                emit signalSyncOperationError(syncSetting);
                auto syncErrorCode(static_cast<MegaSync::Error>(error->getSyncError()));

                if (syncSetting && syncErrorCode != MegaSync::NO_SYNC_ERROR)
                {
                    std::unique_ptr<const char[]> syncErrorText(
                        MegaSync::getMegaSyncErrorCode(syncErrorCode));
                    QString logMsg =
                        QString::fromUtf8(
                            "Error resetting sync (%1) \"%2\" for \"%3\" to \"%4\": %5")
                            .arg(getSyncTypeString(syncSetting->getType()),
                                 syncSetting->name(),
                                 syncSetting->getLocalFolder(),
                                 syncSetting->getMegaFolder(),
                                 QString::fromUtf8(syncErrorText.get()));
                    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
                }
                else if (!syncSetting || errorCode != mega::API_OK)
                {
                    QString errorMsg = getSyncAPIErrorMsg(errorCode);
                    if (errorMsg.isEmpty())
                    {
                        errorMsg = QString::fromUtf8(error->getErrorString());
                    }

                    QString logMsg =
                        QString::fromUtf8("Error resetting sync (%1) (request error): %2")
                            .arg(getSyncTypeString(syncSetting ? syncSetting->getType() :
                                                                 MegaSync::SyncType::TYPE_UNKNOWN),
                                 errorMsg);
                    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
                }
            }
        }

        syncOperationEnds();
    };

    syncOperationBegins();

    MegaApiSynchronizedRequest::runRequestWithResult(
        &mega::MegaApi::setSyncRunState,
        MegaSyncApp->getMegaApi(),
        [&error, syncSetting, processResult](mega::MegaRequest*, mega::MegaError* e)
        {
            if (e->getErrorCode() == mega::MegaError::API_OK)
            {
                MegaApiSynchronizedRequest::runRequestWithResult(
                    &mega::MegaApi::setSyncRunState,
                    MegaSyncApp->getMegaApi(),
                    [&error, processResult](mega::MegaRequest*, mega::MegaError* e)
                    {
                        if (e->getErrorCode() != mega::MegaError::API_OK)
                        {
                            error.reset(e->copy());
                        }
                        processResult();
                    },
                    syncSetting->backupId(),
                    MegaSync::RUNSTATE_RUNNING);
            }
            else
            {
                error.reset(e->copy());
                processResult();
            }
        },
        syncSetting->backupId(),
        initialState);
}

namespace SyncControllerInternal
{

ScopedBackupIdGuard::ScopedBackupIdGuard(QSet<mega::MegaHandle>& set, mega::MegaHandle id):
    mSet(set),
    mId(id)
{
    mSet.insert(mId);
}

ScopedBackupIdGuard::~ScopedBackupIdGuard()
{
    mSet.remove(mId);
}

bool isAlreadyStoppedState(mega::MegaSync::SyncRunningState runState)
{
    return runState == mega::MegaSync::RUNSTATE_SUSPENDED ||
           runState == mega::MegaSync::RUNSTATE_DISABLED;
}

PauseRunAndResumeOutcome pauseRunAndResumeCore(const std::shared_ptr<SyncSettings>& syncSetting,
                                               const std::function<void()>& duringPause,
                                               QSet<mega::MegaHandle>& inProgress,
                                               const SetSyncRunStateRequest& request)
{
    if (!syncSetting)
    {
        return PauseRunAndResumeOutcome::NullSync;
    }

    if (inProgress.contains(syncSetting->backupId()))
    {
        // A previous call for this same sync is still waiting on the SDK (this call is a
        // nested QEventLoop away, so a reentrant click can reach here). Running the
        // mutation again now would race the one already in flight.
        return PauseRunAndResumeOutcome::Reentrant;
    }
    ScopedBackupIdGuard inProgressGuard(inProgress, syncSetting->backupId());

    if (isAlreadyStoppedState(
            static_cast<mega::MegaSync::SyncRunningState>(syncSetting->getRunState())))
    {
        // Already stopped by the user: nothing can be actively scanning the folder, so
        // there's no need to force a state change - just run the mutation directly. The
        // SDK will regenerate .megaignore whenever this sync is next resumed. PENDING and
        // LOADING are deliberately not included here: both are on their way to RUNNING on
        // their own, so they need the same protection as an already-running sync.
        if (duringPause)
        {
            duringPause();
        }
        return PauseRunAndResumeOutcome::AlreadyStopped;
    }

    // Both calls block (via a nested QEventLoop, inside the real request implementation)
    // until the sync has actually finished transitioning, so duringPause() is guaranteed
    // to run only while it is suspended.
    const auto suspendError = request(syncSetting->backupId(), mega::MegaSync::RUNSTATE_SUSPENDED);
    if (suspendError && suspendError->getErrorCode() != mega::MegaError::API_OK)
    {
        return PauseRunAndResumeOutcome::SuspendFailed;
    }

    if (duringPause)
    {
        duringPause();
    }

    const auto resumeError = request(syncSetting->backupId(), mega::MegaSync::RUNSTATE_RUNNING);
    if (resumeError && resumeError->getErrorCode() != mega::MegaError::API_OK)
    {
        return PauseRunAndResumeOutcome::ResumeFailed;
    }

    return PauseRunAndResumeOutcome::Completed;
}

} // namespace SyncControllerInternal

void SyncController::pauseRunAndResume(std::shared_ptr<SyncSettings> syncSetting,
                                       std::function<void()> duringPause)
{
    using namespace SyncControllerInternal;

    const SetSyncRunStateRequest request =
        [](mega::MegaHandle backupId, MegaSync::SyncRunningState state)
    {
        return MegaApiSynchronizedRequest::runRequestWithResult(&mega::MegaApi::setSyncRunState,
                                                                MegaSyncApp->getMegaApi(),
                                                                nullptr,
                                                                backupId,
                                                                state);
    };

    syncOperationBegins();
    // Runs even if pauseRunAndResumeCore() (specifically duringPause(), which runs
    // arbitrary caller-supplied code) throws, so the operation accounting stays
    // balanced either way.
    const auto endOperationOnExit = qScopeGuard(
        [this]()
        {
            syncOperationEnds();
        });

    const auto outcome =
        pauseRunAndResumeCore(syncSetting, duringPause, mPauseRunAndResumeInProgress, request);

    switch (outcome)
    {
        case PauseRunAndResumeOutcome::NullSync:
            MegaApi::log(
                MegaApi::LOG_LEVEL_ERROR,
                QString::fromUtf8("Trying to pause and resume a null sync").toUtf8().constData());
            break;

        case PauseRunAndResumeOutcome::Reentrant:
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING,
                         QString::fromUtf8("Ignoring reentrant pause/resume for sync (%1) \"%2\"")
                             .arg(getSyncTypeString(syncSetting->getType()), syncSetting->name())
                             .toUtf8()
                             .constData());
            break;

        case PauseRunAndResumeOutcome::AlreadyStopped:
        case PauseRunAndResumeOutcome::Completed:
            break;

        case PauseRunAndResumeOutcome::SuspendFailed:
            MegaApi::log(
                MegaApi::LOG_LEVEL_ERROR,
                QString::fromUtf8("Failed to pause sync (%1) \"%2\" for a local-only change")
                    .arg(getSyncTypeString(syncSetting->getType()), syncSetting->name())
                    .toUtf8()
                    .constData());
            emit signalSyncOperationError(syncSetting);
            break;

        case PauseRunAndResumeOutcome::ResumeFailed:
            MegaApi::log(
                MegaApi::LOG_LEVEL_ERROR,
                QString::fromUtf8("Failed to resume sync (%1) \"%2\" after a local-only change")
                    .arg(getSyncTypeString(syncSetting->getType()), syncSetting->name())
                    .toUtf8()
                    .constData());
            emit signalSyncOperationError(syncSetting);
            break;
    }
}

void SyncController::updateSyncSettings(const MegaError& e, std::shared_ptr<SyncSettings> syncSetting)
{
    if (syncSetting &&
        (e.getSyncError() != MegaSync::NO_SYNC_ERROR ||
         e.getErrorCode() != mega::API_OK))
    {
        if(syncSetting)
        {
            //If the sync state change has failed, update the sync object to get the latest errors
            syncSetting->setSync(MegaSyncApp->getMegaApi()->getSyncByBackupId(syncSetting->backupId()));
        }
    }
}

// Checks if a path belongs is in an existing sync or backup tree; and if the selected
// folder has a sync or backup in its tree.
QString SyncController::getIsLocalFolderAlreadySyncedMsg(const QString& path, const MegaSync::SyncType& syncType)
{
    QString inputPath (QDir::toNativeSeparators(QDir(path).absolutePath()));
    inputPath = inputPath.normalized(QString::NormalizationForm_C);

    QString message;

    // Gather all synced or backed-up dirs
    QMap<QString, MegaSync::SyncType> localFolders = SyncInfo::instance()->getLocalFoldersAndTypeMap(true);

    // Check if the path is already synced or part of a sync
    foreach (const QString& existingPath, localFolders.keys())
    {
        if (existingPath == inputPath)
        {
            if (syncType == MegaSync::SyncType::TYPE_BACKUP)
            {
                message = localFolders.value(existingPath) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("Folder can't be backed up as it is already synced.")
                          : tr("Folder is already backed up. Select a different one.");
            }
            else
            {
                message = localFolders.value(existingPath) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("Folder can't be synced as it's already synced")
                          : tr("Folder can't be synced as is already backed up");
            }
        }
        else if (inputPath.startsWith(existingPath)
                 && inputPath[existingPath.size() - QDir(existingPath).isRoot()] == QDir::separator())
        {
            if (syncType == MegaSync::SyncType::TYPE_BACKUP)
            {
                message = localFolders.value(existingPath) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't backup this folder as it's already inside a synced folder.")
                          : getErrStrCurrentBackupInsideExistingBackup();
            }
            else
            {
                message = localFolders.value(existingPath) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("Folder can't be synced as it's inside a synced folder")
                          : tr("Folder can't be synced as it's inside a backed up folder");
            }
        }
        else if (existingPath.startsWith(inputPath)
                 && existingPath[inputPath.size() - QDir(inputPath).isRoot()] == QDir::separator())
        {
            if (syncType == MegaSync::SyncType::TYPE_BACKUP)
            {
                message = localFolders.value(existingPath) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't backup this folder as it contains synced folders.")
                          : getErrStrCurrentBackupOverExistingBackup();
            }
            else
            {
                message = localFolders.value(existingPath) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("Folder can't be synced as it contains synced folders")
                          : tr("Folder can't be synced as it contains backed up folders");
            }
        }
    }
    return message;
}

SyncController::Syncability SyncController::isLocalFolderAlreadySynced(const QString& path, const MegaSync::SyncType &syncType, QString& message)
{
    message = getIsLocalFolderAlreadySyncedMsg(path, syncType);
    return (message.isEmpty() ? Syncability::CAN_SYNC : Syncability::CANT_SYNC);
}

QString SyncController::getIsLocalFolderAllowedForSyncMsg(const QString& path, const MegaSync::SyncType& syncType)
{
    QString inputPath (path);
    QString message;

#ifdef WIN32
    if (inputPath.startsWith(QString::fromLatin1("\\\\?\\")))
    {
        inputPath = inputPath.mid(4);
    }
#endif

    inputPath = Platform::getInstance()->preparePathForSync(inputPath);

    if (inputPath == QDir::rootPath())
    {
        if (syncType == MegaSync::SyncType::TYPE_BACKUP)
        {
            message = tr("Can't backup “%1” as it's the root folder. To continue, select a different folder").arg(inputPath);
        }
        else
        {
            message = tr("Can't sync “%1” as it's the root folder. To continue, select a different folder").arg(inputPath);
        }
    }
    return message;
}

SyncController::Syncability SyncController::isLocalFolderAllowedForSync(const QString& path, const MegaSync::SyncType &syncType, QString& message)
{
    message = getIsLocalFolderAllowedForSyncMsg(path, syncType);
    return (message.isEmpty() ? Syncability::CAN_SYNC : Syncability::CANT_SYNC);
}

// Returns wether the path is syncable.
// The message to display to the user is stored in <message>.
// The first error encountered is returned.
// Errors trump warnings
// In case of several warnings, only the last one is returned.
SyncController::Syncability SyncController::isLocalFolderSyncable(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message)
{
    // Check if the directory exists
    bool isLink = false;
#ifdef Q_OS_WINDOWS
    isLink = path.endsWith(QLatin1String(".lnk"));
#endif
    if (isLink || !QDir(path).exists())
    {
        message = QCoreApplication::translate("MegaSyncError", "Local path not available");
        return Syncability::CANT_SYNC;
    }

    Syncability syncability (Syncability::CAN_SYNC);

    // First check if the path is allowed
    syncability = isLocalFolderAllowedForSync(path, syncType, message);

    // The check if it is not synced already
    if (syncability != Syncability::CANT_SYNC)
    {
        syncability = std::max(isLocalFolderAlreadySynced(path, syncType, message), syncability);
    }

    // We no longer check if the local folder has the correct rights, the SDK does.

    return (syncability);
}

SyncController::Syncability
    SyncController::isLocalFolderSyncable(const QString& path,
                                          const mega::MegaSync::SyncType& syncType)
{
    QString message;
    return isLocalFolderSyncable(path, syncType, message);
}

// Returns wether the remote path is syncable.
// The message to display to the user is stored in <message>.
SyncController::Syncability SyncController::isRemoteFolderSyncable(std::shared_ptr<mega::MegaNode> node, QString& message)
{
    Syncability syncability (Syncability::CANT_SYNC);
    std::unique_ptr<MegaError> err (MegaSyncApp->getMegaApi()->isNodeSyncableWithError(node.get()));

    if (err->getErrorCode() != MegaError::API_OK)
    {
        message = getRemoteFolderErrorMessage(err->getErrorCode(), err->getSyncError());
    }
    else
    {
        syncability = Syncability::CAN_SYNC;
    }

    return (syncability);
}

QString SyncController::getRemoteFolderErrorMessage(int errorCode, int syncErrorCode)
{
    QString message;

    switch (errorCode)
    {
        case MegaError::API_EACCESS:
        {
            switch (syncErrorCode)
            {
                case SyncError::SHARE_NON_FULL_ACCESS:
                {
                    message = tr("You don't have enough permissions for this remote folder.");
                    break;
                }
                case SyncError::REMOTE_NODE_INSIDE_RUBBISH:
                {
                    message = tr("Folder can't be synced as it's in the MEGA Rubbish bin.");
                    break;
                }
                case SyncError::INVALID_REMOTE_TYPE:
                {
                    message = tr("This selection can't be synced as it’s a file.");
                    break;
                }
            }
            break;
        }
        case MegaError::API_EEXIST:
        {
            switch (syncErrorCode)
            {
                case SyncError::ACTIVE_SYNC_SAME_PATH:
                {
                    message = tr("This folder is already being synced.");
                    break;
                }
                case SyncError::ACTIVE_SYNC_BELOW_PATH:
                {
                    message = tr("Folder contents already synced.");
                    break;
                }
                case SyncError::ACTIVE_SYNC_ABOVE_PATH:
                {
                    message = tr("Folder already synced.");
                    break;
                }
            }
            break;
        }
        case MegaError::API_ENOENT:
        case MegaError::API_EARGS:
        default:
        {
            message = tr("Invalid remote path.");
            break;
        }
    }

    return message;
}

Qt::CaseSensitivity SyncController::isSyncCaseSensitive(mega::MegaHandle backupId)
{
    if (auto syncSettings = SyncInfo::instance()->getSyncSettingByTag(backupId))
    {
        return Utilities::isCaseSensitive(syncSettings->getLocalFolder());
    }

#ifdef Q_OS_LINUX
    return Qt::CaseSensitive;
#else
    return Qt::CaseInsensitive;
#endif
}

QString SyncController::getSyncNameFromPath(const QString& path)
{
    QDir dir (path);
    QString syncName;

    // Handle fs root case
    if (dir.isRoot())
    {
        // Cleanup the path (in Windows: get "F:" from "F:\")
        QString cleanPath (QDir::toNativeSeparators(dir.absolutePath()).remove(QDir::separator()));
        // Try to get the volume label
        QStorageInfo storage(dir);
        QString label (QString::fromUtf8(storage.subvolume()));
        if (label.isEmpty())
        {
            label = storage.name();
        }
        // If we have no label, fallback to the cleaned path
        syncName = label.isEmpty() ? cleanPath
                                   : QString::fromUtf8("%1 (%2)").arg(label, cleanPath);
    }
    else
    {
        // Take the folder name as sync name
        syncName = dir.dirName();
    }

    return syncName;
}

QString SyncController::getErrStrCurrentBackupOverExistingBackup()
{
    return tr("You can't backup this folder as it contains backed up folders.");
}

QString SyncController::getErrStrCurrentBackupInsideExistingBackup()
{
    return tr("You can't backup this folder as it's already inside a backed up folder.");
}

QString SyncController::getSyncAPIErrorMsg(int megaError) const
{
    switch (megaError)
    {
        case MegaError::API_EARGS:
            return tr("Unable to create sync as selected folder is not valid. Try again.");
            break;
        case MegaError::API_EACCESS:
            return tr("Unable to create sync. Try again and if issue continues, contact "
                      "[A]Support[/A].");
            break;
        case MegaError::API_EINCOMPLETE:
        // Fallthrough
        case MegaError::API_EINTERNAL:
        // Fallthrough
        case MegaError::API_ENOENT:
        // Fallthrough
        case MegaError::API_EEXIST:
            return tr("Unable to create sync. For further information, contact [A]Support[/A].");
        default:
            break;
    }
    return {};
}

QString SyncController::getSyncTypeString(const mega::MegaSync::SyncType& syncType)
{
    QString typeString;
    switch (syncType)
    {
        case MegaSync::SyncType::TYPE_TWOWAY:
        {
            typeString = QLatin1String("Two-way");
            break;
        }
        case MegaSync::SyncType::TYPE_BACKUP:
        {
            typeString = QLatin1String("Backup");
            break;
        }
        case MegaSync::SyncType::TYPE_UNKNOWN:
        default:
        {
            typeString = QLatin1String("Unknown");
            break;
        }
    }
    return typeString;
}

//This system has been designed for Backups, as several backups can be created in a row
//and only after the last one the signal should be sent
void SyncController::syncOperationBegins()
{
    emit signalSyncOperationBegins();
    ++mActiveOperations;
}

void SyncController::syncOperationEnds()
{
    if(--mActiveOperations == 0)
    {
        emit signalSyncOperationEnds();
    }
}
