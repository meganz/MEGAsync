#include "Syncs.h"

#include <memory>

#include "mega/types.h"
#include "MegaApplication.h"
#include "TextDecorator.h"
#include "QMegaMessageBox.h"

Syncs::Syncs(QObject *parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
    , mSyncController(mega::make_unique<SyncController>(new SyncController()))
    , remoteFolder()
    , localFolder()
    , mCreatingFolder(false)
{
    mMegaApi->addRequestListener(mDelegateListener.get());
    connect(mSyncController.get(), &SyncController::syncAddStatus,
            this, &Syncs::onSyncAddRequestStatus);
}

void Syncs::addSync(const QString& local, const QString& remote)
{
    if (errorOnSyncPaths(local, remote))
    {
        return;
    }

    auto remoteHandle = mega::INVALID_HANDLE;
    auto megaNode = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(remote.toStdString().c_str()));
    if (megaNode != nullptr)
    {
        remoteHandle = megaNode->getHandle();
    }

    if (remoteHandle == mega::INVALID_HANDLE)
    {
        mCreatingFolder = true;
        remoteFolder = remote;
        localFolder = local;
        mMegaApi->createFolder(remote.toStdString().c_str(), MegaSyncApp->getRootNode().get());
    }
    else
    {
        mSyncController->addSync(local, remoteHandle);
    }
}

bool Syncs::errorOnSyncPaths(const QString &localPath, const QString &remotePath)
{
    bool error = false;

    QString errorMessage;
    if (!helperCheckLocalSync(localPath, errorMessage))
    {
        error = true;
        emit cantSync(errorMessage, true);
    }

    if (!helperCheckRemoteSync(remotePath, errorMessage))
    {
        error = true;
        emit cantSync(errorMessage, false);
    }

    return error;
}

bool Syncs::helperCheckLocalSync(const QString& path, QString& errorMessage) const
{
    if (path.isEmpty())
    {
        errorMessage = QLatin1String("local path is empty");
        return false;
    }

    auto localFolderPath = QDir::toNativeSeparators(path);
    QDir openFromFolderDir(localFolderPath);
    if (!openFromFolderDir.exists())
    {
        return true;
    }

    auto syncability = SyncController::isLocalFolderSyncable(path, mega::MegaSync::TYPE_TWOWAY, errorMessage);

    if (syncability == SyncController::WARN_SYNC)
    {
        // Only local WARN_SYNC at this point
        //local warning write permission needs to be different for this case
        //on onboarding so we will make up it here
        errorMessage = tr("Folder can't be synced as you don't have write permissions.");
    }

#if defined DEBUG
    qDebug() << "localPath : " << path << " syncability : " << syncability << " message : " << errorMessage;
#endif

    return (syncability != SyncController::CANT_SYNC);
}

bool Syncs::helperCheckRemoteSync(const QString& path, QString& errorMessage) const
{
    if (path.isEmpty())
    {
        errorMessage = QLatin1String("remote path is empty");
        return false;
    }

    QString message;
    SyncController::Syncability syncability = SyncController::Syncability::CAN_SYNC;
    auto megaNode = std::shared_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(path.toStdString().c_str()));
    if (megaNode)
    {
        syncability = SyncController::isRemoteFolderSyncable(megaNode, message);
    }

#if defined DEBUG
    qDebug() << "remotePath : " << path << " syncability : " << syncability << " message : " << message;
#endif

    return (syncability != SyncController::CANT_SYNC);
}

bool Syncs::checkLocalSync(const QString &path) const
{
    QString error;
    return helperCheckLocalSync(path, error);
}

bool Syncs::checkRemoteSync(const QString &path) const
{
    QString error;
    return helperCheckRemoteSync(path, error);
}

void Syncs::onRequestFinish(mega::MegaApi* api,
                            mega::MegaRequest* request,
                            mega::MegaError* error)
{
    Q_UNUSED(api)

    if (request->getType() == mega::MegaRequest::TYPE_CREATE_FOLDER
            && mCreatingFolder && remoteFolder.compare(QString::fromUtf8(request->getName())))
    {
        mCreatingFolder = false;

        if (error->getErrorCode() == mega::MegaError::API_OK)
        {
            auto megaNode = std::shared_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(remoteFolder.toStdString().c_str()));
            if (megaNode != nullptr)
            {
                mSyncController->addSync(localFolder, request->getNodeHandle());
            }
            else
            {
                emit cantSync(tr("%1 folder doesn't exist").arg(remoteFolder), false);
            }
        }
        else if (error->getErrorCode() != mega::MegaError::API_ESSL
                && error->getErrorCode() != mega::MegaError::API_ESID)
        {
            emit cantSync(QCoreApplication::translate("MegaError", error->getErrorString()), false);
        }
    }
}

void Syncs::onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString errorMsg, QString name)
{
    Q_UNUSED(name)
    Q_UNUSED(syncErrorCode)

    if (errorCode != mega::MegaError::API_OK)
    {
        Text::Link link(Utilities::SUPPORT_URL);
        Text::Decorator dec(&link);
        QString msg = errorMsg;
        dec.process(msg);

        emit cantSync(msg, false);
    }
    else
    {
        emit syncSetupSuccess();
    }
}
