#include "ScreenshotsController.h"

#include "DialogOpener.h"
#include "MegaApplication.h"
#include "ScreenshotComponent.h"
#include "SettingsDialog.h"
#include "StatsEventHandler.h"
#include "Utilities.h"

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
            &ScreenshotLinkGenerator::linkReady,
            this,
            &ScreenshotsController::onLinkReady);

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

void ScreenshotsController::refresh()
{
    const bool shouldRun = Preferences::instance()->uploadScreenshotsEnabled() ||
                           !Preferences::instance()->isOneTimeActionUserDone(
                               Preferences::ONE_TIME_ACTION_SCREENSHOT_ENABLE_OFFER);

    if (shouldRun && !mDetector.isRunning())
    {
        mDetector.start();
    }
    else if (!shouldRun && mDetector.isRunning())
    {
        mDetector.stop();
    }
}

void ScreenshotsController::enableClipboardCopyScreenshotLink(bool enabled)
{
    mClipboardScreenshotLinkCopyEnabled = enabled;
}

void ScreenshotsController::onCaptureUploadDiscarded()
{
    mUploader.abortCurrent();
}

void ScreenshotsController::onCaptureDetected(const QString& filePath)
{
    if (!Preferences::instance()->uploadScreenshotsEnabled() &&
        !Preferences::instance()->isOneTimeActionUserDone(
            Preferences::ONE_TIME_ACTION_SCREENSHOT_ENABLE_OFFER))
    {
        Preferences::instance()->setOneTimeActionUserDone(
            Preferences::ONE_TIME_ACTION_SCREENSHOT_ENABLE_OFFER,
            true);

        mPendingOfferCapturePath = filePath;
        showEnableOfferDialog();

        return;
    }

    if (Preferences::instance()->uploadScreenshotsEnabled())
    {
        mUploader.upload(filePath);
    }
}

void ScreenshotsController::onEnableOfferAccepted()
{
    Preferences::instance()->setUploadScreenshotsEnabled(true);

    // Reflect the change if the settings dialog is currently open.
    if (auto dialogInfo = DialogOpener::findDialog<SettingsDialog>())
    {
        dialogInfo->getDialog()->refreshScreenshotUploadSettings();
    }

    if (!mPendingOfferCapturePath.isEmpty())
    {
        mUploader.upload(mPendingOfferCapturePath);
        mPendingOfferCapturePath.clear();
    }

    mMegaApplication->showInfoDialog();
}

void ScreenshotsController::showEnableOfferDialog()
{
    QPointer<QmlDialogWrapper<ScreenshotComponent>> dialog(
        new QmlDialogWrapper<ScreenshotComponent>());
    DialogOpener::showDialog(dialog,
                             [this]()
                             {
                                 // Declined or accepted alike: the accepted
                                 // path has already consumed the pending
                                 // capture at this point.
                                 mPendingOfferCapturePath.clear();
                                 refresh();
                             });
}

void ScreenshotsController::onUploadFailed(ScreenshotUploader::Error error, const QString& message)
{
    mMegaApplication->showErrorMessage(message);
}

void ScreenshotsController::onUploadFinished(mega::MegaHandle nodeHandle)
{
    MegaSyncApp->getStatsEventHandler()->sendEvent(AppStatsEvents::EventType::SCREENSHOT_UPLOADED);

    if (mClipboardScreenshotLinkCopyEnabled)
    {
        mLinkGenerator.generateLink(nodeHandle);
    }
}

void ScreenshotsController::onLinkReady(const QString& link)
{
    // The clipboard copy and the "link copied" notification are handled by
    // the app's standard TYPE_EXPORT flow (MegaApplication::onRequestFinish),
    // like any other exported link. Here we only account for it.
    Q_UNUSED(link)

    MegaSyncApp->getStatsEventHandler()->sendEvent(
        AppStatsEvents::EventType::SCREENSHOT_LINK_CREATED);
}

void ScreenshotsController::onLinkFailed(const QString& message)
{
    // Only reached when no export request was made (node not found); SDK-side
    // export errors are reported by the app's global TYPE_EXPORT handler.
    mMegaApplication->showErrorMessage(message);
}
