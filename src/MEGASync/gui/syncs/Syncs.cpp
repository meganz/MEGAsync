#include "Syncs.h"

#include "ChooseFolder.h"
#include "mega/types.h"
#include "MegaApplication.h"
#include "TextDecorator.h"

const QString Syncs::DEFAULT_MEGA_FOLDER = QString::fromUtf8("MEGA");
const QString Syncs::DEFAULT_MEGA_PATH = QString::fromUtf8("/") + Syncs::DEFAULT_MEGA_FOLDER;

Syncs::Syncs(QObject* parent):
    QObject(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mDelegateListener(
        std::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this)),
    mCreatingFolder(false)
{
    mMegaApi->addRequestListener(mDelegateListener.get());

    connect(&SyncController::instance(), &SyncController::syncAddStatus,
            this, &Syncs::onSyncAddRequestStatus);
    connect(SyncInfo::instance(), &SyncInfo::syncRemoved,
            this, &Syncs::onSyncRemoved);

    onSyncRemoved(nullptr);
}

void Syncs::addSync(SyncInfo::SyncOrigin origin, const QString& local, const QString& remote)
{
    cleanErrorsPrivately();

    if (checkErrorsOnSyncPaths(local, remote))
    {
        return;
    }

    auto remoteHandle = mega::INVALID_HANDLE;
    auto megaNode = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(remote.toUtf8().constData()));
    if (megaNode != nullptr)
    {
        remoteHandle = megaNode->getHandle();
    }

    mSyncConfig.localFolder = local;
    mSyncConfig.origin = origin;
    mSyncConfig.remoteHandle = remoteHandle;
    mRemoteFolder = remote;

    if (remoteHandle == mega::INVALID_HANDLE)
    {
        mCreatingFolder = true;

        /*
         *  need to remove the first / from the remote path,
         *  we already state in createFolder the origin point.
         */
        if (mRemoteFolder.indexOf(QLatin1Char('/')) == 0)
        {
            mRemoteFolder.remove(0,1);
        }

        mMegaApi->createFolder(mRemoteFolder.toUtf8().constData(),
                               MegaSyncApp->getRootNode().get());
    }
    else
    {
        SyncController::instance().addSync(mSyncConfig);
    }
}

bool Syncs::checkErrorsOnSyncPaths(const QString& localPath, const QString& remotePath)
{
    helperCheckLocalSync(localPath);
    helperCheckRemoteSync(remotePath);

    return (!mLocalErrorMessage.isEmpty() || mRemoteError.has_value());
}

void Syncs::helperCheckLocalSync(const QString& path)
{
    std::optional<LocalErrors> localError;

    if (path.isEmpty())
    {
        localError = LocalErrors::EmptyPath;
    }
    else
    {
        auto localFolderPath = QDir::toNativeSeparators(path);
        QDir openFromFolderDir(localFolderPath);
        if (!openFromFolderDir.exists())
        {
            ChooseLocalFolder localFolder;
            if (localFolderPath == localFolder.getDefaultFolder(DEFAULT_MEGA_FOLDER))
            {
                if (!localFolder.createFolder(localFolderPath))
                {
                    localError = LocalErrors::NoAccessPermissionsCantCreate;
                }
            }
            else
            {
                localError = LocalErrors::NoAccessPermissionsNoExist;
            }
        }
    }

    QString errorMessage;
    if (!localError.has_value())
    {
        auto syncability = SyncController::instance().isLocalFolderSyncable(path, mega::MegaSync::TYPE_TWOWAY, errorMessage);
        if (syncability == SyncController::CANT_SYNC)
        {
            localError = LocalErrors::CantSync;
        }
    }
    if (errorMessage.isEmpty())
        errorMessage = getLocalErrorMessage(localError, path);

    if (mLocalErrorMessage != errorMessage)
    {
        mLocalErrorMessage = errorMessage;
        emit localErrorChanged(mLocalErrorMessage);
    }
}

void Syncs::helperCheckRemoteSync(const QString& path)
{
    std::optional<RemoteErrors> remoteError;

    if (path.isEmpty())
    {
        remoteError = RemoteErrors::EmptyPath;
    }
    else
    {
        auto megaNode = std::unique_ptr<mega::MegaNode>(mMegaApi->getNodeByPath(path.toStdString().c_str()));
        if (megaNode)
        {
            std::unique_ptr<mega::MegaError> remoteMegaError(MegaSyncApp->getMegaApi()->isNodeSyncableWithError(megaNode.get()));
            if (remoteMegaError->getErrorCode() != mega::MegaError::API_OK)
            {
                remoteError = RemoteErrors::CantSync;
                mRemoteMegaError.error = remoteMegaError->getErrorCode();
                mRemoteMegaError.syncError = remoteMegaError->getSyncError();
            }
        }
        else if (path != Syncs::DEFAULT_MEGA_PATH)
        {
            remoteError = RemoteErrors::CantSync;
        }
    }

    if (mRemoteError != remoteError)
    {
        mRemoteError.swap(remoteError);
        emit remoteErrorChanged(getRemoteError());
    }
}

bool Syncs::checkLocalSync(const QString& path)
{
    helperCheckLocalSync(path);
    return (mLocalErrorMessage.isEmpty());
}

bool Syncs::checkRemoteSync(const QString& path)
{
    helperCheckRemoteSync(path);
    return (!mRemoteError.has_value());
}

QString Syncs::getDefaultMegaFolder() const
{
    return DEFAULT_MEGA_FOLDER;
}

