#include "ScreenshotsController.h"

#include "MegaApplication.h"
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

void ScreenshotsController::enableClipboardCopyScreenshotLink(bool enable)
{
    mClipboardScreenshotLinkCopyEnabled = enable;
}

void ScreenshotsController::onCaptureDetected(const QString& filePath)
{
    mUploader.upload(filePath);
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
    }
    else
    {
        mMegaApplication->showInfoMessage(tr("The screenshot has been uploaded."));
    }
}

void ScreenshotsController::onLinkFailed(const QString& message, bool authExpired)
{
    mMegaApplication->showErrorMessage(message);
}

void ScreenshotsController::onLinkReady(const QString& link)
{
    QApplication::clipboard()->setText(link);
    mMegaApplication->showInfoMessage(tr("The link has been copied to the clipboard"));
}

void ScreenshotsController::onUploadProgress(QString filename,
                                             long long currentProgress,
                                             long long totalSize)
{
    qDebug()
        << QString::fromUtf8("{%0} : {%1}/{%2}").arg(filename).arg(currentProgress).arg(totalSize);
}
