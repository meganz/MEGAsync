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
}

void ScanStageController::updateReference(InfoDialog *_infoDialog)
{
    mInfoDialog = _infoDialog;
}

void ScanStageController::updateReference(TransferManager *_transferManager)
{
    mTransferManager = _transferManager;

    if (isInScanningState())
    {
        mTransferManager->enterBlockingState(mCanBeCancelled);
    }
}

void ScanStageController::startDelayedScanStage(bool canBeCancelled)
{
    mCanBeCancelled = canBeCancelled;

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
}

void ScanStageController::startScanStage()
{
    mIsInScanningState = true;
    mIsInScanningStateInMinimumTime = true;
    QTimer::singleShot(mMinimumDialogDisplayTimeInMs, this,
                       &ScanStageController::onMinimumDisplayTimeElapsed);

    setUiInScanStage();
}

void ScanStageController::setUiInScanStage()
{
    emit enableTransferActions(false);

    if (mTransferManager)
    {
        mTransferManager->enterBlockingState(mCanBeCancelled);
    }

    if (mInfoDialog)
    {
        mInfoDialog->enterBlockingState(mCanBeCancelled);
    }
}

void ScanStageController::setUiInNormalStage()
{
    emit enableTransferActions(true);

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
