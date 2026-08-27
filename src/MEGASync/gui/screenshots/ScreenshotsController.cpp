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
    // Full reset, not just the queue: a capture waiting on the duplicate-name
    // dialog or on createFolder holds the uploader's single in-flight slot
    // without a transfer to release it, and that slot would stay taken across
    // the next login.
    mUploader.reset();
    // The offer dialog may still be open; its capture belongs to the session
    // that is going away.
    mPendingOfferCapturePath.clear();
}

bool ScreenshotsController::isRunning() const
{
    return mDetector.isRunning();
}

void ScreenshotsController::refresh()
{
    // The offer dialog's close callback lands here during logout too, after the
    // deferred cleanup has already unlinked Preferences: the per-user reads
    // below would fall back to the global scope and restart the detector with
    // no session behind it. stop() has already run at that point, so there is
    // nothing to refresh.
    if (!Preferences::instance()->logged())
    {
        return;
    }

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

void ScreenshotsController::onCaptureUploadSkipped()
{
    mUploader.skipCurrent();
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
    // The offer dialog stays clickable during the short window between
    // onLogout() and the deferred closeAllDialogs(). Accepting it then would
    // write the preference into the global scope (no user is logged) and pop
    // the info dialog in guest state, so treat it as a decline.
    if (!Preferences::instance()->logged())
    {
        return;
    }

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
    // Nothing here was asked for by the user: the capture happened because they
    // pressed a key on their own desktop. Only notify about outcomes they can
    // actually act on, and log the rest.
    switch (error)
    {
        // The capture was moved, renamed or deleted between being detected and
        // the transfer starting. Routine, and there is nothing to fix.
        case ScreenshotUploader::Error::LocalFileMissing:
        // The user logged out; uploads stopping is the expected consequence.
        case ScreenshotUploader::Error::NotLoggedIn:
        // Session expiry has its own app-wide handling, which already tells the
        // user and takes them to the login flow. A second notification from
        // here would only duplicate it.
        case ScreenshotUploader::Error::AuthExpired:
        case ScreenshotUploader::Error::None:
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                               QString::fromUtf8("Screenshot upload failed silently (%1): %2")
                                   .arg(static_cast<int>(error))
                                   .arg(message)
                                   .toUtf8()
                                   .constData());
            return;
        }

        // Genuine problems that leave the feature broken until they clear, so
        // the user should know. showErrorMessage() is an OS notification with
        // its own repeat-suppression, not a modal, so it is safe from a
        // background flow.
        case ScreenshotUploader::Error::FolderCreationFailed:
        case ScreenshotUploader::Error::TransferFailed:
            break;
    }

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
