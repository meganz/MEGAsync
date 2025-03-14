#include "Syncs.h"

#include "ChooseFolder.h"
#include "mega/types.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "RequestListenerManager.h"
#include "SyncsData.h"
#include "TextDecorator.h"

Syncs::Syncs(QObject* parent):
    QObject(parent),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mSyncController(SyncController::instance()),
    mSyncsData(std::make_unique<SyncsData>())
{
    connect(&mSyncController, &SyncController::syncAddStatus, this, &Syncs::onSyncAddRequestStatus);
    connect(SyncInfo::instance(), &SyncInfo::syncRemoved, this, &Syncs::onSyncRemoved);

    onSyncRemoved(nullptr);
}

void Syncs::addSync(SyncInfo::SyncOrigin origin, const QString& local, const QString& remote)
{
    cleanErrors();

    mSyncConfig.localFolder = local;

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

    mSyncConfig.remoteFolder = remote;
    mSyncConfig.localFolder = local;
    mSyncConfig.origin = origin;
    mSyncConfig.remoteHandle = remoteHandle;

    if (remoteHandle == mega::INVALID_HANDLE)
    {
        mCreatingFolder = true;

        /*
         *  need to remove the first / from the remote path,
         *  we already state in createFolder the origin point.
         */
        if (mSyncConfig.remoteFolder.indexOf(QLatin1Char('/')) == 0)
        {
            mSyncConfig.remoteFolder.remove(0, 1);
        }

        auto listener = RequestListenerManager::instance().registerAndGetFinishListener(this, true);
        mMegaApi->createFolder(mSyncConfig.remoteFolder.toUtf8().constData(),
                               MegaSyncApp->getRootNode().get(),
                               listener.get());
    }
    else
    {
        mSyncController.addSync(mSyncConfig);
    }
}

bool Syncs::checkErrorsOnSyncPaths(const QString& localPath, const QString& remotePath)
{
    helperCheckLocalSync(localPath);
    helperCheckRemoteSync(remotePath);

    return (mLocalError.has_value() || mRemoteError.has_value());
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
            if (localFolderPath == localFolder.getDefaultFolder(mSyncsData->getDefaultMegaFolder()))
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

    if (!localError.has_value())
    {
        QString errorMessage;
        auto syncability =
            mSyncController.isLocalFolderSyncable(path, mega::MegaSync::TYPE_TWOWAY, errorMessage);
        if (syncability == SyncController::CANT_SYNC)
        {
            localError = LocalErrors::CantSync;
        }
    }

    if (mLocalError != localError)
    {
        mLocalError.swap(localError);
        mSyncsData->setLocalError(getLocalError());
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
        else if (path != mSyncsData->getDefaultMegaPath())
        {
            remoteError = RemoteErrors::CantSync;
        }
    }

    if (mRemoteError != remoteError)
    {
        mRemoteError.swap(remoteError);
        mSyncsData->setRemoteError(getRemoteError());
    }
}

bool Syncs::checkLocalSync(const QString& path)
{
    helperCheckLocalSync(path);
    return (!mLocalError.has_value());
}

bool Syncs::checkRemoteSync(const QString& path)
{
    helperCheckRemoteSync(path);
    return (!mRemoteError.has_value());
}

void Syncs::onRequestFinish(mega::MegaRequest* request, mega::MegaError* error)
{
    if (request->getType() == mega::MegaRequest::TYPE_CREATE_FOLDER && mCreatingFolder &&
        (mSyncConfig.remoteFolder.compare(QString::fromUtf8(request->getName())) == 0))
    {
        mCreatingFolder = false;

        if (error->getErrorCode() == mega::MegaError::API_OK)
        {
            auto megaNode = std::shared_ptr<mega::MegaNode>(
                mMegaApi->getNodeByPath(mSyncConfig.remoteFolder.toUtf8().constData(),
                                        MegaSyncApp->getRootNode().get()));
            if (megaNode != nullptr)
            {
                mSyncConfig.remoteHandle = request->getNodeHandle();
                mSyncController.addSync(mSyncConfig);
            }
            else
            {
                mRemoteError = RemoteErrors::CantCreateRemoteFolder;

                mSyncsData->setRemoteError(getRemoteError());
            }
        }
        else if (error->getErrorCode() != mega::MegaError::API_ESSL
                && error->getErrorCode() != mega::MegaError::API_ESID)
        {
            mRemoteError = RemoteErrors::CantCreateRemoteFolderMsg;
            mRemoteMegaError.error = mega::MegaError::API_OK;
            mRemoteMegaError.syncError = mega::SyncError::NO_SYNC_ERROR;
            mRemoteStringMessage = QString::fromUtf8(error->getErrorString());

            mSyncsData->setRemoteError(getRemoteError());
        }
    }
}

void Syncs::onSyncRemoved(std::shared_ptr<SyncSettings> syncSettings)
{
    Q_UNUSED(syncSettings)

    emit mSyncsData->syncRemoved();
}

void Syncs::onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString name)
{
    Q_UNUSED(name)

    if (errorCode != mega::MegaError::API_OK)
    {
        mRemoteError = RemoteErrors::CantAddSync;
        mRemoteMegaError.error = errorCode;
        mRemoteMegaError.syncError = syncErrorCode;

        mSyncsData->setRemoteError(getRemoteError());
    }
    else
    {
        emit mSyncsData->syncSetupSuccess();
    }
}

QString Syncs::getLocalError() const
{
    if (!mLocalError.has_value())
    {
        return {};
    }

    switch (mLocalError.value())
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
            mSyncController.isLocalFolderSyncable(mSyncConfig.localFolder,
                                                  mega::MegaSync::TYPE_TWOWAY,
                                                  errorMessage);
            return errorMessage;
        }
    }

    return {};
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
                return mSyncController.getRemoteFolderErrorMessage(mRemoteMegaError.error,
                                                                   mRemoteMegaError.syncError);
            }
            else
            {
                return tr("Folder can't be synced as it can't be located. "
                          "It may have been moved or deleted, or you might not have access.");
            }
        }

        case RemoteErrors::CantCreateRemoteFolder:
        {
            return tr("%1 folder doesn't exist").arg(mSyncConfig.remoteFolder);
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
            QString msg =
                mSyncController.getErrorString(mRemoteMegaError.error, mRemoteMegaError.syncError);
            dec.process(msg);

            return msg;
        }
    }

    return {};
}

void Syncs::cleanErrors()
{
    clearLocalError();
    clearRemoteError();
}

void Syncs::clearRemoteError()
{
    mRemoteError.reset();
    mRemoteStringMessage.clear();
    mRemoteMegaError.error = mega::MegaError::API_OK;
    mRemoteMegaError.syncError = mega::SyncError::NO_SYNC_ERROR;

    mSyncsData->setRemoteError({});
}

void Syncs::clearLocalError()
{
    mLocalError.reset();
    mSyncsData->setLocalError({});
}

SyncsData* Syncs::getSyncsData() const
{
    return mSyncsData.get();
}
