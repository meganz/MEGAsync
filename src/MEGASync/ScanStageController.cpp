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
}

void ScanStageController::startDelayedScanStage()
{
    if (!mScanStageTimer.isActive())
    {
        mScanStageTimer.start(mDelayToShowDialogInMs);
    }
}

void ScanStageController::stopDelayedScanStage()
{
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

bool ScanStageController::IsInScanningState()
{
    return mIsInScanningState;
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

    if (mTransferManager)
    {
        mTransferManager->leaveBlockingState();
    }

    if (mInfoDialog)
    {
        mInfoDialog->leaveBlockingState();
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
