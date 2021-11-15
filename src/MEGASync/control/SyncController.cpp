#include "SyncController.h"
#include "MegaApplication.h"
#include "Platform.h"

using namespace mega;

#define MegaSyncApp (static_cast<MegaApplication *>(QCoreApplication::instance()))

SyncController::SyncController(QObject* parent)
    : QObject(parent),
      mApi(MegaSyncApp->getMegaApi()),
      mDelegateListener (new QTMegaRequestListener(mApi, this)),
      mSyncModel(SyncModel::instance()),
      mIsDeviceNameSetOnRemote(false),
      mForceSetDeviceName(false)
{
    // The controller shouldn't ever be instantiated before we have an API and a SyncModel available
    assert(mApi);
    assert(mSyncModel);

    // Make sure the device name is set, and set it if not
    ensureDeviceNameIsSetOnRemote();
}

SyncController::~SyncController()
{
    delete mDelegateListener;
}

void SyncController::addSync(const QString &localFolder, const MegaHandle &remoteHandle,
                             const QString& syncName, mega::MegaSync::SyncType type)
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

void SyncController::removeSync(std::shared_ptr<SyncSetting> syncSetting)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Removing invalid sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Removing sync \"%1\"")
                 .arg(syncSetting->name()).toUtf8().constData());

    mApi->removeSync(syncSetting->backupId());
}

void SyncController::enableSync(std::shared_ptr<SyncSetting> syncSetting)
{
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Trying to enable null sync").toUtf8().constData());
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
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Trying to disable invalid sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Disabling sync \"%1\" to \"%2\"")
                 .arg(syncSetting->getLocalFolder(), syncSetting->getMegaFolder())
                 .toUtf8().constData());

    mApi->disableSync(syncSetting->backupId(), mDelegateListener);
}

void SyncController::createMyBackupsDir(const QString& name)
{
    // Check that name is not empty
    if (name.isEmpty())        
    {
        int errorCode (MegaError::API_EARGS);
        QString errorMsg (QCoreApplication::translate("MegaError", MegaError::getErrorString(errorCode)));
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Error creating MyBackups folder: empty name")
                     .toUtf8().constData());
        // Name empty, report error
        emit setMyBackupsDirStatus(errorCode, errorMsg);
    }
    else
    {
        auto  rootNode (MegaSyncApp->getRootNode());
        std::unique_ptr<mega::MegaNode> backupsDirNode (mApi->getChildNode(rootNode.get(), name.toUtf8().constData()));

        if (backupsDirNode)
        {
            int errorCode (MegaError::API_EEXIST);
            QString errorMsg (QCoreApplication::translate("MegaError", MegaError::getErrorString(errorCode)));
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                         QString::fromUtf8("Error creating MyBackups folder: \"%1\" exists")
                         .arg(name).toUtf8().constData());
            // Folder exists, report error
            emit setMyBackupsDirStatus(errorCode, errorMsg);
        }
        else
        {
            // Create folder
            MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                         QString::fromUtf8("Creating MyBackups folder: \"%1\"")
                         .arg(name).toUtf8().constData());
            mApi->createFolder(name.toUtf8().constData(), rootNode.get(), mDelegateListener);
        }
    }
}

void SyncController::setMyBackupsDir(mega::MegaHandle handle)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Setting MyBackups dir to handle %1")
                 .arg(handle).toUtf8().constData());
    assert(Preferences::instance()->logged());
    mApi->setMyBackupsFolder(handle, mDelegateListener);
}

void SyncController::setDeviceName(const QString& name)
{
    if (!mIsDeviceNameSetOnRemote)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                     QString::fromUtf8("Setting device name to \"%1\" on remote")
                     .arg(name).toUtf8().constData());
        assert(Preferences::instance()->logged());
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
    assert(Preferences::instance()->logged());
    mApi->getDeviceName(mDelegateListener);
}

void SyncController::ensureDeviceNameIsSetOnRemote()
{
    if (!mIsDeviceNameSetOnRemote && !mForceSetDeviceName)
    {
        mForceSetDeviceName = true;
        getDeviceName();
    }
}

void SyncController::setBackupsRootDirHandle(mega::MegaHandle handle)
{
    emit backupsRootDirHandle(handle);
}

void SyncController::getBackupsRootDirHandle()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Request MyBackups folder");
    mApi->getMyBackupsFolder(mDelegateListener);
}