QString Syncs::getDefaultMegaPath() const
{
    return DEFAULT_MEGA_PATH;
}

Syncs::SyncStatusCode Syncs::getSyncStatus() const
{
    return mSyncStatus;
}

void Syncs::setSyncStatus(SyncStatusCode status)
{
    if(status != mSyncStatus)
    {
        mSyncStatus = status;
        emit syncStatusChanged();
    }
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
            auto megaNode = std::shared_ptr<mega::MegaNode>(
                mMegaApi->getNodeByPath(mRemoteFolder.toUtf8().constData(),
                                        MegaSyncApp->getRootNode().get()));
            if (megaNode != nullptr)
            {
                mSyncConfig.remoteHandle = request->getNodeHandle();
                SyncController::instance().addSync(mSyncConfig);
            }
            else
            {
                mRemoteError = RemoteErrors::CantCreateRemoteFolder;
                emit remoteErrorChanged(getRemoteError());
            }
        }
        else if (error->getErrorCode() != mega::MegaError::API_ESSL
                && error->getErrorCode() != mega::MegaError::API_ESID)
        {
            mRemoteError = RemoteErrors::CantCreateRemoteFolderMsg;
            mRemoteMegaError.error = mega::MegaError::API_OK;
            mRemoteMegaError.syncError = mega::SyncError::NO_SYNC_ERROR;
            mRemoteStringMessage = QString::fromUtf8(error->getErrorString());

            emit remoteErrorChanged(getRemoteError());
        }
    }
}

void Syncs::onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings)
{
    Q_UNUSED(syncSettings)

    SyncInfo* syncInfo = SyncInfo::instance();
    if(syncInfo->getNumSyncedFolders(mega::MegaSync::SyncType::TYPE_TWOWAY) <= 0)
    {
        setSyncStatus(NONE);
    }
    else
    {
        setSyncStatus(FULL);
    }
    emit syncRemoved();
}

void Syncs::onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString name)
{
    Q_UNUSED(name)

    if (errorCode != mega::MegaError::API_OK)
    {
        mRemoteError = RemoteErrors::CantAddSync;
        mRemoteMegaError.error = errorCode;
        mRemoteMegaError.syncError = syncErrorCode;

        emit remoteErrorChanged(getRemoteError());
    }
    else
    {
        emit syncSetupSuccess();
    }
}

QString Syncs::getLocalError() const
{
    return mLocalErrorMessage;
}

QString Syncs::getRemoteError() const
{
    if (!mRemoteError.has_value())
    {
        return {};
    }

    switch (mRemoteError.value())
    {
        case RemoteErrors::EmptyPath:
        {
            return tr("Select a MEGA folder to sync.");
        }

        case RemoteErrors::CantSync:
        {
            if (mRemoteMegaError.error != mega::MegaError::API_OK)
            {
                return SyncController::instance().getRemoteFolderErrorMessage(mRemoteMegaError.error, mRemoteMegaError.syncError);
            }
            else
            {
                return tr("Folder can't be synced as it can't be located. "
                          "It may have been moved or deleted, or you might not have access.");
            }
        }

        case RemoteErrors::CantCreateRemoteFolder:
        {
            return tr("%1 folder doesn't exist").arg(mRemoteFolder);
        }

        case RemoteErrors::CantCreateRemoteFolderMsg:
        {
            if (!mRemoteStringMessage.isEmpty())
            {
                return QCoreApplication::translate("MegaError", mRemoteStringMessage.toStdString().c_str());
            }

            break;
        }

        case RemoteErrors::CantAddSync:
        {
            Text::Link link(Utilities::SUPPORT_URL);
            Text::Decorator dec(&link);
            QString msg = SyncController::instance().getErrorString(mRemoteMegaError.error, mRemoteMegaError.syncError);
            dec.process(msg);

            return msg;
        }
    }

    return {};
}

void Syncs::cleanErrorsPrivately()
{
    mLocalErrorMessage.clear();
    mRemoteError.reset();
    mRemoteStringMessage.clear();
    mRemoteMegaError.error = mega::MegaError::API_OK;
    mRemoteMegaError.syncError = mega::SyncError::NO_SYNC_ERROR;
}

void Syncs::clearRemoteError()
{
    mRemoteError.reset();
    mRemoteStringMessage.clear();
    mRemoteMegaError.error = mega::MegaError::API_OK;
    mRemoteMegaError.syncError = mega::SyncError::NO_SYNC_ERROR;

    emit remoteErrorChanged(getRemoteError());
}

void Syncs::clearLocalError()
{
    mLocalErrorMessage.clear();
    emit localErrorChanged(QString());
}

QString Syncs::getLocalErrorMessage(std::optional<LocalErrors> error, const QString& path) const
{
    if (!error.has_value())
    {
        return QString();
    }

    switch (error.value())
    {
        case LocalErrors::EmptyPath:
        {
            return tr("Select a local folder to sync.");
        }

        case LocalErrors::NoAccessPermissionsCantCreate:
        {
            return QCoreApplication::translate(
                "OnboardingStrings",
                "Folder can’t be synced as you don’t have permissions to create a new folder. To "
                "continue, select an existing folder.");
        }

        case LocalErrors::NoAccessPermissionsNoExist:
        {
            return QCoreApplication::translate("MegaSyncError", "Local path not available");
        }

        case LocalErrors::CantSync:
        {
            QString errorMessage;
            SyncController::instance().isLocalFolderSyncable(path,
                                                             mega::MegaSync::TYPE_TWOWAY,
                                                             errorMessage);
            return errorMessage;
        }
    }

    return QString();
}
