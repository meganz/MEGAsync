#include "SyncController.h"
#include "MegaApplication.h"
#include "Platform.h"

using namespace mega;

void SyncController::addSync(const QString &localFolder, const MegaHandle &remoteHandle,
                             const QString& syncName, mega::MegaSync::SyncType type)
{
    assert(mApi);

    if (localFolder.isEmpty() && syncName.isEmpty())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Adding invalid sync %1")
                     .arg(localFolder).toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Adding sync %1 for path %2")
                 .arg(syncName, localFolder).toUtf8().constData());

    mApi->syncFolder(type, localFolder.toUtf8().constData(),
                     syncName.toUtf8().constData(), remoteHandle, nullptr, mDelegateListener);
}

void SyncController::removeSync(std::shared_ptr<SyncSetting> syncSetting)
{
    assert(mApi);

    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Removing invalid sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Removing sync %1")
                 .arg(syncSetting->name())
                 .toUtf8().constData());

    mApi->removeSync(syncSetting->backupId());
}

void SyncController::enableSync(std::shared_ptr<SyncSetting> syncSetting)
{
    assert(mApi);

    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Trying to enable null sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Enabling sync %1 to %2")
                 .arg(syncSetting->getLocalFolder(), syncSetting->getMegaFolder())
                 .toUtf8().constData() );

    mApi->enableSync(syncSetting->backupId(), mDelegateListener);
}

void SyncController::disableSync(std::shared_ptr<SyncSetting> syncSetting)
{
    assert(mApi);
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Trying to disable invalid sync").toUtf8().constData());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Disabling sync %1 to %2")
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
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error creating MyBackups folder: empty name").toUtf8().constData());
        // Name empty, report error
        emit setMyBackupsDirStatus(errorCode, errorMsg);
    }
    else
    {
        assert(mApi);
        // Check for name collision
        auto  rootNode (MegaSyncApp->getRootNode());
        std::unique_ptr<mega::MegaNode> backupsDirNode (mApi->getChildNode(rootNode.get(), name.toUtf8().constData()));

        if (backupsDirNode)
        {
            int errorCode (MegaError::API_EEXIST);
            QString errorMsg (QCoreApplication::translate("MegaError", MegaError::getErrorString(errorCode)));
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error creating MyBackups folder: folder %1 exists")
                         .arg(name).toUtf8().constData());
            // Folder exists, report error
            emit setMyBackupsDirStatus(errorCode, errorMsg);
        }
        else
        {
            // Create folder
            mApi->createFolder(name.toUtf8().constData(), rootNode.get(), mDelegateListener);
        }
    }
}

void SyncController::setMyBackupsDir(mega::MegaHandle handle)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Setting MyBackups dir to handle %1")
                 .arg(handle).toUtf8().constData());
    assert(Preferences::instance()->logged() && mApi);
    mApi->setMyBackupsFolder(handle, mDelegateListener);
}

void SyncController::setDeviceName(const QString& name)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Setting device name to %1 on remote")
                 .arg(name).toUtf8().constData());
    if (!mIsDeviceNameSetOnRemote)
    {
        assert(Preferences::instance()->logged() && mApi);
        mApi->setDeviceName(name.toUtf8().constData(), mDelegateListener);
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Name already set on remote").toUtf8().constData());
        emit setDeviceDirStatus(MegaError::API_OK, QString());
    }
}

void SyncController::getDeviceName()
{
    assert(Preferences::instance()->logged() && mApi);
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
            errorMsg = QCoreApplication::translate("MegaError", e->getErrorString());
            QString logMsg = QString::fromUtf8("Error adding sync %1 for %2 to %3 (request error): %4")
                    .arg(QString::fromUtf8(req->getName()),
                         QString::fromUtf8(req->getFile()),
                         QString::fromUtf8(api->getNodePath(remoteNode.get())),
                         errorMsg);
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
        }
        emit syncAddStatus(errorCode, errorMsg);
        break;
    }
    case MegaRequest::TYPE_REMOVE_SYNC:
    {
        std::shared_ptr<SyncSetting> sync = mSyncModel->getSyncSettingByTag(req->getParentHandle());
        if (req->getNumDetails() && sync)
        {
            QString errorMsg = QString::fromUtf8("Error removing sync %1 for %2 to %3: %4").arg(
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
            QString errorMsg = QString::fromUtf8("Error disabling sync %1 for %2 to %3: %4").arg(
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
            QString errorMsg = QString::fromUtf8("Error enabling sync %1 for %2 to %3: %4").arg(
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
            }
            else
            {
                errorMsg = QCoreApplication::translate("MegaError", e->getErrorString());
                QString logMsg (QString::fromUtf8("Error setting MyBackups folder: %1").arg(errorMsg));
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
            }
            emit setMyBackupsDirStatus(errorCode, errorMsg);
        }
        else if (subCommand == MegaApi::USER_ATTR_DEVICE_NAMES)
        {
            if (errorCode == MegaError::API_OK)
            {
                mIsDeviceNameSetOnRemote = true;
                emit deviceName(QString::fromUtf8(req->getName()));
            }
            else //if (errorCode == MegaError::API_ENOENT)
            {
                errorMsg = QCoreApplication::translate("MegaError", e->getErrorString());
                QString logMsg (QString::fromUtf8("Error setting device folder: %1").arg(errorMsg));
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
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
            errorMsg = QCoreApplication::translate("MegaError", e->getErrorString());
            QString logMsg (QString::fromUtf8("Error creating MyBackups folder: %1").arg(errorMsg));
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, logMsg.toUtf8().constData());
            emit setMyBackupsDirStatus(errorCode, errorMsg);
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
                // Set the node handle
                setBackupsRootDirHandle(req->getNodeHandle());
            }
            else
            {
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error getting MyBackups folder: %1")
                             .arg(QCoreApplication::translate("MegaError", e->getErrorString()))
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
            }
            else //if (errorCode == MegaError::API_ENOENT)
            {
                // If we still don't have one, get it from the Platform
                devName = Platform::getDeviceName();
                // If nothing, use generic one.
                if (devName.isNull())
                {
                    devName = tr("Your computer");
                }
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error getting device name: %1")
                             .arg(QCoreApplication::translate("MegaError", e->getErrorString()))
                             .toUtf8().constData());
            }
            if (mForceSetDeviceName)
            {
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

SyncController::SyncController(QObject* parent)
    : QObject(parent),
      mApi(nullptr),
      mDelegateListener (new QTMegaRequestListener(mApi, this)),
      mSyncModel(nullptr),
      mIsDeviceNameSetOnRemote(false),
      mForceSetDeviceName(false)
{
}

SyncController& SyncController::instance()
{
    static SyncController instance;
    return instance;
}

void SyncController::setApi(MegaApi* api)
{
    mApi = api;
}

void SyncController::setModel(SyncModel* model)
{
    mSyncModel = model;
}