void SyncController::onRequestFinish(MegaApi *api, MegaRequest *req, MegaError *e)
{
    Q_UNUSED (api)

    static MegaHandle tempMyBackupsHandle = mega::INVALID_HANDLE;

    switch(req->getType())
    {
    case MegaRequest::TYPE_ADD_SYNC:
    {
        int errorCode (e->getErrorCode());
        QString errorMsg;

        if (errorCode != MegaError::API_OK)
        {
            std::shared_ptr<MegaNode> remoteNode(api->getNodeByHandle(req->getNodeHandle()));

            QString name (QString::fromUtf8(req->getName()));
            errorMsg = QCoreApplication::translate("MegaError", MegaSync::getMegaSyncErrorCode(req->getNumDetails()));

            QString logMsg = QString::fromUtf8("Error adding sync \"%1\" for \"%2\" to \"%3\" (request error): %4")
                    .arg(name,
                         QString::fromUtf8(req->getFile()),
                         QString::fromUtf8(api->getNodePath(remoteNode.get())),
                         errorMsg);
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());

            errorMsg = tr("Adding backup \"%1\" failed.\n"
                          "Reason: %2")
                       .arg(name, errorMsg);
        }
        emit syncAddStatus(errorCode, errorMsg);
        break;
    }
    case MegaRequest::TYPE_REMOVE_SYNC:
    {
        std::shared_ptr<SyncSetting> sync = mSyncModel->getSyncSettingByTag(req->getParentHandle());
        if (req->getNumDetails() && sync)
        {
            QString errorMsg = QString::fromUtf8("Error removing sync \"%1\" for \"%2\" to \"%3\": %4").arg(
                        sync->name(),
                        sync->getLocalFolder(),
                        sync->getMegaFolder(),
                        QCoreApplication::translate("MegaError", MegaSync::getMegaSyncErrorCode(req->getNumDetails())));

            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, errorMsg.toUtf8().constData());
            emit syncRemoveError(sync);
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error removing sync (request error): %1")
                         .arg(QCoreApplication::translate("MegaError", e->getErrorString()))
                         .toUtf8().constData());
        }
        break;
    }
    case MegaRequest::TYPE_DISABLE_SYNC:
    {
        std::shared_ptr<SyncSetting> sync = mSyncModel->getSyncSettingByTag(req->getParentHandle());
        if (req->getNumDetails() && sync)
        {
            QString errorMsg = QString::fromUtf8("Error disabling sync \"%1\" for \"%2\" to \"%3\": %4").arg(
                        sync->name(),
                        sync->getLocalFolder(),
                        sync->getMegaFolder(),
                        QCoreApplication::translate("MegaError", MegaSync::getMegaSyncErrorCode(req->getNumDetails())));

            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, errorMsg.toUtf8().constData());
            emit syncDisableError(sync);
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error disabling sync (request error): %1")
                         .arg(QCoreApplication::translate("MegaError", e->getErrorString()))
                         .toUtf8().constData());
        }
        break;
    }
    case MegaRequest::TYPE_ENABLE_SYNC:
    {
        std::shared_ptr<SyncSetting> sync = mSyncModel->getSyncSettingByTag(req->getParentHandle());
        if (req->getNumDetails() && sync)
        {
            QString errorMsg = QString::fromUtf8("Error enabling sync \"%1\" for \"%2\" to \"%3\": %4").arg(
                        sync->name(),
                        sync->getLocalFolder(),
                        sync->getMegaFolder(),
                        QCoreApplication::translate("MegaError", MegaSync::getMegaSyncErrorCode(req->getNumDetails())));

            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, errorMsg.toUtf8().constData());
            emit syncEnableError(sync);
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error enabling sync (request reason): %1")
                         .arg(QCoreApplication::translate("MegaError", e->getErrorString()))
                         .toUtf8().constData());
        }
        break;
    }
    case MegaRequest::TYPE_SET_ATTR_USER:
    {
        int subCommand (req->getParamType());
        int errorCode (e->getErrorCode());
        QString errorMsg;

        if (subCommand == MegaApi::USER_ATTR_MY_BACKUPS_FOLDER)
        {
            if (errorCode == MegaError::API_OK)
            {
                // We get the node handle,
                setBackupsRootDirHandle(tempMyBackupsHandle);
                tempMyBackupsHandle = mega::INVALID_HANDLE;
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, "MyBackups folder set successfully");
            }
            else
            {
                errorMsg = QString::fromUtf8(e->getErrorString());
                QString logMsg (QString::fromUtf8("Error setting MyBackups folder: \"%1\"").arg(errorMsg));
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
                errorMsg = QCoreApplication::translate("MegaError", errorMsg.toUtf8().constData());
            }
            emit setMyBackupsDirStatus(errorCode, errorMsg);
        }
        else if (subCommand == MegaApi::USER_ATTR_DEVICE_NAMES)
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
    case MegaRequest::TYPE_CREATE_FOLDER:
    {
        int errorCode (e->getErrorCode());
        QString errorMsg;

        if (errorCode == MegaError::API_OK)
        {
            tempMyBackupsHandle = req->getNodeHandle();
            setMyBackupsDir(tempMyBackupsHandle);
        }
        else
        {
            errorMsg = QString::fromUtf8(e->getErrorString());
            QString logMsg (QString::fromUtf8("Error creating MyBackups folder: \"%1\"").arg(errorMsg));
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
            emit setMyBackupsDirStatus(errorCode, errorMsg);
            errorMsg = QCoreApplication::translate("MegaError", errorMsg.toUtf8().constData());
        }
        break;
    }
    case MegaRequest::TYPE_GET_ATTR_USER:
    {
        int subCommand (req->getParamType());
        int errorCode (e->getErrorCode());

        if (subCommand == MegaApi::USER_ATTR_MY_BACKUPS_FOLDER)
        {
            if (errorCode == MegaError::API_OK)
            {
                MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Got MyBackups dir from remote");
                // Set the node handle
                setBackupsRootDirHandle(req->getNodeHandle());
            }
            else
            {
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                             QString::fromUtf8("Error getting MyBackups folder: \"%1\"")
                             .arg(QString::fromUtf8(e->getErrorString()))
                             .toUtf8().constData());
                setBackupsRootDirHandle(mega::INVALID_HANDLE);
            }
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
            }
            if (mForceSetDeviceName)
            {
                MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Force setting device name on remote");
                mForceSetDeviceName = false;
                setDeviceName(devName);
            }
            else
            {
                emit deviceName(devName);
            }
        }
        break;
    }
    }
}
