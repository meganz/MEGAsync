#include "SyncController.h"
#include "MegaApplication.h"
#include "Platform.h"

#include <QInputDialog>
#include <QStorageInfo>
#include <QTemporaryFile>

using namespace mega;

const char* SyncController::DEFAULT_BACKUPS_ROOT_DIRNAME = "My backups";

SyncController::SyncController(QObject* parent)
    : QObject(parent),
      mApi(MegaSyncApp->getMegaApi()),
      mDelegateListener (new QTMegaRequestListener(mApi, this)),
      mSyncModel(SyncModel::instance()),
      mMyBackupsHandle(INVALID_HANDLE),
      mIsDeviceNameSetOnRemote(false),
      mForceSetDeviceName(false)
{
    // The controller shouldn't ever be instantiated before we have an API and a SyncModel available
    assert(mApi);
    assert(mSyncModel);
}

SyncController::~SyncController()
{
    delete mDelegateListener;
}

void SyncController::addBackup(const QString& localFolder, const QString& syncName)
{
    addSync(QDir::toNativeSeparators(localFolder), mega::INVALID_HANDLE,
            syncName.isEmpty() ? getSyncNameFromPath(localFolder) : syncName,
            mega::MegaSync::TYPE_BACKUP);
}

void SyncController::addSync(const QString& localFolder, const MegaHandle& remoteHandle,
                             const QString& syncName, MegaSync::SyncType type)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Adding sync \"%1\" for path \"%2\"")
                 .arg(syncName, localFolder).toUtf8().constData());

    mApi->syncFolder(type, localFolder.toUtf8().constData(),
                     syncName.isEmpty() ? nullptr : syncName.toUtf8().constData(),
                     remoteHandle, nullptr, mDelegateListener);
}

void SyncController::removeSync(std::shared_ptr<SyncSetting> syncSetting, const MegaHandle& remoteHandle)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Removing invalid sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Removing sync \"%1\"")
                 .arg(syncSetting->name()).toUtf8().constData());

    mApi->removeSync(syncSetting->backupId(), remoteHandle, mDelegateListener);
    // FIXME: There is a bug in SyncModel class handling that persists the saved Backup entry after SDK delete
}

void SyncController::enableSync(std::shared_ptr<SyncSetting> syncSetting)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Trying to enable null sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Enabling sync \"%1\" to \"%2\"")
                 .arg(syncSetting->getLocalFolder(), syncSetting->getMegaFolder())
                 .toUtf8().constData());

    mApi->enableSync(syncSetting->backupId(), mDelegateListener);
}

void SyncController::disableSync(std::shared_ptr<SyncSetting> syncSetting)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Trying to disable invalid sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Disabling sync \"%1\" to \"%2\"")
                 .arg(syncSetting->getLocalFolder(), syncSetting->getMegaFolder())
                 .toUtf8().constData());

    mApi->disableSync(syncSetting->backupId(), mDelegateListener);
}

void SyncController::setDeviceName(const QString& name)
{
    if (!mIsDeviceNameSetOnRemote)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                     QString::fromUtf8("Setting device name to \"%1\" on remote")
                     .arg(name).toUtf8().constData());
        mApi->setDeviceName(name.toUtf8().constData(), mDelegateListener);
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                     QString::fromUtf8("Set device name on remote: already set")
                     .toUtf8().constData());
        emit setDeviceDirStatus(MegaError::API_OK, QString());
    }
}

void SyncController::getDeviceName()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Request device name");
    ensureDeviceNameIsSetOnRemote();
}

