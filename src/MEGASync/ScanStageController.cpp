#include "ScanStageController.h"

#include "InfoDialog.h"
#include "TransferManager.h"

ScanStageController::ScanStageController(QObject *parent)
    : QObject(parent), mInfoDialog(nullptr),
      mTransferManager(nullptr), mScanStageTimer(this)
{
    mScanStageTimer.setSingleShot(true);
    connect(&mScanStageTimer, &QTimer::timeout,
            this, &ScanStageController::startScanStage);

    mCancelStageTimer.setSingleShot(true);
    connect(&mCancelStageTimer, &QTimer::timeout,
            this, &ScanStageController::setUiInCancellingStage);
}

void ScanStageController::updateReference(InfoDialog *_infoDialog)
{
    mInfoDialog = _infoDialog;
    connect(mInfoDialog, &InfoDialog::cancelScanning,
            this, &ScanStageController::setDelayedCancellingStage);
}

void ScanStageController::updateReference(TransferManager *_transferManager)
{
    mTransferManager = _transferManager;
    connect(mTransferManager, &TransferManager::cancelScanning,
            this, &ScanStageController::setDelayedCancellingStage);
}

void ScanStageController::startDelayedScanStage()
{
    if (!mScanStageTimer.isActive())
    {
        mScanStageTimer.start(mDelayToShowDialogInMs);
    }
}

void ScanStageController::stopDelayedScanStage(bool fromCancellation)
{
    mLastScanCancelled = fromCancellation;
    if (mScanStageTimer.isActive())
    {
        mScanStageTimer.stop();
    }
    else
    {
        stopScanStage();
    }
}

void ScanStageController::onFolderTransferUpdate(const FolderTransferUpdateEvent& event)
{
    if (mTransferManager)
    {
        mTransferManager->onFolderTransferUpdate(event);
    }

    if (mInfoDialog)
    {
        mInfoDialog->updateUiOnFolderTransferUpdate(event);
    }
}

void ScanStageController::startScanStage()
{
    mIsInScanningState = true;
    mIsInScanningStateInMinimumTime = true;
    QTimer::singleShot(mMinimumDialogDisplayTimeInMs, this,
                       &ScanStageController::onMinimumDisplayTimeElapsed);

    setUiInScanStage();
}

void ScanStageController::stopScanStage()
{
    mIsInScanningState = false;
    if (mIsInScanningStateInMinimumTime)
    {
        setUiInDisabledScanStage();
    }
    else
    {
        setUiInNormalStage();
    }
}

void ScanStageController::setUiInScanStage()
{
    emit enableTransferActions(false);

    if (mTransferManager)
    {
        mTransferManager->enterBlockingState();
    }

    if (mInfoDialog)
    {
        mInfoDialog->enterBlockingState();
    }
}

void ScanStageController::setUiInNormalStage()
{
    emit enableTransferActions(true);

    if (mCancelStageTimer.isActive())
    {
        mCancelStageTimer.stop();
    }

    if (mTransferManager)
    {
        mTransferManager->leaveBlockingState(mLastScanCancelled);
    }

    if (mInfoDialog)
    {
        mInfoDialog->leaveBlockingState(mLastScanCancelled);
    }
}

void ScanStageController::setUiInDisabledScanStage()
{
    if (mTransferManager)
    {
        mTransferManager->disableCancelling();
    }

    if (mInfoDialog)
    {
        mInfoDialog->disableCancelling();
    }
}

void ScanStageController::setDelayedCancellingStage()
{
    mCancelStageTimer.start();
}

void ScanStageController::setUiInCancellingStage()
{
    if (mTransferManager)
    {
        mTransferManager->setUiInCancellingStage();
    }

    if (mInfoDialog)
    {
        mInfoDialog->setUiInCancellingStage();
    }
}

void ScanStageController::onMinimumDisplayTimeElapsed()
{
    mIsInScanningStateInMinimumTime = false;
    if (!mIsInScanningState)
    {
        setUiInNormalStage();
    }
}

bool ScanStageController::isInScanningState() const
{
    return mIsInScanningState;
}
