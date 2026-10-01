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
    if ((mBusy && mLocalPath == localPath) || mQueue.contains(localPath) ||
        mTrackedTransfers.key(localPath, -1) != -1)
    {
        return; // already handed over, in flight or queued
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
    mTrackedTransfers.clear();
    mBusy = false;
    mLocalPath.clear();
    mTargetFolderHandle = INVALID_HANDLE;
}

void ScreenshotUploader::skipCurrent()
{
    if (!mBusy)
    {
        return;
    }

    releaseSlot();
}

void ScreenshotUploader::releaseSlot()
{
    mBusy = false;
    mLocalPath.clear();
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
    if (!matchesPendingCapture(transfer))
    {
        return;
    }

    // The SDK owns the capture from here: its transfer is created, persisted
    // and visible in the Transfer Manager, and it honours pause/resume on its
    // own. Track it by tag (its stable identity, which the local path is not)
    // and hand the next capture over right away. Holding the slot until the
    // transfer finished parked every later capture in this in-memory queue --
    // invisible, uncancellable and lost on restart -- for as long as the user
    // kept transfers paused.
    mTrackedTransfers.insert(transfer->getTag(), mLocalPath);
    releaseSlot();
}

QString ScreenshotUploader::trackedPath(MegaTransfer* transfer) const
{
    if (!transfer)
    {
        return QString();
    }

    return mTrackedTransfers.value(transfer->getTag());
}

void ScreenshotUploader::onTransferFinish(MegaApi*, MegaTransfer* transfer, MegaError* error)
{
    if (matchesPendingCapture(transfer))
    {
        // Finished before its start was seen: adopt it so the slot is not
        // wedged on a transfer that no longer exists.
        mTrackedTransfers.insert(transfer->getTag(), mLocalPath);
        releaseSlot();
    }

    const QString path = trackedPath(transfer);
    if (path.isEmpty())
    {
        return;
    }
    mTrackedTransfers.remove(transfer->getTag());

    if (error->getErrorCode() == MegaError::API_OK)
    {
        emit uploadFinished(transfer->getNodeHandle());
        return;
    }

    // Cancelled by the user, e.g. from the Transfer Manager. The SDK finishes it
    // as STATE_CANCELLED + API_EINCOMPLETE; that is not a failure, so nothing
    // is reported.
    if (transfer->getState() == MegaTransfer::STATE_CANCELLED)
    {
        MegaApi::log(
            MegaApi::LOG_LEVEL_INFO,
            QString::fromUtf8("Screenshot upload cancelled: %1").arg(path).toUtf8().constData());
        return;
    }

    // Outcome of a transfer the SDK already owned: report it, but leave the
    // hand-over slot alone -- it may be busy with a later capture.
    if (isAuthError(error->getErrorCode()))
    {
        emit uploadFailed(
            Error::AuthExpired,
            QCoreApplication::translate("ScreenshotUploader",
                                        "Your session has expired. Please log in again"));
        return;
    }

    emit uploadFailed(Error::TransferFailed, QString::fromUtf8(error->getErrorString()));
}

void ScreenshotUploader::onTransferUpdate(MegaApi*, MegaTransfer* transfer)
{
    const QString path = trackedPath(transfer);
    if (path.isEmpty())
    {
        return;
    }

    const auto total = transfer->getTotalBytes();
    if (total > 0)
    {
        emit uploadProgress(path, transfer->getTransferredBytes(), total);
    }
}

void ScreenshotUploader::onTransferTemporaryError(MegaApi*,
                                                  MegaTransfer* transfer,
                                                  MegaError* error)
{
    const QString path = trackedPath(transfer);
    if (path.isEmpty())
    {
        return;
    }

    const int code = error->getErrorCode();
    if (code == MegaError::API_EOVERQUOTA || code == MegaError::API_EPAYWALL)
    {
        // Storage over quota: the SDK keeps the upload parked and resumes it
        // on its own once the account has space again, possibly days later.
        // Stop tracking it: a link copied when it finally completes would
        // overwrite the clipboard out of context.
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                     QString::fromUtf8("Screenshot upload parked by the SDK (storage over "
                                       "quota), no longer tracked: %1")
                         .arg(path)
                         .toUtf8()
                         .constData());
        mTrackedTransfers.remove(transfer->getTag());
        return;
    }

    // Connectivity drops surface here (the SDK keeps retrying with backoff
    // and resumes the transfer when the network returns). Let the UI show a
    // "waiting for connection" state instead of failing.
    emit waitingForConnection();
}

void ScreenshotUploader::fail(Error error, const QString& message)
{
    mBusy = false;
    mLocalPath.clear();
    mTargetFolderHandle = INVALID_HANDLE;
    emit uploadFailed(error, message);
    processNextInQueue();
}

bool ScreenshotUploader::isAuthError(int errorCode)
{
    return errorCode == MegaError::API_ESID || errorCode == MegaError::API_EBLOCKED;
}
