#include "SyncController.h"

#include "mega/types.h"
#include "MegaApiSynchronizedRequest.h"
#include "MegaApplication.h"
#include "MyBackupsHandle.h"
#include "RequestListenerManager.h"
#include "StalledIssuesUtilities.h"
#include "StatsEventHandler.h"

#include <QStorageInfo>
#include <QTemporaryFile>

using namespace mega;

const QLatin1String MEGA_IGNORE_FILE_NAME = QLatin1String(".megaignore");

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

void SyncController::createPendingBackups(SyncInfo::SyncOrigin origin)
{
    for(auto it = mPendingBackups.cbegin(); it != mPendingBackups.cend(); it++)
    {
        SyncConfig conf;
        conf.localFolder = QDir::toNativeSeparators(it.key());
        conf.syncName = it.value().isEmpty() ? getSyncNameFromPath(it.key()) : it.value();
        conf.type = mega::MegaSync::TYPE_BACKUP;
        conf.origin = origin;
        addSync(conf);
    }
    mPendingBackups.clear();
}

void SyncController::addBackup(const QString& localFolder,
                               const QString& syncName,
                               SyncInfo::SyncOrigin origin)
{
    mPendingBackups.insert(localFolder, syncName);

    auto request = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    connect(request.get(),
            &UserAttributes::MyBackupsHandle::attributeReady,
            this,
            [this, origin]() {
                createPendingBackups(origin);
            });

    if(request->isAttributeReady())
    {
        createPendingBackups(origin);
    }
    else
    {
        request->createMyBackupsFolderIfNeeded();
    }
}

