#ifndef SCREENSHOT_UPLOADER_H
#define SCREENSHOT_UPLOADER_H

#include "megaapi.h"
#include "QTMegaTransferListener.h"

#include <QObject>
#include <QQueue>
#include <QString>

#include <memory>
#include <optional>

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

    // Asynchronous; one transfer in flight at a time, further captures are
    // queued and uploaded in order. Each one reports via uploadFinished()
    // or uploadFailed().
    void upload(const QString& localPath);
    bool isBusy() const;
    // Teardown (logout / app shutdown). Drops queued uploads AND forgets the
    // capture currently in flight, so the next session starts from a clean
    // slate: a capture can be in flight with no transfer created yet (waiting
    // on the duplicate-name dialog, or on createFolder), and leaving the busy
    // slot taken would block every later capture for the life of the process.
    //
    // Deliberately emits no signal and calls into no SDK object: one caller
    // runs during shutdown, after the MegaApi instances have been removed. A
    // transfer the SDK already created is left to the SDK's own teardown, and
    // its completion callback is ignored from here on.
    void reset();
    // The capture was deliberately not attempted, or was declined by the user.
    // Nothing went wrong, so nothing is reported: releases the in-flight slot
    // and moves on to the next queued capture. Callers that do have something
    // to tell the user report it themselves.
    void skipCurrent();

    // True if the path the SDK reports for a transfer (MegaTransfer::getPath())
    // names the same file as a local path obtained through Qt. The two need not
    // be the same string; see the definition. Public for the unit tests.
    static bool isSameLocalPath(const QString& localPath, const char* transferPath);

    static inline const char* REMOTE_FOLDER_NAME = "Screenshots";

signals:
    void uploadProgress(QString filename, long long currentTransfered, long long totalTransfered);
    void uploadFinished(mega::MegaHandle nodeHandle);
    void uploadFailed(ScreenshotUploader::Error error, const QString& message);
    // The SDK is retrying (typically connectivity loss). The transfer is
    // still alive; it resumes automatically when the network is back.
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
    void fail(Error error, const QString& message);
    // True if the transfer belongs to the capture currently being uploaded;
    // the listener is global, so callbacks arrive for every app transfer.
    // Matches on the latched transfer tag rather than on the local path: the
    // SDK rewrites a transfer's path on completion when its own local name for
    // the file differs (MegaApiImpl::file_complete), and a path alone does not
    // distinguish our upload from a sync or manual upload of the same file.
    bool isCurrentTransfer(mega::MegaTransfer* transfer) const;
    // Identity checks used to adopt a starting transfer as the current
    // capture, before its tag is known.
    bool matchesPendingCapture(mega::MegaTransfer* transfer) const;
    static bool isAuthError(int errorCode);

    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaTransferListener> mDelegateListener;
    QString mLocalPath;
    QQueue<QString> mQueue;
    // Tag of the transfer carrying the current capture, latched in
    // onTransferStart. Empty until the SDK reports the transfer starting.
    std::optional<int> mCurrentTag;
    // Folder the current capture is being uploaded into; part of the identity
    // check, so a same-named file going elsewhere is never mistaken for it.
    mega::MegaHandle mTargetFolderHandle = mega::INVALID_HANDLE;
    bool mBusy = false;
};

#endif // SCREENSHOT_UPLOADER_H
