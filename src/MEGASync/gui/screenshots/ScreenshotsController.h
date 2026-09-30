#ifndef SCREENSHOTS_CONTROLLER_H
#define SCREENSHOTS_CONTROLLER_H

#include "ScreenCaptureDetector.h"
#include "ScreenshotLinkGenerator.h"
#include "ScreenshotUploader.h"

#include <QObject>

class MegaApplication;

// Entry point of the screenshot workflow. Owns the capture detector.
// Started by MegaApplication once the account is ready; stopped on logout.
class ScreenshotsController: public QObject
{
    Q_OBJECT

public:
    static ScreenshotsController* instance();

    void start();
    void stop();
    bool isRunning() const;
    // Recomputes whether the detector should run from Preferences (feature
    // enabled, or the one-time enable offer still pending) and starts/stops
    // it accordingly. Single source of truth for that decision.
    void refresh();
    void enableClipboardCopyScreenshotLink(bool enabled);
    // Called by MegaApplication when a queued capture will not get a transfer
    // after all: the user declined it or the destination was unusable. Unblocks
    // the uploader without reporting an error -- nothing failed, and the
    // capture is still on disk.
    void onCaptureUploadSkipped();
    // Called by ScreenshotComponent when the user accepts the one-time enable
    // offer: turns the setting on, uploads the capture that triggered the
    // offer and brings up the info dialog.
    void onEnableOfferAccepted();

private slots:
    void onCaptureDetected(const QString& filePath);
    void onUploadFailed(ScreenshotUploader::Error error, const QString& message);
    void onUploadFinished(mega::MegaHandle nodeHandle);
    void onLinkReady(const QString& link);
    void onLinkFailed(const QString& message);

private:
    explicit ScreenshotsController(QObject* parent = nullptr);

    // Shows the one-time "enable screenshot uploads" offer dialog. The
    // detector is refreshed when the dialog closes, whatever the choice.
    void showEnableOfferDialog();

    ScreenCaptureDetector mDetector;
    ScreenshotUploader mUploader;
    ScreenshotLinkGenerator mLinkGenerator;
    MegaApplication* mMegaApplication;
    bool mClipboardScreenshotLinkCopyEnabled = false;
    // Capture that triggered the enable offer; uploaded if the user accepts,
    // dropped otherwise.
    QString mPendingOfferCapturePath;
};

#endif // SCREENSHOTS_CONTROLLER_H
