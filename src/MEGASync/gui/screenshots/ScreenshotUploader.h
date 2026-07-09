#ifndef SCREENSHOT_UPLOADER_H
#define SCREENSHOT_UPLOADER_H

#include "megaapi.h"
#include "QTMegaTransferListener.h"

#include <QObject>
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

    // Asynchronous; one upload at a time. Result arrives via uploadFinished()
    // or uploadFailed().
    void upload(const QString& localPath);
    bool isBusy() const;

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
    void onTransferFinish(mega::MegaApi* api,
                          mega::MegaTransfer* transfer,
                          mega::MegaError* error) override;
    void onTransferUpdate(mega::MegaApi* api, mega::MegaTransfer* transfer) override;
    void onTransferTemporaryError(mega::MegaApi* api,
                                  mega::MegaTransfer* transfer,
                                  mega::MegaError* error) override;

    void ensureRemoteFolderAndUpload();
    void startUpload(mega::MegaHandle parentHandle);
    void fail(Error error, const QString& message);
    static bool isAuthError(int errorCode);

    mega::MegaApi* mMegaApi;
    std::unique_ptr<mega::QTMegaTransferListener> mDelegateListener;
    QString mLocalPath;
    bool mBusy = false;
};

#endif // SCREENSHOT_UPLOADER_H
