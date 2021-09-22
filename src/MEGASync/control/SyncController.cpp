#include "SyncController.h"
#include "MegaApplication.h"

using namespace mega;

void SyncController::addSync(const QString &localFolder, const MegaHandle &remoteHandle, QString syncName)
{
    assert(mApi);

    if (localFolder.isEmpty() && syncName.isEmpty())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Adding invalid sync %1").arg(localFolder).toStdString().c_str());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Adding sync %1 for path %2")
                 .arg(syncName)
                 .arg(localFolder)
                 .toStdString().c_str());

    mApi->syncFolder(MegaSync::TYPE_TWOWAY, localFolder.toStdString().c_str(),
                     syncName.toStdString().c_str(), remoteHandle, nullptr, mDelegateListener);
}

void SyncController::removeSync(std::shared_ptr<SyncSetting> syncSetting)
{
    assert(mApi);

    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Removing invalid sync").toStdString().c_str());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Removing sync %1")
                 .arg(syncSetting->name())
                 .toStdString().c_str());

    mApi->removeSync(syncSetting->backupId());
}

void SyncController::enableSync(std::shared_ptr<SyncSetting> syncSetting)
{
    assert(mApi);

    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Trying to enable null sync").toStdString().c_str());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Enabling sync %1 to %2")
                 .arg(syncSetting->getLocalFolder())
                 .arg(syncSetting->getMegaFolder())
                 .toStdString().c_str() );

    mApi->enableSync(syncSetting->backupId(), mDelegateListener);
}

void SyncController::disableSync(std::shared_ptr<SyncSetting> syncSetting)
{
    assert(mApi);
    if (!syncSetting)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                     QString::fromUtf8("Trying to disable invalid sync").toStdString().c_str());
        return;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Disabling sync %1 to %2")
                 .arg(syncSetting->getLocalFolder())
                 .arg(syncSetting->getMegaFolder())
                 .toStdString().c_str() );

    mApi->disableSync(syncSetting->backupId(), mDelegateListener);
}

void SyncController::onRequestFinish(MegaApi *api, MegaRequest *req, MegaError *e)
{
    Q_UNUSED (api)

    if (e->getErrorCode() == MegaError::API_OK)
    {
        // We are not interested in emitting anything for success for now;
        // could be implemented more easily by subclassing from QTMegaListener instead?
        return;
    }

    switch(req->getType())
    {
    case MegaRequest::TYPE_ADD_SYNC:
    {
        std::shared_ptr<MegaNode> remoteNode(api->getNodeByHandle(req->getNodeHandle()));

        QString errorMsg = QString::fromUtf8("Error adding sync %1 for %2 to %3 (request error): %4")
                .arg(QLatin1String(req->getName()))
                .arg(QLatin1String(req->getFile()))
                .arg(QLatin1String(api->getNodePath(remoteNode.get())))
                .arg(QCoreApplication::translate("MegaError", e->getErrorString()));

        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, errorMsg.toStdString().c_str());
        emit syncAddError(errorMsg);
        break;
    }
    case MegaRequest::TYPE_REMOVE_SYNC:
    {
        std::shared_ptr<SyncSetting> sync = mSyncModel->getSyncSettingByTag(req->getParentHandle());
        if (req->getNumDetails() && sync)
        {
            QString errorMsg = QString::fromUtf8("Error removing sync %1 for %2 to %3: %4")
                         .arg(sync->name())
                         .arg(sync->getLocalFolder())
                         .arg(sync->getMegaFolder())
                         .arg(QCoreApplication::translate("MegaError", MegaSync::getMegaSyncErrorCode(req->getNumDetails())));

            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, errorMsg.toStdString().c_str());
            emit syncRemoveError(sync);
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error removing sync (request error): %1")
                         .arg(QCoreApplication::translate("MegaError", e->getErrorString()))
                         .toStdString().c_str());
        }
        break;
    }
    case MegaRequest::TYPE_DISABLE_SYNC:
    {
        std::shared_ptr<SyncSetting> sync = mSyncModel->getSyncSettingByTag(req->getParentHandle());
        if (req->getNumDetails() && sync)
        {
            QString errorMsg = QString::fromUtf8("Error disabling sync %1 for %2 to %3: %4")
                         .arg(sync->name())
                         .arg(sync->getLocalFolder())
                         .arg(sync->getMegaFolder())
                         .arg(QCoreApplication::translate("MegaError", MegaSync::getMegaSyncErrorCode(req->getNumDetails())));

            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, errorMsg.toStdString().c_str());
            emit syncDisableError(sync);
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error disabling sync (request error): %1")
                         .arg(QCoreApplication::translate("MegaError", e->getErrorString()))
                         .toStdString().c_str());
        }
        break;
    }
    case MegaRequest::TYPE_ENABLE_SYNC:
    {
        std::shared_ptr<SyncSetting> sync = mSyncModel->getSyncSettingByTag(req->getParentHandle());
        if (req->getNumDetails() && sync)
        {
            QString errorMsg = QString::fromUtf8("Error enabling sync %1 for %2 to %3: %4")
                         .arg(sync->name())
                         .arg(sync->getLocalFolder())
                         .arg(sync->getMegaFolder())
                         .arg(QCoreApplication::translate("MegaError", MegaSync::getMegaSyncErrorCode(req->getNumDetails())));

            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, errorMsg.toStdString().c_str());
            emit syncEnableError(sync);
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error enabling sync (request reason): %1")
                         .arg(QCoreApplication::translate("MegaError", e->getErrorString()))
                         .toStdString().c_str());
        }
        break;
    }
    }
}

SyncController::SyncController(QObject *parent)
    : QObject(parent),
      mApi(nullptr),
      mDelegateListener (new QTMegaRequestListener(mApi, this)),
      mSyncModel(nullptr)
{
}

SyncController& SyncController::instance()
{
    static SyncController instance;
    return instance;
}

void SyncController::setApi(MegaApi *value)
{
    mApi = value;
}

void SyncController::setModel(SyncModel *model)
{
    mSyncModel = model;
}
