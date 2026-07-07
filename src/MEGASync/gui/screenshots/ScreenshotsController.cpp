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
    // send notification and maybe copy the link to it in the clipboard.

    // 1 - if no link desired to be created, notify the user about the upload succes.
    // 2 - if link wanted, generate link and inform user.
    //     2.1 - if link generation fails should we notify the user :
    //           upload success and link generation failed.

    mLinkGenerator.generateLink(nodeHandle);
}

void ScreenshotsController::onLinkFailed(const QString& message, bool authExpired)
{
    // send error notification
    mMegaApplication->showErrorMessage(message);
}

void ScreenshotsController::onLinkReady(const QString& link)
{
    // send notification & copy link to the clipboard.
    // A public link has been created, put it in the clipboard and inform users
    QApplication::clipboard()->setText(link);
    mMegaApplication->showInfoMessage(tr("The link has been copied to the clipboard"));
}