// Checks if a path belongs is in an existing sync or backup tree; and if the selected
// folder has a sync or backup in its tree.
QString SyncController::getIsLocalFolderAlreadySyncedMsg(const QString& path, const MegaSync::SyncType& syncType)
{
    QString inputPath (QDir::toNativeSeparators(QDir(path).absolutePath()));
    QString message;

    // Gather all synced or backed-up dirs
    QMap<QString, MegaSync::SyncType> localFolders = SyncModel::instance()->getLocalFoldersAndTypeMap();

    // Check if the path is already synced or part of a sync
    foreach (auto& existingPath, localFolders.keys())
    {
        if (inputPath == existingPath)
        {
            if (syncType == MegaSync::SyncType::TYPE_BACKUP)
            {
                message = localFolders.value(existingPath) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't sync this folder as it's already synced.")
                          : tr("Folder is already backed up. Select a different one.");
            }
            else
            {
                message = localFolders.value(existingPath) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't sync this folder as it's already synced.")
                          : tr("You can't sync this folder as it's backed up.");
            }
        }
        else if (inputPath.startsWith(existingPath)
                 && inputPath[existingPath.size() - QDir(existingPath).isRoot()] == QDir::separator())
        {
            if (syncType == MegaSync::SyncType::TYPE_BACKUP)
            {
                message = localFolders.value(existingPath) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't backup this folder as it's already inside a synced folder.")
                          : tr("You can't backup this folder as it's already inside a backed up folder.");
            }
            else
            {
                message = localFolders.value(existingPath) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't sync folders that are inside synced folders.")
                          : tr("You can't sync folders that are inside backed up folders. ");
            }
        }
        else if (existingPath.startsWith(inputPath)
                 && existingPath[inputPath.size() - QDir(inputPath).isRoot()] == QDir::separator())
        {
            if (syncType == MegaSync::SyncType::TYPE_BACKUP)
            {
                message = localFolders.value(existingPath) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't backup this folder as it contains synced folders.")
                          : tr("You can't backup this folder as it contains backed up folders.");
            }
            else
            {
                message = localFolders.value(existingPath) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't sync folders that contain synced folders.")
                          : tr("You can't sync folders that contain backed up folders.  ");
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

QString SyncController::getIsLocalPathAllowedForSyncMsg(const QString& path, const MegaSync::SyncType& syncType)
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
            message = tr("You are trying to backup an extremely large folder.\n"
                         "To prevent the backup of entire boot volumes,"
                         " which is inefficient and dangerous,\n"
                         "we ask you to start with a smaller folder"
                         " and add more data while MEGAsync is running.");
        }
        else
        {
            message = tr("You are trying to sync an extremely large folder.\n"
                         "To prevent the syncing of entire boot volumes,"
                         " which is inefficient and dangerous,\n"
                         "we ask you to start with a smaller folder"
                         " and add more data while MEGAsync is running.");
        }
    }
    return message;
}

SyncController::Syncability SyncController::isLocalPathAllowedForSync(const QString& path, const MegaSync::SyncType &syncType, QString& message)
{
    message = getIsLocalPathAllowedForSyncMsg(path, syncType);
    return (message.isEmpty() ? Syncability::CAN_SYNC : Syncability::CANT_SYNC);
}

QString SyncController::getAreLocalFolderAccessRightsOkMsg(const QString& path, const mega::MegaSync::SyncType& syncType)
{
    QString message;

    // We only check rw rights for two-way syncs
    if (syncType == MegaSync::TYPE_TWOWAY)
    {
        QTemporaryFile test (path + QDir::separator());
        if (!test.open())
        {
            message = tr("You don't have write permissions in this local folder.")
                    + QChar::fromLatin1('\n')
                    + tr("MEGAsync won't be able to download anything here.");
        }
    }
    return message;
}

SyncController::Syncability SyncController::areLocalFolderAccessRightsOk(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message)
{
    message = getAreLocalFolderAccessRightsOkMsg(path, syncType);
    return (message.isEmpty() ? Syncability::CAN_SYNC : Syncability::WARN_SYNC);
}

// Returns wether the path is syncable.
// The message to display to the user is stored in <message>.
// The first error encountered is returned.
// Errors trump warnings
SyncController::Syncability SyncController::isLocalFolderSyncable(const QString& path, const mega::MegaSync::SyncType& syncType, QString& message)
{
    Syncability syncability (Syncability::CAN_SYNC);

    // First check if the path is allowed
    syncability = isLocalPathAllowedForSync(path, syncType, message);

    // Then check that we have rw rights for this path
    if (syncability != Syncability::CANT_SYNC)
    {
        syncability = areLocalFolderAccessRightsOk(path, syncType, message);
    }

    // The check if it is not synced already
    if (syncability != Syncability::CANT_SYNC)
    {
        syncability = isLocalFolderAlreadySynced(path, syncType, message);
    }

    return (syncability);
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

void SyncController::ensureDeviceNameIsSetOnRemote()
{
    if (!mIsDeviceNameSetOnRemote && !mForceSetDeviceName)
    {
        mForceSetDeviceName = true;
        mApi->getDeviceName(mDelegateListener);
    }
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

void SyncController::setMyBackupsDirName()
{
    QString name = tr(SyncController::DEFAULT_BACKUPS_ROOT_DIRNAME);
    mApi->setMyBackupsFolder(name.toUtf8().constData(), mDelegateListener);
}

void SyncController::getMyBackupsHandle()
{
    if(mMyBackupsHandle == INVALID_HANDLE)
        mApi->getUserAttribute(MegaApi::USER_ATTR_MY_BACKUPS_FOLDER, mDelegateListener);
    else
        emit myBackupsHandle(mMyBackupsHandle);
}

void SyncController::setMyBackupsHandle(MegaHandle handle)
{
    mMyBackupsHandle = handle;
    emit myBackupsHandle(mMyBackupsHandle);
}

// For now the path looks like "/My backups", without the "/Backups" root
QString SyncController::getMyBackupsLocalizedPath()
{
    QString backupsDirPath = QString::fromLatin1("/");

    if (mMyBackupsHandle != INVALID_HANDLE)
    {
        // If the node exists, it's very easy: get the path from there
        auto backupsRootNode = std::unique_ptr<MegaNode> (mApi->getNodeByHandle(mMyBackupsHandle));
        backupsDirPath += QString::fromUtf8(backupsRootNode->getName());
    }
    else
    {
        backupsDirPath += tr(SyncController::DEFAULT_BACKUPS_ROOT_DIRNAME);
    }

    qDebug() << QString::fromUtf8("SyncController: Backups root dir: \"%1\"").arg(backupsDirPath);

    return backupsDirPath;
}

void SyncController::onRequestFinish(MegaApi *api, MegaRequest *req, MegaError *e)
{
    int errorCode (e->getErrorCode());

    switch(req->getType())
    {
    case MegaRequest::TYPE_ADD_SYNC:
    {
        int syncErrorCode (req->getNumDetails());
        QString errorMsg;

        bool error = false;

        if (syncErrorCode != MegaSync::NO_SYNC_ERROR)
        {
            errorMsg = QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(syncErrorCode));
            error = true;
        }
        else if (errorCode != MegaError::API_OK)
        {
            errorMsg = getSyncAPIErrorMsg(errorCode);
            if(errorMsg.isEmpty())
                errorMsg = QCoreApplication::translate("MegaError", e->getErrorString());
            error = true;
        }

        if(error)
        {
            std::shared_ptr<MegaNode> remoteNode(api->getNodeByHandle(req->getNodeHandle()));
            QString logMsg = QString::fromUtf8("Error adding sync \"%1\" for \"%2\" to \"%3\" (request error): %4").arg(
                             QString::fromUtf8(req->getName()),
                             QString::fromUtf8(req->getFile()),
                             QString::fromUtf8(api->getNodePath(remoteNode.get())),
                             errorMsg);
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
        }
        emit syncAddStatus(errorCode, errorMsg, QString::fromUtf8(req->getFile()));
        break;
    }
    case MegaRequest::TYPE_REMOVE_SYNC:
    {
        if (errorCode != MegaError::API_OK)
        {
            QString errorMsg = getSyncAPIErrorMsg(errorCode);
            if(errorMsg.isEmpty())
                errorMsg = QCoreApplication::translate("MegaError", e->getErrorString());

            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error removing sync (request error): %1")
                         .arg(errorMsg).toUtf8().constData());
            std::shared_ptr<SyncSetting> sync = mSyncModel->getSyncSettingByTag(req->getParentHandle());
            if(sync)
                emit syncRemoveError(sync);
        }
        break;
    }
    case MegaRequest::TYPE_DISABLE_SYNC:
    {
        if (errorCode == MegaError::API_OK)
            break;

        int syncErrorCode (req->getNumDetails());
        std::shared_ptr<SyncSetting> sync = mSyncModel->getSyncSettingByTag(req->getParentHandle());

        if (sync && (syncErrorCode != MegaSync::NO_SYNC_ERROR))
        {
            QString errorMsg = QString::fromUtf8("Error disabling sync \"%1\" for \"%2\" to \"%3\": %4").arg(
                        sync->name(),
                        sync->getLocalFolder(),
                        sync->getMegaFolder(),
                        QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(syncErrorCode)));

            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, errorMsg.toUtf8().constData());
            emit syncDisableError(sync);
        }
        else
        {
            QString errorMsg = getSyncAPIErrorMsg(errorCode);
            if(errorMsg.isEmpty())
                errorMsg = QCoreApplication::translate("MegaError", e->getErrorString());
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error disabling sync (request error): %1")
                         .arg(errorMsg).toUtf8().constData());
        }
        break;
    }
    case MegaRequest::TYPE_ENABLE_SYNC:
    {
        if (errorCode == MegaError::API_OK)
            break;

        int syncErrorCode (req->getNumDetails());
        std::shared_ptr<SyncSetting> sync = mSyncModel->getSyncSettingByTag(req->getParentHandle());

        if (sync && (syncErrorCode != MegaSync::NO_SYNC_ERROR))
        {
            QString errorMsg = QString::fromUtf8("Error enabling sync \"%1\" for \"%2\" to \"%3\": %4").arg(
                        sync->name(),
                        sync->getLocalFolder(),
                        sync->getMegaFolder(),
                        QCoreApplication::translate("MegaSyncError", MegaSync::getMegaSyncErrorCode(syncErrorCode)));

            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, errorMsg.toUtf8().constData());
            emit syncEnableError(sync);
        }
        else
        {
            QString errorMsg = getSyncAPIErrorMsg(errorCode);
            if(errorMsg.isEmpty())
                errorMsg = QCoreApplication::translate("MegaError", e->getErrorString());
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error enabling sync (request reason): %1")
                         .arg(errorMsg).toUtf8().constData());
        }

        // TODO: Evaluate if I'm needed
        if (!req->getNumDetails() && sync)
        {
            mSyncModel->removeUnattendedDisabledSync(sync->backupId(), sync->getType());
        }
        break;
    }
    case MegaRequest::TYPE_MOVE:
    {
        if (e->getErrorCode() != MegaError::API_OK)
        {
            QString errorMsg = getSyncAPIErrorMsg(errorCode);
            if(errorMsg.isEmpty())
                errorMsg = QCoreApplication::translate("MegaError", e->getErrorString());
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error trashing MEGA folder (request error): %1")
                         .arg(errorMsg).toUtf8().constData());
        }
        break;
    }
    case MegaRequest::TYPE_SET_MY_BACKUPS:
    {
        QString errorMsg;
        if (errorCode == MegaError::API_OK)
        {
            setMyBackupsHandle(req->getNodeHandle());
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, "MyBackups folder set successfully");
        }
        else
        {
            errorMsg = QString::fromUtf8(e->getErrorString());
            QString logMsg (QString::fromUtf8("Error setting MyBackups folder: \"%1\"").arg(errorMsg));
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
            errorMsg = QCoreApplication::translate("MegaError", errorMsg.toUtf8().constData());
        }
        emit setMyBackupsStatus(errorCode, errorMsg);
        break;
    }
    case MegaRequest::TYPE_SET_ATTR_USER:
    {
        QString errorMsg;

        int subCommand (req->getParamType());
        if (subCommand == MegaApi::USER_ATTR_DEVICE_NAMES)
        {
            if (errorCode == MegaError::API_OK)
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Device name set successfully on remote");
                mIsDeviceNameSetOnRemote = true;
                emit deviceName(QString::fromUtf8(req->getName()));
            }
            else //if (errorCode == MegaError::API_ENOENT)
            {
                errorMsg = QString::fromUtf8(e->getErrorString());
                QString logMsg (QString::fromUtf8("Error setting device folder: \"%1\"").arg(errorMsg));
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
                errorMsg = QCoreApplication::translate("MegaError", errorMsg.toUtf8().constData());
            }
            emit setDeviceDirStatus(errorCode, errorMsg);
        }
        break;
    }
    case MegaRequest::TYPE_GET_ATTR_USER:
    {
        int subCommand (req->getParamType());
        if (subCommand == MegaApi::USER_ATTR_MY_BACKUPS_FOLDER)
        {
            MegaHandle handle = INVALID_HANDLE;
            if (errorCode == MegaError::API_OK)
            {
                handle = req->getNodeHandle();
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Got MyBackups folder from remote");
            }
            else
            {
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                             QString::fromUtf8("Error getting MyBackups folder: \"%1\"")
                             .arg(QString::fromUtf8(e->getErrorString()))
                             .toUtf8().constData());
            }
            setMyBackupsHandle(handle);
        }
        else if (subCommand == MegaApi::USER_ATTR_DEVICE_NAMES)
        {
            QString devName;
            if (errorCode == MegaError::API_OK)
            {
                mIsDeviceNameSetOnRemote = true;
                devName = QString::fromUtf8(req->getName());
                MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                             QString::fromUtf8("Got device name from remote: \"%1\"").arg(devName)
                             .toUtf8().constData());
                emit deviceName(devName);
            }
            else //if (errorCode == MegaError::API_ENOENT)
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                             QString::fromUtf8("Error getting device name: %1")
                             .arg(QString::fromUtf8(e->getErrorString()))
                             .toUtf8().constData());
                // If we still don't have one, get it from the Platform
                devName = Platform::getDeviceName();
                MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                             QString::fromUtf8("Got device name from platform: \"%1\"").arg(devName)
                             .toUtf8().constData());

                // If nothing, use generic one.
                if (devName.isNull())
                {
                    devName = tr("Your computer");
                    MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                                 QString::fromUtf8("Using dummy device name: \"%1\"").arg(devName)
                                 .toUtf8().constData());
                }

                if (mForceSetDeviceName)
                {
                    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Force setting device name on remote");
                    mForceSetDeviceName = false;
                    setDeviceName(devName);
                }
            }
        }
        break;
    }
    }
}
