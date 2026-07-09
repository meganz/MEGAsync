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

    connect(&mUploader,
            &ScreenshotUploader::uploadProgress,
            this,
            &ScreenshotsController::onUploadProgress);

    connect(&mLinkGenerator,
            &ScreenshotLinkGenerator::linkFailed,
            this,
            &ScreenshotsController::onLinkFailed);

    connect(&mLinkGenerator,
            &ScreenshotLinkGenerator::linkReady,
            this,
            &ScreenshotsController::onLinkReady);
}

void ScreenshotsController::start()
{
    mDetector.start();
}

void ScreenshotsController::stop()
{
    mDetector.stop();
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
    mFile = QFileInfo(filePath).fileName();
    mUploader.upload(filePath);

    MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SCREENSHOT_UPLOADED);
}

void ScreenshotsController::onUploadFailed(ScreenshotUploader::Error error, const QString& message)
{
    mMegaApplication->showErrorMessage(message);
    mFile.clear();
}

void ScreenshotsController::onUploadFinished(mega::MegaHandle nodeHandle)
{
    mMegaApplication->showInfoMessage(tr("The screenshot \"%1\" has been uploaded.").arg(mFile));

    if (mClipboardScreenshotLinkCopyEnabled)
    {
        mLinkGenerator.generateLink(nodeHandle);

        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::SCREENSHOT_LINK_CREATED);
    }

    mFile.clear();
}

void ScreenshotsController::onLinkFailed(const QString& message, bool authExpired)
{
    mMegaApplication->showErrorMessage(message);
}

void ScreenshotsController::onLinkReady(const QString& link)
{
    // no need to do anything, will be handled at MegaApplication::onRequestFinish.
}

void ScreenshotsController::onUploadProgress(QString filename,
                                             long long currentProgress,
                                             long long totalSize)
{
    qDebug()
        << QString::fromUtf8("{%0}:{%1}/{%2}").arg(filename).arg(currentProgress).arg(totalSize);
}