QString SyncController::getErrorString(int errorCode, int syncErrorCode) const
{
    QString errorMsg;
    if (syncErrorCode != MegaSync::NO_SYNC_ERROR)
    {
        errorMsg = QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(syncErrorCode));
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

    auto correctlyCreated = createMegaIgnoreUsingLegacyRules(sync.localFolder);

    auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [sync, correctlyCreated, this](MegaRequest* request, MegaError* e)
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

                // Remove if legacy rules .megaignore was created
                if (correctlyCreated.has_value() &&
                    correctlyCreated.value() == mega::MegaError::API_OK)
                {
                    removeMegaIgnore(sync.localFolder);
                }
            }
            else
            {
                switch (sync.origin)
                {
                    case SyncInfo::NONE:
                    // Fallthrough
                    case SyncInfo::MAIN_APP_ORIGIN:
                    // Fallthrough
                    case SyncInfo::ONBOARDING_ORIGIN:
                    // Fallthrough
                    case SyncInfo::EXTERNAL_ORIGIN:
                    // Fallthrough
                    default:
                    {
                        break;
                    }
                    case SyncInfo::CLOUD_DRIVE_DIALOG_ORIGIN:
                    {
                        MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
                            AppStatsEvents::EventType::SYNC_ADDED_CLOUD_DRIVE_BUTTON,
                            true);
                        break;
                    }
                    case SyncInfo::INFODIALOG_BUTTON_ORIGIN:
                    {
                        MegaSyncApp->getStatsEventHandler()->sendTrackedEvent(
                            AppStatsEvents::EventType::SYNC_ADDED_ADD_SYNC_BUTTON,
                            true);
                        break;
                    }
                }
            }

            emit syncAddStatus(errorCode, syncErrorCode, sync.localFolder);
            syncOperationEnds();
        });

    mSyncInfo->setSyncToCreateOrigin(sync.origin);
    mApi->syncFolder(sync.type,
                     sync.localFolder.toUtf8().constData(),
                     syncCleanName.isEmpty() ? nullptr : syncCleanName.toUtf8().constData(),
                     sync.remoteHandle,
                     nullptr,
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

    bool isBackup = syncSetting->getType() == MegaSync::TYPE_BACKUP;
    MegaHandle backupRoot = syncSetting->getMegaHandle();
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
                auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
                    this,
                    [=](MegaRequest* request, MegaError* e){
                        if (e->getErrorCode() != MegaError::API_OK)
                        {
                            QString errorMsg = QString::fromUtf8(e->getErrorString());
                            QString logMsg = QString::fromUtf8("Error moving or deleting remote backup folder (request error): %1 (sync id): %2 (Folder handle):%3").arg(
                                errorMsg, QString::number(backupId), QString::number(backupRoot));
                            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());

                            emit backupMoveOrRemoveRemoteFolderError(std::shared_ptr<MegaError>(e->copy()));
                        }
                        else
                        {
                            QString logMsg = QString::fromUtf8("Remote backup folder correctly moved or removed. (Backup id): %1 (Folder handle): %2").arg(
                                QString::number(backupId), QString::number(backupRoot));
                            MegaApi::log(MegaApi::LOG_LEVEL_INFO, logMsg.toUtf8().constData());
                        }
                    });

                // We now have to delete or remove the remote folder
                MegaSyncApp->getMegaApi()->moveOrRemoveDeconfiguredBackupNodes(backupRoot,
                                                                               remoteHandle,
                                                                               listener.get());
            }

            syncOperationEnds();
            emit syncRemoveStatus(e->getErrorCode());
        });

    mApi->removeSync(backupId, removeSyncListener.get());
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
                QString errorMsg =
                    QString::fromUtf8("Error enabling sync (%1) \"%2\" for \"%3\" to \"%4\": %5")
                        .arg(getSyncTypeString(syncSetting->getType()),
                             syncSetting->name(),
                             syncSetting->getLocalFolder(),
                             syncSetting->getMegaFolder(),
                             QString::fromUtf8(MegaSync::getMegaSyncErrorCode(syncErrorCode)));
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
                    QString logMsg = QString::fromUtf8("Error disabling sync (%1) \"%2\" for \"%3\" to \"%4\": %5").arg(
                        getSyncTypeString(syncSetting->getType()),
                        syncSetting->name(),
                        syncSetting->getLocalFolder(),
                        syncSetting->getMegaFolder(),
                        QString::fromUtf8(MegaSync::getMegaSyncErrorCode(syncErrorCode)));
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
                    QString logMsg =
                        QString::fromUtf8(
                            "Error resetting sync (%1) \"%2\" for \"%3\" to \"%4\": %5")
                            .arg(getSyncTypeString(syncSetting->getType()),
                                 syncSetting->name(),
                                 syncSetting->getLocalFolder(),
                                 syncSetting->getMegaFolder(),
                                 QString::fromUtf8(MegaSync::getMegaSyncErrorCode(syncErrorCode)));
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

    // Use canonicalPath() to resolve links
    inputPath = QDir(inputPath).canonicalPath();

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
    QDir dir(path);
    if (!dir.exists()) {
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

void SyncController::resetAllSyncsMegaIgnoreUsingLegacyRules()
{
    const auto syncsSettings = mSyncInfo->getAllSyncSettings();
    for (const auto& sync: syncsSettings)
    {
        overwriteMegaIgnoreUsingLegacyRules(sync);
    }
}

std::optional<int> SyncController::createMegaIgnoreUsingLegacyRules(const QString& syncLocalFolder)
{
    return performMegaIgnoreCreation(syncLocalFolder, mega::INVALID_HANDLE);
}

std::optional<int>
    SyncController::overwriteMegaIgnoreUsingLegacyRules(std::shared_ptr<SyncSettings> sync)
{
    return performMegaIgnoreCreation(sync->getLocalFolder(), sync->backupId());
}

std::optional<int> SyncController::performMegaIgnoreCreation(const QString& syncLocalFolder,
                                                             mega::MegaHandle backupId)
{
    std::optional<int> result(std::nullopt);

    if (Preferences::instance()->hasLegacyExclusionRules())
    {
        if (backupId != mega::INVALID_HANDLE)
        {
            removeMegaIgnore(syncLocalFolder, backupId);
        }

        std::unique_ptr<mega::MegaError> error(
            MegaSyncApp->getMegaApi()->exportLegacyExclusionRules(
                syncLocalFolder.toStdString().c_str()));

        if (error)
        {
            result = error->getErrorCode();
        }
    }

    return result;
}

bool SyncController::removeMegaIgnore(const QString& syncLocalFolder, mega::MegaHandle backupId)
{
    QFile ignoreFile(syncLocalFolder + QString::fromUtf8("/") + MEGA_IGNORE_FILE_NAME);

    if (ignoreFile.exists())
    {
        if (backupId != mega::INVALID_HANDLE)
        {
            Utilities::removeLocalFile(ignoreFile.fileName(), backupId);
        }
        else
        {
            return ignoreFile.remove();
        }
    }

    return false;
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

QString SyncController::getSyncAPIErrorMsg(int megaError)
{
    switch (megaError)
    {
        case MegaError::API_EARGS:
            return tr("Unable to create backup as selected folder is not valid. Try again.");
        break;
        case MegaError::API_EACCESS:
            return tr("Unable to create backup. Try again and if issue continues, contact [A]Support[/A].");
        break;
        case MegaError::API_EINCOMPLETE:
            return tr("Unable to create backup as the device you're backing up from doesn't have a name. "
                     "Give your device a name and then try again. If issue continues, contact [A]Support[/A].");
        case MegaError::API_EINTERNAL:
        // Fallthrough
        case MegaError::API_ENOENT:
        // Fallthrough
        case MegaError::API_EEXIST:
            return tr("Unable to create backup. For further information, contact [A]Support[/A].");
        default:
            break;
    }
    return QString();
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
