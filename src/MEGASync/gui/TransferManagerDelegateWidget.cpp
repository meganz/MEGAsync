#include "TransferManagerDelegateWidget.h"
#include "ui_TransferManagerDelegateWidget.h"
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"

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

            timeString = getData()->mSpeed == 0 ?
                             timeString
                           : Utilities::getTimeString(getData()->mRemainingTime);

            if(getData()->mTotalSize == getData()->mTransferredBytes
                    ||getData()->mTransferredBytes == 0 && getData()->mSpeed == 0)
            {
                speedString = QLatin1Literal("...");
            }
            else
            {
                speedString = Utilities::getSizeString(getData()->mSpeed)
                        + QLatin1Literal("/s");
            }
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

                if(getData()->mErrorCode == MegaError::API_EOVERQUOTA)
                {
                    QString retryMsg (getData()->mErrorValue ? tr("Out of transfer quota")
                                                        : tr("Out of storage space"));
                    mUi->lRetryMsg->setText(retryMsg);
                    mUi->sStatus->setCurrentWidget(mUi->pRetry);
                }
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
            timeString = QDateTime::fromSecsSinceEpoch(getData()->getFinishedTime())
                         .toString(QLatin1Literal("hh:mm"));
            speedString = Utilities::getSizeString(getData()->mMeanSpeed) + QLatin1Literal("/s");
            break;
        }
        case TransferData::TRANSFER_COMPLETING:
        {
            if(stateHasChanged())
            {
                statusString = tr("Completing");
                speedString = QLatin1Literal("...");
                showTPauseResume = false;
                showTCancelClear = false;
                mUi->sStatus->setCurrentWidget(mUi->pActive);
                mLastPauseResuemtTransferIconName.clear();
            }
            break;
        }
        case TransferData::TRANSFER_FAILED:
        {
            if(stateHasChanged())
            {
                mLastPauseResuemtTransferIconName.clear();
                mUi->sStatus->setCurrentWidget(mUi->pFailed);
                mUi->tItemRetry->setVisible(!getData()->mTemporaryError);
                cancelClearTooltip = tr("Clear transfer");
                mUi->lItemFailed->setToolTip(tr(MegaError::getErrorString(getData()->mErrorCode)));
                showTPauseResume = false;
            }


            speedString = QLatin1Literal("...");

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
            speedString = Utilities::getSizeString(getData()->mMeanSpeed == 0
                                                   ? getData()->mTotalSize : getData()->mMeanSpeed) + QLatin1Literal("/s");
            timeString = Utilities::getFinishedTimeString(getData()->getFinishedTime());
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

        mUi->lDone->setVisible(!(getData()->mState & TransferData::FINISHED_STATES_MASK));
    }

    // Total size
    mUi->lTotal->setText(Utilities::getSizeString(getData()->mTotalSize));
    // File name
    auto transferedB (getData()->mTransferredBytes);
    auto totalB (getData()->mTotalSize);
    mUi->lDone->setText(Utilities::getSizeString(transferedB) + QLatin1Literal(" / "));
    // Progress bar
    int permil = getData()->mState & (TransferData::TRANSFER_COMPLETED | TransferData::TRANSFER_COMPLETING) ?
                     PB_PRECISION
                   : totalB > 0 ? Utilities::partPer(transferedB, totalB, PB_PRECISION)
                                : 0;
    mUi->pbTransfer->setValue(permil);

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

    // File name
    QString localPath = getData()->path();
    mUi->lTransferName->setToolTip(localPath.isEmpty() ? getData()->mFilename : localPath);
    mUi->lTransferName->setText(mUi->lTransferName->fontMetrics()
                                .elidedText(getData()->mFilename, Qt::ElideMiddle,
                                            mUi->wName->contentsRect().width() - 12));
}

void TransferManagerDelegateWidget::setType()
{
    QIcon icon;

    auto transferType = getData()->mType;

    if(transferType & TransferData::TRANSFER_SYNC)
    {
        icon = Utilities::getCachedPixmap(QLatin1Literal(":/images/synching_ico.png"));
    }
    else if(transferType & TransferData::TRANSFER_DOWNLOAD || transferType & TransferData::TRANSFER_LTCPDOWNLOAD)
    {
        icon = Utilities::getCachedPixmap(QLatin1Literal(":/images/arrow_download_ico.png"));
    }
    else if(transferType & TransferData::TRANSFER_UPLOAD)
    {
        icon = Utilities::getCachedPixmap(QLatin1Literal(":/images/arrow_upload_ico.png"));
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
        bool inRetry = isMouseHoverInAction(mUi->tItemRetry, pos);

        if(getData())
        {
            auto hoverPauseResume = getData()->mState == TransferData::TransferState::TRANSFER_PAUSED ? QString::fromAscii("://images/lists_resume_hover_ico.png") :
                                                                                                    QString::fromAscii("://images/lists_pause_hover_ico.png");
            update |= setActionTransferIcon(mUi->tPauseResumeTransfer, inPauseResume ? hoverPauseResume
                                                                : mLastPauseResuemtTransferIconName);
        }

        if(inCancelClear || inPauseResume || inRetry)
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

void TransferManagerDelegateWidget::render(QPainter *painter, const QRegion &sourceRegion)
{
    TransferBaseDelegateWidget::render(painter, sourceRegion);
}

void TransferManagerDelegateWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit openTransfer();

    TransferBaseDelegateWidget::mouseDoubleClickEvent(event);
}

void TransferManagerDelegateWidget::on_tPauseResumeTransfer_clicked()
{
    emit pauseResumeTransfer();
}

void TransferManagerDelegateWidget::on_tCancelClearTransfer_clicked()
{
    QPointer<TransferManagerDelegateWidget> dialog = QPointer<TransferManagerDelegateWidget>(this);

    bool isClear = getData()->isFinished();
    auto message = tr("Are you sure you want to %1 this transfer?")
            .arg(isClear ? tr("clear") : tr("cancel"));

    if (QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                             message,
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    emit cancelClearTransfer(isClear);
}

void TransferManagerDelegateWidget::on_tItemRetry_clicked()
{
    QPointer<TransferManagerDelegateWidget> dialog = QPointer<TransferManagerDelegateWidget>(this);

    auto message = tr("Are you sure you want to retry this transfer?");

    if (QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                             message,
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    emit retryTransfer();
}
