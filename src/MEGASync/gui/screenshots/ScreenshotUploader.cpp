#include "ScreenshotUploader.h"

#include "MegaApplication.h"
#include "RequestListenerManager.h"

#include <QCoreApplication>
#include <QFileInfo>

using namespace mega;

namespace
{
// Captures come in small bursts; anything beyond this is dropped (logged).
constexpr int MAX_QUEUED_UPLOADS = 16;
}

ScreenshotUploader::ScreenshotUploader(MegaApi* megaApi, QObject* parent):
    QObject(parent),
    mMegaApi(megaApi),
    mDelegateListener(std::make_unique<QTMegaTransferListener>(megaApi, this))
{
    // Registered on the same MegaApi the delegate listener was constructed
    // with, so the two can never end up bound to different instances.
    mMegaApi->addTransferListener(mDelegateListener.get());
}

void ScreenshotUploader::upload(const QString& localPath)
{
    if ((mBusy && mLocalPath == localPath) || mQueue.contains(localPath))
    {
        return; // already uploading or queued
    }

    if (mBusy)
    {
        if (mQueue.size() >= MAX_QUEUED_UPLOADS)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING,
                         "Screenshot upload queue full, dropping capture");
            return;
        }

        mQueue.enqueue(localPath);
        return;
    }

    startUploadFor(localPath);
}

void ScreenshotUploader::startUploadFor(const QString& localPath)
{
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
    mCurrentTag.reset();
    mTargetFolderHandle = INVALID_HANDLE;
    ensureRemoteFolderAndUpload();
}

bool ScreenshotUploader::isBusy() const
{
    return mBusy;
}

void ScreenshotUploader::reset()
{
    mQueue.clear();
    mBusy = false;
    mLocalPath.clear();
    mCurrentTag.reset();
    mTargetFolderHandle = INVALID_HANDLE;
}

void ScreenshotUploader::skipCurrent()
{
    if (!mBusy)
    {
        return;
    }

    mBusy = false;
    mLocalPath.clear();
    mCurrentTag.reset();
    mTargetFolderHandle = INVALID_HANDLE;
    processNextInQueue();
}

void ScreenshotUploader::processNextInQueue()
{
    if (!mQueue.isEmpty())
    {
        startUploadFor(mQueue.dequeue());
    }
}

void ScreenshotUploader::ensureRemoteFolderAndUpload()
{
    std::unique_ptr<MegaNode> root(mMegaApi->getRootNode());
    if (!root)
    {
        // The detector only runs once fetch-nodes has finished, so this is an
        // internal timing problem rather than something the user can act on.
        // Log it and drop the capture quietly instead of raising an error the
        // user cannot do anything with.
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING,
                     "Screenshot upload skipped: no root node available yet");
        skipCurrent();
        return;
    }

    // Ask for a node of type FOLDER specifically. MEGA permits a file and a
    // folder to share a name under one parent, and the previous
    // getNodeByPath("/Screenshots") returned whichever of the two it found
    // first. When that was a file, isFolder() was false and every capture fell
    // through to createFolder() below -- re-running it for the life of the
    // session, once per capture, with nothing to show for it.
    std::unique_ptr<MegaNode> screenShotTarget(
        mMegaApi->getChildNodeOfType(root.get(), REMOTE_FOLDER_NAME, MegaNode::TYPE_FOLDER));
    if (screenShotTarget)
    {
        startUpload(screenShotTarget->getHandle());
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

    mTargetFolderHandle = parentHandle;
    static_cast<MegaApplication*>(qApp)->uploadScreenshot(mLocalPath, parentHandle);
}

bool ScreenshotUploader::matchesPendingCapture(MegaTransfer* transfer) const
{
    if (!mBusy || !transfer)
    {
        return false;
    }

    // A sync-engine upload is never the capture we started, even when it moves
    // the very same file: the watched capture folder may sit inside a synced
    // folder, in which case the sync uploads it too. Folder transfers are the
    // parent bookkeeping entries of a recursive upload, never our single file.
    if (transfer->getType() != MegaTransfer::TYPE_UPLOAD || transfer->isSyncTransfer() ||
        transfer->isFolderTransfer())
    {
        return false;
    }

    // Same file, going into the folder we resolved for this capture.
    return transfer->getParentHandle() == mTargetFolderHandle &&
           mLocalPath == QString::fromUtf8(transfer->getPath());
}

void ScreenshotUploader::onTransferStart(MegaApi*, MegaTransfer* transfer)
{
    if (mCurrentTag.has_value() || !matchesPendingCapture(transfer))
    {
        return;
    }

    // Latch the tag: it is the transfer's stable identity for the rest of its
    // life, which the local path is not (see isCurrentTransfer).
    mCurrentTag = transfer->getTag();
}

bool ScreenshotUploader::isCurrentTransfer(MegaTransfer* transfer) const
{
    if (!mBusy || !transfer)
    {
        return false;
    }

    if (mCurrentTag.has_value())
    {
        return transfer->getTag() == *mCurrentTag;
    }

    // No tag latched (onTransferStart not seen for this transfer): fall back to
    // the identity checks so the capture still completes instead of wedging the
    // queue on a permanently busy slot.
    return matchesPendingCapture(transfer);
}

void ScreenshotUploader::onTransferFinish(MegaApi*, MegaTransfer* transfer, MegaError* error)
{
    if (isCurrentTransfer(transfer))
    {
        if (error->getErrorCode() == MegaError::API_OK)
        {
            const auto uploadedNode = transfer->getNodeHandle();
            mBusy = false;
            mCurrentTag.reset();
            mTargetFolderHandle = INVALID_HANDLE;
            emit uploadFinished(uploadedNode);
            processNextInQueue();
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
    if (!isCurrentTransfer(transfer))
    {
        return;
    }

    const auto total = transfer->getTotalBytes();
    if (total > 0)
    {
        emit uploadProgress(mLocalPath, transfer->getTransferredBytes(), total);
    }
}

void ScreenshotUploader::onTransferTemporaryError(MegaApi*,
                                                  MegaTransfer* transfer,
                                                  MegaError* error)
{
    // Connectivity drops surface here (the SDK keeps retrying with backoff
    // and resumes the transfer when the network returns). Let the UI show a
    // "waiting for connection" state instead of failing.
    Q_UNUSED(error)

    if (!isCurrentTransfer(transfer))
    {
        return;
    }

    emit waitingForConnection();
}

void ScreenshotUploader::fail(Error error, const QString& message)
{
    mBusy = false;
    mCurrentTag.reset();
    mTargetFolderHandle = INVALID_HANDLE;
    emit uploadFailed(error, message);
    processNextInQueue();
}

bool ScreenshotUploader::isAuthError(int errorCode)
{
    return errorCode == MegaError::API_ESID || errorCode == MegaError::API_EBLOCKED;
}
