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
    // Called by MegaApplication when the queued capture was discarded before
    // a transfer was created; unblocks the uploader.
    void onCaptureUploadDiscarded();

private slots:
    void onCaptureDetected(const QString& filePath);
    void onUploadFailed(ScreenshotUploader::Error error, const QString& message);
    void onUploadFinished(mega::MegaHandle nodeHandle);
    void onLinkReady(const QString& link);
    void onLinkFailed(const QString& message);

private:
    explicit ScreenshotsController(QObject* parent = nullptr);

    ScreenCaptureDetector mDetector;
    ScreenshotUploader mUploader;
    ScreenshotLinkGenerator mLinkGenerator;
    MegaApplication* mMegaApplication;
    bool mClipboardScreenshotLinkCopyEnabled = false;
};

#endif // SCREENSHOTS_CONTROLLER_H
