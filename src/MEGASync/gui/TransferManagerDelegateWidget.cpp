#include "TransferManagerDelegateWidget.h"
#include "ui_TransferManagerDelegateWidget.h"
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"
#include "MegaApplication.h"

#include <QMouseEvent>

constexpr uint PB_PRECISION = 1000;

using namespace mega;

TransferManagerDelegateWidget::TransferManagerDelegateWidget(QWidget *parent) :
    TransferBaseDelegateWidget (parent),
    mUi (new Ui::TransferManagerDelegateWidget)
{
    mUi->setupUi(this);
    mUi->pbTransfer->setMaximum(PB_PRECISION);
}

TransferManagerDelegateWidget::~TransferManagerDelegateWidget()
{
    delete mUi;
}

void TransferManagerDelegateWidget::updateTransferState()
{
    QIcon icon;
    QString speedString;
    QString pauseResumeTooltip;
    QString cancelClearTooltip;
    bool showTPauseResume(true);
    bool showTCancelClear(true);
    QString timeString;
    QString statusString;

    // Set values according to transfer state
    switch (getData()->mState)
    {
        case TransferData::TRANSFER_ACTIVE:
        {
            if(stateHasChanged())
            {
                switch (getData()->mType)
                {
                    case TransferData::TRANSFER_DOWNLOAD:
                    case TransferData::TRANSFER_LTCPDOWNLOAD:
                    {
                        statusString = tr("Downloading");
                        break;
                    }
                    case TransferData::TRANSFER_UPLOAD:
                    {
                        statusString = tr("Uploading");
                        break;
                    }
                    default:
                    {
                        statusString = tr("Syncing");
                        break;
                    }
                }
                mLastPauseResuemtTransferIconName = QLatin1Literal(":images/lists_pause_ico.png");
                pauseResumeTooltip = tr("Pause transfer");
                cancelClearTooltip = tr("Cancel transfer");
                mUi->sStatus->setCurrentWidget(mUi->pActive);
            }

            // Override speed if http speed is lower
            auto httpSpeed (static_cast<unsigned long long>(getData()->mMegaApi->getCurrentSpeed((getData()->mType & TransferData::TYPE_MASK) >> 1)));
            timeString = (httpSpeed == 0 || getData()->mSpeed == 0) ?
                             timeString
                           : Utilities::getTimeString(getData()->mRemainingTime);
            speedString = Utilities::getSizeString(std::min(getData()->mSpeed, httpSpeed))
                          + QLatin1Literal("/s");
            break;
        }
        case TransferData::TRANSFER_PAUSED:
        {
            if(stateHasChanged())
            {
                mLastPauseResuemtTransferIconName = QLatin1Literal(":images/lists_resume_ico.png");
                pauseResumeTooltip = tr("Resume transfer");
                cancelClearTooltip = tr("Cancel transfer");
                mUi->sStatus->setCurrentWidget(mUi->pPaused);
            }
            break;
        }
        case TransferData::TRANSFER_QUEUED:
        {
            if(stateHasChanged())
            {
                mLastPauseResuemtTransferIconName = QLatin1Literal(":images/lists_pause_ico.png");
                pauseResumeTooltip = tr("Pause transfer");
                cancelClearTooltip = tr("Cancel transfer");
                mUi->sStatus->setCurrentWidget(mUi->pQueued);
            }

            if(getData()->mErrorCode == MegaError::API_EOVERQUOTA)
            {
                QString retryMsg (getData()->mErrorValue ? tr("Out of transfer quota")
                                                    : tr("Out of storage space"));
                mUi->lRetryMsg->setText(retryMsg);
                mUi->sStatus->setCurrentWidget(mUi->pRetry);
            }

            break;
        }
        case TransferData::TRANSFER_CANCELLED:
        {
            if(stateHasChanged())
            {
                mUi->sStatus->setCurrentWidget(mUi->pActive);
                statusString = tr("Canceled");
                cancelClearTooltip = tr("Clear transfer");
                showTPauseResume = false;
                mLastPauseResuemtTransferIconName.clear();
            }
            timeString = QDateTime::fromSecsSinceEpoch(getData()->mFinishedTime)
                         .toString(QLatin1Literal("hh:mm"));
            speedString = Utilities::getSizeString(getData()->mMeanSpeed) + QLatin1Literal("/s");
            break;
        }
        case TransferData::TRANSFER_COMPLETING:
        {
            if(stateHasChanged())
            {
                statusString = tr("Completing");
                showTPauseResume = false;
                showTCancelClear = false;
                mUi->sStatus->setCurrentWidget(mUi->pActive);
                mLastPauseResuemtTransferIconName.clear();
            }
            speedString = Utilities::getSizeString(getData()->mMeanSpeed) + QLatin1Literal("/s");
            break;
        }
        case TransferData::TRANSFER_FAILED:
        {
            if(stateHasChanged())
            {
                timeString = QDateTime::fromSecsSinceEpoch(getData()->mFinishedTime)
                             .toString(QLatin1Literal("hh:mm"));
                speedString = Utilities::getSizeString(getData()->mMeanSpeed) + QLatin1Literal("/s");
                mLastPauseResuemtTransferIconName.clear();
            }

            mUi->sStatus->setCurrentWidget(mUi->pFailed);
            cancelClearTooltip = tr("Clear transfer");
            showTPauseResume = false;
            mUi->tItemRetry->setToolTip(tr(MegaError::getErrorString(getData()->mErrorCode)));
            break;
        }
        case TransferData::TRANSFER_RETRYING:
        {
            if(stateHasChanged())
            {
                statusString = tr("Retrying");
                mLastPauseResuemtTransferIconName = QLatin1Literal(":images/lists_pause_ico.png");
                pauseResumeTooltip = tr("Pause transfer");
                cancelClearTooltip = tr("Cancel transfer");
                mUi->lItemStatus->setToolTip(tr(MegaError::getErrorString(getData()->mErrorCode)));
                mUi->sStatus->setCurrentWidget(mUi->pActive);
            }

            if(getData()->mErrorCode == MegaError::API_EOVERQUOTA)
            {
                QString retryMsg (getData()->mErrorValue ? tr("Out of transfer quota")
                                                    : tr("Out of storage space"));
                mUi->lRetryMsg->setText(retryMsg);
                mUi->sStatus->setCurrentWidget(mUi->pRetry);
            }
            break;
        }
        case TransferData::TRANSFER_COMPLETED:
        {
            if(stateHasChanged())
            {
                statusString = tr("Completed");
                cancelClearTooltip = tr("Clear transfer");
                showTPauseResume = false;
                mLastPauseResuemtTransferIconName.clear();

                mUi->sStatus->setCurrentWidget(mUi->pActive);
            }
            speedString = Utilities::getSizeString(getData()->mMeanSpeed) + QLatin1Literal("/s");
            timeString = QDateTime::fromSecsSinceEpoch(getData()->mFinishedTime)
                         .toString(QLatin1String("hh:mm"));
            break;
        }
    }

    if(stateHasChanged())
    {
        // Status string
        mUi->lItemStatus->setText(statusString);
        mUi->lItemStatus->setToolTip(statusString);

        // Pause/Resume button
        if (showTPauseResume)
        {
            icon = Utilities::getCachedPixmap(mLastPauseResuemtTransferIconName);
            mUi->tPauseResumeTransfer->setIcon(icon);
            mUi->tPauseResumeTransfer->setToolTip(pauseResumeTooltip);
        }
        mUi->tPauseResumeTransfer->setVisible(showTPauseResume);

        // Cancel/Clear Button
        if ((getData()->mType & TransferData::TRANSFER_SYNC)
                && !(getData()->mState & (TransferData::TRANSFER_FAILED | TransferData::TRANSFER_COMPLETED)))
        {
            showTCancelClear = false;
        }
        if (showTCancelClear)
        {
            mUi->tCancelClearTransfer->setToolTip(cancelClearTooltip);
        }
        mUi->tCancelClearTransfer->setVisible(showTCancelClear);
    }

    // Speed
    mUi->bItemSpeed->setText(speedString);
    // Remaining time
    mUi->lItemTime->setText(timeString);
}

