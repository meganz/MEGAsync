#ifndef SCREENSHOT_UPLOADER_H
#define SCREENSHOT_UPLOADER_H

#include "megaapi.h"
#include "QTMegaTransferListener.h"

#include <QHash>
#include <QObject>
#include <QQueue>
#include <QString>

#include <memory>

// Uploads a saved capture to the user's MEGA account. Ensures the remote
// destination folder ("/Screenshots") exists, starts the transfer and reports
// the resulting node handle. All signals are emitted on the Qt main thread
// (QTMegaTransferListener marshals SDK callbacks through the event loop).
//
// Auth note: this class never touches credentials. It reuses the app session
// owned by the SDK (the session token itself is stored encrypted by
// Preferences/EncryptedSettings), so auth expiry surfaces here only as the
// AuthExpired error.
class ScreenshotUploader: public QObject, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    enum class Error
    {
        None,
        NotLoggedIn, // no session at all (user logged out)
        AuthExpired, // session was invalidated server-side (API_ESID/API_EBLOCKED)
        LocalFileMissing,
        FolderCreationFailed,
        TransferFailed
    };
    Q_ENUM(Error)

    explicit ScreenshotUploader(mega::MegaApi* megaApi, QObject* parent = nullptr);
    ~ScreenshotUploader() override = default;

    // Asynchronous. Captures are handed to the SDK one at a time (resolving the
    // destination folder and the upload queue's conflict handling are
    // sequential); once the SDK has created a capture's transfer it is tracked
    // by tag and the next capture is handed over, so any number can be in
    // flight -- all visible in the Transfer Manager and persisted by the SDK,
    // paused or not. Each one reports via uploadFinished() or uploadFailed(),
    // except a transfer the user cancelled, which reports nothing.
    void upload(const QString& localPath);
    // True while a capture is between detection and SDK hand-over.
    bool isBusy() const;
    // Teardown (logout / app shutdown). Drops queued uploads AND forgets the
    // capture being handed over, so the next session starts from a clean
    // slate: a capture can be in that state with no transfer created yet
    // (waiting on the duplicate-name dialog, or on createFolder), and leaving
    // the busy slot taken would block every later capture for the life of the
    // process. Transfers the SDK already owns are forgotten too.
    //
    // Deliberately emits no signal and calls into no SDK object: one caller
    // runs during shutdown, after the MegaApi instances have been removed. A
    // transfer the SDK already created is left to the SDK's own teardown, and
    // its completion callback is ignored from here on.
    void reset();
    // The capture was deliberately not attempted, or was declined by the user.
    // Nothing went wrong, so nothing is reported: releases the hand-over slot
    // and moves on to the next queued capture. Callers that do have something
    // to tell the user report it themselves.
    void skipCurrent();

    static inline const char* REMOTE_FOLDER_NAME = "Screenshots";

signals:
    void uploadProgress(QString filename, long long currentTransfered, long long totalTransfered);
    void uploadFinished(mega::MegaHandle nodeHandle);
    void uploadFailed(ScreenshotUploader::Error error, const QString& message);
    // The SDK is retrying after a connectivity loss. The transfer is still
    // alive; it resumes automatically when the network is back. Not emitted
    // for storage over quota: that transfer is dropped from tracking instead.
    void waitingForConnection();

private:
    // MegaTransferListener (called on the Qt main thread via the delegate)
    void onTransferStart(mega::MegaApi* api, mega::MegaTransfer* transfer) override;
    void onTransferFinish(mega::MegaApi* api,
                          mega::MegaTransfer* transfer,
                          mega::MegaError* error) override;
    void onTransferUpdate(mega::MegaApi* api, mega::MegaTransfer* transfer) override;
    void onTransferTemporaryError(mega::MegaApi* api,
                                  mega::MegaTransfer* transfer,
                                  mega::MegaError* error) override;

    void startUploadFor(const QString& localPath);
    void processNextInQueue();
    void ensureRemoteFolderAndUpload();
    void startUpload(mega::MegaHandle parentHandle);
    // Failure of the hand-over itself (no session, file gone, folder): reports
    // it and frees the slot for the next capture.
    void fail(Error error, const QString& message);
    // Releases the hand-over slot and moves on to the next queued capture.
    void releaseSlot();
    // Local path of the capture a transfer belongs to, or an empty string if
    // the transfer is not one of ours; the listener is global, so callbacks
    // arrive for every app transfer. Keyed by transfer tag rather than local
    // path: the SDK rewrites a transfer's path on completion when its own
    // local name for the file differs (MegaApiImpl::file_complete), and a
    // path alone does not distinguish our upload from a sync or manual upload
    // of the same file.
    QString trackedPath(mega::MegaTransfer* transfer) const;
    // Identity checks used to adopt a starting transfer as the capture being
    // handed over, before its tag is known.
    bool matchesPendingCapture(mega::MegaTransfer* transfer) const;
    static bool isAuthError(int errorCode);

    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaTransferListener> mDelegateListener;
    // Capture currently being handed to the SDK (while mBusy), and the folder
    // it is going into; both are part of the identity check in
    // onTransferStart, so a same-named file going elsewhere is never mistaken
    // for it.
    QString mLocalPath;
    mega::MegaHandle mTargetFolderHandle = mega::INVALID_HANDLE;
    bool mBusy = false;
    QQueue<QString> mQueue;
    // Transfers the SDK has created for our captures, by tag, with the local
    // path each carries. Entries leave on finish, on a storage-quota park and
    // on reset().
    QHash<int, QString> mTrackedTransfers;
};

#endif // SCREENSHOT_UPLOADER_H
