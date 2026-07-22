#include "ScreenshotUploader.h"

#include "MegaApplication.h"
#include "RequestListenerManager.h"

#include <QCoreApplication>
#include <QFileInfo>

using namespace mega;

ScreenshotUploader::ScreenshotUploader(MegaApi* megaApi, QObject* parent):
    QObject(parent),
    mMegaApi(megaApi),
    mDelegateListener(std::make_unique<QTMegaTransferListener>(megaApi, this))
{
    MegaSyncApp->getMegaApi()->addTransferListener(mDelegateListener.get());
}

void ScreenshotUploader::upload(const QString& localPath)
{
    if (mBusy)
    {
        return;
    }

    if (!mMegaApi->isLoggedIn())
    {
        fail(Error::NotLoggedIn,
             QCoreApplication::translate("ScreenshotUploader", "You are not logged in to MEGA"));
        return;
    }

    if (!QFileInfo::exists(localPath))
    {
        fail(Error::LocalFileMissing,
             QCoreApplication::translate("ScreenshotUploader",
                                         "The screenshot file no longer exists"));
        return;
    }

    mBusy = true;
    mLocalPath = localPath;
    ensureRemoteFolderAndUpload();
}

bool ScreenshotUploader::isBusy() const
{
    return mBusy;
}

void ScreenshotUploader::ensureRemoteFolderAndUpload()
{
    const QString remotePath = QString::fromUtf8("/") + QString::fromUtf8(REMOTE_FOLDER_NAME);
    std::unique_ptr<MegaNode> screenShotTarget(
        mMegaApi->getNodeByPath(remotePath.toUtf8().constData()));
    if (screenShotTarget && screenShotTarget->isFolder())
    {
        startUpload(screenShotTarget->getHandle());
        return;
    }

    std::unique_ptr<MegaNode> root(mMegaApi->getRootNode());
    if (!root)
    {
        fail(Error::FolderCreationFailed,
             QCoreApplication::translate("ScreenshotUploader",
                                         "Your MEGA account is not ready yet"));
        return;
    }

    auto listener = RequestListenerManager::instance().registerAndGetCustomFinishListener(
        this,
        [this](MegaRequest* request, MegaError* error)
        {
            if (error->getErrorCode() != MegaError::API_OK)
            {
                const Error reason = isAuthError(error->getErrorCode()) ?
                                         Error::AuthExpired :
                                         Error::FolderCreationFailed;
                fail(reason, QString::fromUtf8(error->getErrorString()));
                return;
            }

            startUpload(request->getNodeHandle());
        });

    mMegaApi->createFolder(REMOTE_FOLDER_NAME, root.get(), listener.get());
}

void ScreenshotUploader::startUpload(MegaHandle parentHandle)
{
    std::unique_ptr<MegaNode> parent(mMegaApi->getNodeByHandle(parentHandle));
    if (!parent)
    {
        fail(Error::FolderCreationFailed,
             QCoreApplication::translate("ScreenshotUploader",
                                         "Could not access the Screenshots folder"));
        return;
    }

    static_cast<MegaApplication*>(qApp)->uploadScreenshot(mLocalPath, parentHandle);
}

void ScreenshotUploader::onTransferFinish(MegaApi*, MegaTransfer* transfer, MegaError* error)
{
    if (mBusy && mLocalPath == QString::fromUtf8(transfer->getPath()))
    {
        if (error->getErrorCode() == MegaError::API_OK)
        {
            mBusy = false;
            emit uploadFinished(transfer->getNodeHandle());
            return;
        }

        if (isAuthError(error->getErrorCode()))
        {
            fail(Error::AuthExpired,
                 QCoreApplication::translate("ScreenshotUploader",
                                             "Your session has expired. Please log in again"));
            return;
        }

        fail(Error::TransferFailed, QString::fromUtf8(error->getErrorString()));
    }
}

void ScreenshotUploader::onTransferUpdate(MegaApi*, MegaTransfer* transfer)
{
    const auto total = transfer->getTotalBytes();
    if (total > 0)
    {
        emit uploadProgress(mLocalPath, transfer->getTransferredBytes(), total);
    }
}

void ScreenshotUploader::onTransferTemporaryError(MegaApi*, MegaTransfer*, MegaError* error)
{
    // Connectivity drops surface here (the SDK keeps retrying with backoff
    // and resumes the transfer when the network returns). Let the UI show a
    // "waiting for connection" state instead of failing.
    Q_UNUSED(error)
    emit waitingForConnection();
}

void ScreenshotUploader::fail(Error error, const QString& message)
{
    mBusy = false;
    emit uploadFailed(error, message);
}

bool ScreenshotUploader::isAuthError(int errorCode)
{
    return errorCode == MegaError::API_ESID || errorCode == MegaError::API_EBLOCKED;
}