void TransferManagerDelegateWidget::setFileNameAndType()
{
    // Update members
    QIcon icon;
    // File type icon
    icon = Utilities::getCachedPixmap(Utilities::getExtensionPixmapName(
                                          getData()->mFilename, QLatin1Literal(":/images/drag_")));
    mUi->tFileType->setIcon(icon);
    // Total size
    mUi->lTotal->setText(QLatin1Literal("/ ") + Utilities::getSizeString(getData()->mTotalSize));
    // File name
    auto transferedB (getData()->mTransferredBytes);
    auto totalB (getData()->mTotalSize);
    mUi->lDone->setText(Utilities::getSizeString(transferedB));
    // Progress bar
    int permil = getData()->mState & (TransferData::TRANSFER_COMPLETED | TransferData::TRANSFER_COMPLETING) ?
                     PB_PRECISION
                   : totalB > 0 ? Utilities::partPer(transferedB, totalB, PB_PRECISION)
                                : 0;
    mUi->pbTransfer->setValue(permil);

    // File name
    mUi->lTransferName->setToolTip(getData()->mFilename);
    mUi->lTransferName->setText(mUi->lTransferName->fontMetrics()
                                .elidedText(getData()->mFilename, Qt::ElideMiddle,
                                            mUi->wName->contentsRect().width() - 12));
}

