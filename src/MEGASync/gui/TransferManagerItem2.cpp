#include "TransferManagerItem2.h"
#include "ui_TransferManagerItem.h"
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"
#include "MegaApplication.h"

#include <QMouseEvent>

constexpr int PB_PRECISION = 1000;

using namespace mega;

TransferManagerItem2::TransferManagerItem2(QWidget *parent) :
    QWidget (parent),
    mUi (new Ui::TransferManagerItem),
    mPreferences (Preferences::instance()),
    mTransferTag (-1),
    mIsPaused (false),
    mIsFinished (false),
    mRow (0)
{
    mUi->setupUi(this);
    mUi->pbTransfer->setMaximum(PB_PRECISION);
}

TransferManagerItem2::~TransferManagerItem2()
{
    delete mUi;
}

void TransferManagerItem2::updateUi(const QExplicitlySharedDataPointer<TransferData> data, int row)
{
    // Update members
    mRow = row;
    mIsPaused = false;
    mIsFinished = false;

    QIcon icon;

    TransferData::TransferState state (data->mState);
    TransferData::TransferType type (data->mType);

    // Data to update only if another transfer is displayed
    if (mTransferTag != data->mTag)
    {
        mTransferTag = data->mTag;
        // File type icon
        icon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                              data->mFilename, QLatin1Literal(":/images/drag_")));
        mUi->tFileType->setIcon(icon);
        // Total size
        mUi->lTotal->setText(Utilities::getSizeString(data->mTotalSize));
        // File name
        mUi->lTransferName->setToolTip(data->mFilename);

        // Transfer type icon
        switch (type)
        {
            case TransferData::TransferType::TRANSFER_DOWNLOAD:
            case TransferData::TransferType::TRANSFER_LTCPDOWNLOAD:
            {
                icon = Utilities::getCachedPixmap(QLatin1Literal(
                                                      ":/images/arrow_download_ico.png"));
                break;
            }
            case TransferData::TransferType::TRANSFER_UPLOAD:
            {
                icon = Utilities::getCachedPixmap(QLatin1Literal(
                                                      ":/images/arrow_upload_ico.png"));
                break;
            }
        }
        mUi->bItemSpeed->setIcon(icon);
    }

    // Data to update in any case
    QString statusString;
    QString timeString;
    QString speedString;
    QString pauseResumeTooltip;
    QString cancelClearTooltip;
    bool showTPauseResume(true);
    bool showTCancelClear(true);

    // Amount transfered
    auto transferedB (data->mTransferredBytes);
    auto totalB (data->mTotalSize);
    mUi->lDone->setText(Utilities::getSizeString(transferedB));
    // Progress bar
    int permil = state & (TransferData::TransferState::TRANSFER_COMPLETED
                          | TransferData::TransferState::TRANSFER_COMPLETING) ?
                     PB_PRECISION
                   : totalB > 0 ? Utilities::partPer(transferedB, totalB, PB_PRECISION)
                                : 0;
    mUi->pbTransfer->setValue(permil);
    // File name
    mUi->lTransferName->setText(mUi->lTransferName->fontMetrics()
                                .elidedText(data->mFilename, Qt::ElideMiddle,
                                            mUi->wName->contentsRect().width() - 12));
    // Set values according to transfer state
    switch (state)
    {
        case TransferData::TransferState::TRANSFER_ACTIVE:
        {
            switch (type)
            {
                case TransferData::TransferType::TRANSFER_DOWNLOAD:
                case TransferData::TransferType::TRANSFER_LTCPDOWNLOAD:
                {
                    statusString = tr("Downloading");
                    break;
                }
                case TransferData::TransferType::TRANSFER_UPLOAD:
                {
                    statusString = tr("Uploading");
                    break;
                }
            }
            // Override speed if http speed is lower
            long long httpSpeed (data->mMegaApi->getCurrentSpeed(type >> 2));
            timeString = (httpSpeed == 0 || data->mSpeed == 0) ?
                             timeString
                           : Utilities::getTimeString(data->mRemainingTime);
            speedString = Utilities::getSizeString(std::min(data->mSpeed, httpSpeed))
                          + QLatin1Literal("/s");
            icon = Utilities::getCachedPixmap(QLatin1Literal(
                                                  ":images/lists_pause_ico.png"));
            pauseResumeTooltip = tr("Pause transfer");
            cancelClearTooltip = tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            break;
        }
        case TransferData::TransferState::TRANSFER_PAUSED:
        {
            icon = Utilities::getCachedPixmap(QLatin1Literal(
                                                  ":images/lists_resume_ico.png"));
            pauseResumeTooltip = tr("Resume transfer");
            cancelClearTooltip = tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pPaused);
            mIsPaused = true;
            break;
        }
        case TransferData::TransferState::TRANSFER_QUEUED:
        {
            icon = Utilities::getCachedPixmap(QLatin1Literal(
                                                  ":images/lists_pause_ico.png"));
            pauseResumeTooltip = tr("Pause transfer");
            cancelClearTooltip = tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pQueued);
            break;
        }
        case TransferData::TransferState::TRANSFER_CANCELLED:
        {
            statusString = tr("Canceled");
            cancelClearTooltip = tr("Clear transfer");
            timeString = QDateTime::fromSecsSinceEpoch(data->mFinishedTime)
                         .toString(QLatin1Literal("hh:mm"));
            speedString = Utilities::getSizeString(data->mMeanSpeed) + QLatin1Literal("/s");
            showTPauseResume = false;
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            mIsFinished = true;
            break;
        }
        case TransferData::TransferState::TRANSFER_COMPLETING:
        {
            statusString = tr("Completing");
            speedString = Utilities::getSizeString(data->mMeanSpeed) + QLatin1Literal("/s");
            showTPauseResume = false;
            showTCancelClear = false;
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            break;
        }
        case TransferData::TransferState::TRANSFER_FAILED:
        {
            mUi->sStatus->setCurrentWidget(mUi->pFailed);
            cancelClearTooltip = tr("Clear transfer");
            timeString = QDateTime::fromSecsSinceEpoch(data->mFinishedTime)
                         .toString(QLatin1Literal("hh:mm"));
            speedString = Utilities::getSizeString(data->mMeanSpeed) + QLatin1Literal("/s");
            showTPauseResume = false;
            mIsFinished = true;
            mUi->tItemRetry->setToolTip(tr(MegaError::getErrorString(data->mErrorCode)));
            break;
        }
        case TransferData::TransferState::TRANSFER_RETRYING:
        {
            statusString = tr("Retrying");
            icon = Utilities::getCachedPixmap(QLatin1Literal(
                                                  ":images/lists_pause_ico.png"));
            pauseResumeTooltip = tr("Pause transfer");
            cancelClearTooltip = tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            break;
        }
        case TransferData::TransferState::TRANSFER_COMPLETED:
        {
            statusString = tr("Completed");
            cancelClearTooltip = tr("Clear transfer");
            showTPauseResume = false;
            speedString = Utilities::getSizeString(data->mMeanSpeed) + QLatin1Literal("/s");
            timeString = QDateTime::fromSecsSinceEpoch(data->mFinishedTime)
                         .toString(QLatin1String("hh:mm"));
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            mIsFinished = true;
            break;
        }
    }

    // Override status in case of overquota
    if (data->mErrorCode == MegaError::API_EOVERQUOTA
            && (state & (TransferData::TransferState::TRANSFER_QUEUED
                | TransferData::TransferState::TRANSFER_RETRYING)))
    {
        QString retryMsg (data->mErrorValue ? tr("Out of transfer quota")
                                            : tr("Out of storage space"));
        mUi->lRetryMsg->setText(retryMsg);
        mUi->sStatus->setCurrentWidget(mUi->pRetry);
    }

    // Status string
    mUi->lItemStatus->setText(statusString);
    // Speed
    mUi->bItemSpeed->setText(speedString);
    // Remaining time
    mUi->lItemTime->setText(timeString);
    // Pause/Resume button
    if (showTPauseResume)
    {
        mUi->tPauseResumeTransfer->setIcon(icon);
        mUi->tPauseResumeTransfer->setToolTip(pauseResumeTooltip);
    }
    mUi->tPauseResumeTransfer->setVisible(showTPauseResume);

    // Cancel/Clear Button
    if (showTCancelClear)
    {
        mUi->tCancelClearTransfer->setToolTip(cancelClearTooltip);
    }
    mUi->tCancelClearTransfer->setVisible(showTCancelClear);
}

void TransferManagerItem2::on_tPauseResumeTransfer_clicked()
{
    emit pauseResumeTransfer(mRow, !mIsPaused);
}

void TransferManagerItem2::on_tCancelClearTransfer_clicked()
{
    emit cancelClearTransfer(mRow);
}

void TransferManagerItem2::forwardMouseEvent(QMouseEvent *me)
{
    auto w (childAt(me->pos() - pos()));
    if (w)
    {
        auto t (qobject_cast<QToolButton*>(w));
        if (t)
        {
            t->click();
        }
    }
}

void TransferManagerItem2::on_tItemRetry_clicked()
{
    emit retryTransfer(mTransferTag);
}
