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

private slots:
    void onCaptureDetected(const QString& filePath);
    void onUploadFailed(ScreenshotUploader::Error error, const QString& message);
    void onUploadFinished(mega::MegaHandle nodeHandle);
    void onLinkFailed(const QString& message, bool authExpired);
    void onLinkReady(const QString& link);

private:
    explicit ScreenshotsController(QObject* parent = nullptr);

    ScreenCaptureDetector mDetector;
    ScreenshotUploader mUploader;
    ScreenshotLinkGenerator mLinkGenerator;
    MegaApplication* mMegaApplication;
};

#endif // SCREENSHOTS_CONTROLLER_H
