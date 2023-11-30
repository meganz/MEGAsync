#include "Syncs.h"

#include <memory>

#include "mega/types.h"
#include "MegaApplication.h"
#include "TextDecorator.h"

const QString Syncs::DEFAULT_MEGA_FOLDER = QString::fromUtf8("MEGA");
const QString Syncs::DEFAULT_MEGA_PATH = QString::fromUtf8("/") + Syncs::DEFAULT_MEGA_FOLDER;

Syncs::Syncs(QObject *parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(mega::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
    , mSyncController(mega::make_unique<SyncController>())
    , mRemoteFolder()
    , mLocalFolder()
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
        mRemoteFolder = remote;

        /*
         *  need to remove the first / from the remote path,
         *  we already state in createFolder the origin point.
         */
        if (mRemoteFolder.indexOf(QLatin1Char('/')) == 0)
        {
            mRemoteFolder.remove(0,1);
        }

        mLocalFolder = local;
        mMegaApi->createFolder(mRemoteFolder.toStdString().c_str(), MegaSyncApp->getRootNode().get());
    }
    else
    {
        mSyncController->addSync(local, remoteHandle);
    }
}

bool Syncs::errorOnSyncPaths(const QString &localPath, const QString &remotePath)
{
    bool error = false;

    QString localErrorMessage;
    if (!helperCheckLocalSync(localPath, localErrorMessage))
    {
        error = true;
        emit cantSync(localErrorMessage, true);
    }

    QString remoteErrorMessage;
    if (!helperCheckRemoteSync(remotePath, remoteErrorMessage))
    {
        error = true;
        emit cantSync(remoteErrorMessage, false);
    }

    return error;
}

bool Syncs::helperCheckLocalSync(const QString& path, QString& errorMessage) const
{
    if (path.isEmpty())
    {
        errorMessage = tr("Select a local folder to sync.");
        return false;
    }

    auto localFolderPath = QDir::toNativeSeparators(path);
    QDir openFromFolderDir(localFolderPath);
    if (!openFromFolderDir.exists() )
    {
        errorMessage = tr("The local path is unavailable.");
        return false;
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
        errorMessage = tr("Select a MEGA folder to sync.");
        return false;
    }

    SyncController::Syncability syncability = SyncController::Syncability::CAN_SYNC;
    auto megaNode = std::shared_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(path.toStdString().c_str()));
    if (megaNode)
    {
        syncability = SyncController::isRemoteFolderSyncable(megaNode, errorMessage);
    }
    else if(path != Syncs::DEFAULT_MEGA_PATH)
    {
        syncability = SyncController::CANT_SYNC;
        errorMessage = tr("Folder can't be synced as it can't be located. "
                          "It may have been moved or deleted, or you might not have access.");
    }

#if defined DEBUG
    qDebug() << "remotePath : " << path << " syncability : " << syncability << " message : " << errorMessage;
#endif

    return (syncability != SyncController::CANT_SYNC);
}

bool Syncs::checkLocalSync(const QString &path) const
{
    if (path.isEmpty())
    {
        return false;
    }

    auto localFolderPath = QDir::toNativeSeparators(path);
    QDir openFromFolderDir(localFolderPath);
    if (!openFromFolderDir.exists())
    {
        return true;
    }

    QString errorMessage;
    auto syncability = SyncController::isLocalFolderSyncable(path, mega::MegaSync::TYPE_TWOWAY, errorMessage);

    return (syncability != SyncController::CANT_SYNC);
}

bool Syncs::checkRemoteSync(const QString &path) const
{
    QString error;
    return helperCheckRemoteSync(path, error);
}

QString Syncs::getDefaultMegaFolder() const
{
    return DEFAULT_MEGA_FOLDER;
}

QString Syncs::getDefaultMegaPath() const
{
    return DEFAULT_MEGA_PATH;
}

void Syncs::onRequestFinish(mega::MegaApi* api,
                            mega::MegaRequest* request,
                            mega::MegaError* error)
{
    Q_UNUSED(api)

    if (request->getType() == mega::MegaRequest::TYPE_CREATE_FOLDER
            && mCreatingFolder && (mRemoteFolder.compare(QString::fromUtf8(request->getName()))==0))
    {
        mCreatingFolder = false;

        if (error->getErrorCode() == mega::MegaError::API_OK)
        {
            auto megaNode = std::shared_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(mRemoteFolder.toStdString().c_str(), MegaSyncApp->getRootNode().get()));
            if (megaNode != nullptr)
            {
                mSyncController->addSync(mLocalFolder, request->getNodeHandle());
            }
            else
            {
                emit cantSync(tr("%1 folder doesn't exist").arg(mRemoteFolder), false);
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