void TransferManagerDelegateWidget::setType()
{
    QIcon icon;

    // Transfer type icon
    switch (getData()->mType)
    {
        case TransferData::TRANSFER_DOWNLOAD:
        case TransferData::TRANSFER_LTCPDOWNLOAD:
        {
            icon = Utilities::getCachedPixmap(QLatin1Literal(":/images/arrow_download_ico.png"));
            break;
        }
        case TransferData::TRANSFER_UPLOAD:
        {
            icon = Utilities::getCachedPixmap(QLatin1Literal(":/images/arrow_upload_ico.png"));
            break;
        }
        default:
        {
            icon = Utilities::getCachedPixmap(QLatin1Literal(":/images/synching_ico.png"));
            break;
        }
    }
    mUi->bItemSpeed->setIcon(icon);
}

TransferBaseDelegateWidget::ActionHoverType TransferManagerDelegateWidget::mouseHoverTransfer(bool isHover, const QPoint &pos)
{
    bool update(false);
    ActionHoverType hoverType(ActionHoverType::NONE);

    if(!getData())
    {
        return hoverType;
    }

    if (isHover)
    {
        bool inCancelClear = isMouseHoverInAction(mUi->tCancelClearTransfer, pos);
        update = setActionTransferIcon(mUi->tCancelClearTransfer, inCancelClear ? QString::fromAscii("://images/lists_cancel_hover_ico.png")
                                                            : QString::fromAscii("://images/lists_cancel_ico.png"));

        bool inPauseResume = isMouseHoverInAction(mUi->tPauseResumeTransfer, pos);

        if(getData())
        {
            auto hoverPauseResume = getData()->mState == TransferData::TransferState::TRANSFER_PAUSED ? QString::fromAscii("://images/lists_resume_hover_ico.png") :
                                                                                                    QString::fromAscii("://images/lists_pause_hover_ico.png");
            update |= setActionTransferIcon(mUi->tPauseResumeTransfer, inPauseResume ? hoverPauseResume
                                                                : mLastPauseResuemtTransferIconName);
        }

        if(inCancelClear || inPauseResume)
        {
            hoverType = ActionHoverType::HOVER_ENTER;
        }
        else if(update)
        {
            hoverType = ActionHoverType::HOVER_LEAVE;
        }
    }
    else
    {
        update = setActionTransferIcon(mUi->tCancelClearTransfer, QString::fromAscii("://images/lists_cancel_ico.png"));
        update |= setActionTransferIcon(mUi->tPauseResumeTransfer, mLastPauseResuemtTransferIconName);

        if(update)
        {
            hoverType = ActionHoverType::HOVER_LEAVE;
        }
    }

    return hoverType;
}

void TransferManagerDelegateWidget::on_tPauseResumeTransfer_clicked()
{
    emit pauseResumeTransfer();
}

void TransferManagerDelegateWidget::on_tCancelClearTransfer_clicked()
{
    emit cancelClearTransfer();
}

void TransferManagerDelegateWidget::on_tItemRetry_clicked()
{
    emit retryTransfer();
}
