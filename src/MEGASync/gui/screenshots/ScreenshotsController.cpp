#include "ScreenshotsController.h"

#include "MegaApplication.h"
#include "StatsEventHandler.h"
#include "Utilities.h"

#include <qclipboard.h>

ScreenshotsController* ScreenshotsController::instance()
{
    static ScreenshotsController controller;
    return &controller;
}

ScreenshotsController::ScreenshotsController(QObject* parent):
    QObject(parent),
    mUploader(MegaSyncApp->getMegaApi()),
    mLinkGenerator(MegaSyncApp->getMegaApi()),
    mMegaApplication(static_cast<MegaApplication*>(qApp))
{
    connect(&mDetector,
            &ScreenCaptureDetector::captureDetected,
            this,
            &ScreenshotsController::onCaptureDetected);

    connect(&mUploader,
            &ScreenshotUploader::uploadFailed,
            this,
            &ScreenshotsController::onUploadFailed);

    connect(&mUploader,
            &ScreenshotUploader::uploadFinished,
            this,
            &ScreenshotsController::onUploadFinished);

    connect(&mLinkGenerator,
            &ScreenshotLinkGenerator::linkFailed,
            this,
            &ScreenshotsController::onLinkFailed);
}

void ScreenshotsController::start()
{
    mDetector.start();
}

void ScreenshotsController::stop()
{
    mDetector.stop();
    mUploader.clearPending();
}

bool ScreenshotsController::isRunning() const
{
    return mDetector.isRunning();
}

void ScreenshotsController::enableClipboardCopyScreenshotLink(bool enabled)
{
    mClipboardScreenshotLinkCopyEnabled = enabled;
}

void ScreenshotsController::onCaptureDetected(const QString& filePath)
{
    if (!Preferences::instance()->uploadScreenshotsEnabled() &&
        !Preferences::instance()->isOneTimeActionDone(
            Preferences::ONE_TIME_ACTION_SCREENSHOT_ENABLE_OFFER))
    {
        // show advertise.

        Preferences::instance()->setOneTimeActionUserDone(
            Preferences::ONE_TIME_ACTION_SCREENSHOT_ENABLE_OFFER,
            true);

        return;
    }

    if (Preferences::instance()->uploadScreenshotsEnabled())
    {
        mUploader.upload(filePath);
        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::SCREENSHOT_UPLOADED);
    }
}

void ScreenshotsController::onUploadFailed(ScreenshotUploader::Error error, const QString& message)
{
    mMegaApplication->showErrorMessage(message);
}

void ScreenshotsController::onUploadFinished(mega::MegaHandle nodeHandle)
{
    if (mClipboardScreenshotLinkCopyEnabled)
    {
        mLinkGenerator.generateLink(nodeHandle);

        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::SCREENSHOT_LINK_CREATED);
    }
}

void ScreenshotsController::onLinkFailed(const QString& message, bool authExpired)
{
    mMegaApplication->showErrorMessage(message);
}