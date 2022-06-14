#include "ScanStageController.h"

#include "InfoDialog.h"
#include "TransferManager.h"

ScanStageController::ScanStageController(QObject *parent)
    : QObject(parent), scanStageTimer(this)
{
    scanStageTimer.setSingleShot(true);
    connect(&scanStageTimer, &QTimer::timeout,
            this, &ScanStageController::startScanStage);
}

void ScanStageController::updateReference(InfoDialog *_infoDialog)
{
    infoDialog = _infoDialog;
}

void ScanStageController::updateReference(TransferManager *_transferManager)
{
    transferManager = _transferManager;
}

void ScanStageController::startDelayedScanStage()
{
    if (!scanStageTimer.isActive())
    {
        scanStageTimer.start(delayToShowDialogInMs);
    }
}

void ScanStageController::stopDelayedScanStage()
{
    if (scanStageTimer.isActive())
    {
        scanStageTimer.stop();
    }
    else
    {
        isInScanningState = false;
        if (isInScanningStateInMinimumTime)
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
    isInScanningState = true;
    isInScanningStateInMinimumTime = true;
    QTimer::singleShot(minimumDialogDisplayTimeInMs, this,
                       &ScanStageController::onMinimumDisplayTimeElapsed);

    setUiInScanStage();
}

void ScanStageController::setUiInScanStage()
{
    emit enableTransferActions(false);

    if (transferManager)
    {
        transferManager->enterBlockingState();
    }

    if (infoDialog)
    {
        infoDialog->enterBlockingState();
    }
}

void ScanStageController::setUiInNormalStage()
{
    emit enableTransferActions(true);

    if (transferManager)
    {
        transferManager->leaveBlockingState();
    }

    if (infoDialog)
    {
        infoDialog->leaveBlockingState();
    }
}

void ScanStageController::setUiInDisabledScanStage()
{
    if (transferManager)
    {
        transferManager->disableCancelling();
    }

    if (infoDialog)
    {
        infoDialog->disableCancelling();
    }
}

void ScanStageController::onMinimumDisplayTimeElapsed()
{
    isInScanningStateInMinimumTime = false;
    if (!isInScanningState)
    {
        setUiInNormalStage();
    }
}
