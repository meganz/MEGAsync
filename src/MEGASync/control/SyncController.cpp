#include "SyncController.h"
#include "MegaApplication.h"
#include "Platform.h"

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

void SyncController::addBackup(const QString &localFolder)
{
    QDir dirToBackup(localFolder);
    addBackup(dirToBackup);
}

void SyncController::addBackup(const QDir &dirToBackup)
{
    addSync(QDir::toNativeSeparators(dirToBackup.canonicalPath()), mega::INVALID_HANDLE,
            dirToBackup.dirName(), mega::MegaSync::TYPE_BACKUP);
}

void SyncController::addSync(const QString &localFolder, const MegaHandle &remoteHandle,
                             const QString& syncName, MegaSync::SyncType type)
{
    if (localFolder.isEmpty() && syncName.isEmpty())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Adding invalid sync \"%1\"")
                     .arg(localFolder).toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Adding sync \"%1\" for path \"%2\"")
                 .arg(syncName, localFolder).toUtf8().constData());

    mApi->syncFolder(type, localFolder.toUtf8().constData(),
                     syncName.toUtf8().constData(), remoteHandle, nullptr, mDelegateListener);
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

    std::shared_ptr<MegaNode> node(mApi->getNodeByHandle(syncSetting->getMegaHandle()));

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
QString SyncController::getIsFolderAlreadySyncedMsg(const QString& path,const MegaSync::SyncType& syncType)
{
    auto cleanInputPath (QDir::cleanPath(QDir(path).canonicalPath()));
    QString message;

    // Gather all synced or backed-up dirs
    QMap<QString, MegaSync::SyncType> localFolders = SyncModel::instance()->getLocalFoldersAndTypeMap();

    // First check existing syncs
    foreach(auto& lf, localFolders.keys())
    {
        QString c = QDir::cleanPath(lf);
        if(cleanInputPath == c)
        {
            if(syncType == MegaSync::SyncType::TYPE_BACKUP)
            {
                message = localFolders.value(lf) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't sync this folder as it's already synced.") : tr("Folder is already backed up. Select a different one.");
            }
            else
            {
                message = localFolders.value(lf) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't sync this folder as it's already synced.") : tr("You can't sync this folder as it's backed up.");
            }
        }
        else if (cleanInputPath.startsWith(c))
        {
            if(syncType == MegaSync::SyncType::TYPE_BACKUP)
            {
                message = localFolders.value(lf) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't backup this folder as it's already inside a synced folder.") : tr("You can't backup this folder as it's already inside a backed up folder.");
            }
            else
            {
                message = localFolders.value(lf) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't sync folders that are inside synced folders.") : tr("You can't sync folders that are inside backed up folders. ");
            }
        }
        else if (c.startsWith(cleanInputPath))
        {
            if(syncType == MegaSync::SyncType::TYPE_BACKUP)
            {
                message = localFolders.value(lf) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't backup this folder as it contains synced folders.") : tr("You can't backup this folder as it contains backed up folders.");
            }
            else
            {
                message = localFolders.value(lf) == MegaSync::SyncType::TYPE_TWOWAY ?
                            tr("You can't sync folders that contain synced folders.") : tr("You can't sync folders that contain backed up folders.  ");
            }
        }
    }

    return message;
}

bool SyncController::isFolderAlreadySynced(const QString &path, const MegaSync::SyncType &syncType, QString& message)
{
    message = getIsFolderAlreadySyncedMsg(path, syncType);
    return (!message.isEmpty());
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
        // FIXME: The following 5 strings need to be validated/reworded
        case MegaError::API_EARGS:
            return tr("Local folder not set");
        case MegaError::API_EACCESS:
            return tr("Error with the folder set as root for the backups");
        case MegaError::API_EINTERNAL:
            return tr("The user attribute for the backup root folder does not have a record containing the handle");
        case MegaError::API_ENOENT:
            return tr("The handle of the backup root folder stored in the user attribute was invalid, or the node could not be found.");
        case MegaError::API_EINCOMPLETE:
            return tr("Device id not set or invalid device name");
        default:
            return QString();
    }
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
        int errorCode (e->getErrorCode());
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
        int errorCode (e->getErrorCode());
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
        int errorCode (e->getErrorCode());

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
