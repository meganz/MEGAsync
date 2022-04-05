#include "TransferManagerDelegateWidget.h"
#include "ui_TransferManagerDelegateWidget.h"

#include "MegaTransferView.h"
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"
#include "MegaApplication.h"
#include "QMegaMessageBox.h"

#include <QMouseEvent>

constexpr uint PB_PRECISION = 1000;
const QColor HOVER_COLOR = QColor("#FAFAFA");
const QColor SELECTED_BORDER_COLOR = QColor("#E9E9E9");

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

    auto state = getData()->mState;
    //state = TransferData::TRANSFER_QUEUED;

    // Set values according to transfer state
    switch (state)
    {
        case TransferData::TRANSFER_ACTIVE:
        {
            if(stateHasChanged())
            {
                if (!getData()->mTransferredBytes)
                {
                    statusString = QString::fromUtf8("%1%2").arg(tr("starting"), QString::fromUtf8("…"));
                }
                else
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
                }
                mLastPauseResuemtTransferIconName = QLatin1Literal(":images/lists_pause_ico.png");
                pauseResumeTooltip = tr("Pause transfer");
                cancelClearTooltip = tr("Cancel transfer");
                mUi->wProgressBar->setVisible(true);
                mUi->sStatus->setCurrentWidget(mUi->pActive);
            }

            timeString = getData()->mSpeed == 0 ?
                             timeString
                           : Utilities::getTimeString(getData()->mRemainingTime);

            if(getData()->mTotalSize == getData()->mTransferredBytes)
            {
                speedString = QString::fromUtf8("…");
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
                mUi->wProgressBar->setVisible(true);

                if(getData()->mTransferredBytes != 0)
                {
                    mUi->sStatus->setCurrentWidget(mUi->pPausedQueued);
                }
                else
                {
                    mUi->sStatus->setCurrentWidget(mUi->pPaused);
                }
            }

            speedString = QString::fromUtf8("…");

            break;
        }
        case TransferData::TRANSFER_QUEUED:
        {
            if(stateHasChanged())
            {
                mLastPauseResuemtTransferIconName = QLatin1Literal(":images/lists_pause_ico.png");
                pauseResumeTooltip = tr("Pause transfer");
                cancelClearTooltip = tr("Cancel transfer");
                mUi->wProgressBar->setVisible(true);
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
            //Cancelled transfers are immediately removed from the model
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

            speedString = QString::fromUtf8("…");

            break;
        }
        case TransferData::TRANSFER_FAILED:
        {
            if(stateHasChanged())
            {
                mLastPauseResuemtTransferIconName.clear();
                mUi->sStatus->setCurrentWidget(mUi->pFailed);
                mUi->tItemRetry->setVisible(!getData()->mTemporaryError);
                mUi->wProgressBar->setVisible(false);
                cancelClearTooltip = tr("Clear transfer");
                mUi->lItemFailed->setToolTip(tr(MegaError::getErrorString(getData()->mErrorCode)));
                showTPauseResume = false;
            }

            timeString = getData()->getFormattedFinishedTime();
            speedString = QString::fromUtf8("…");

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
                mUi->wProgressBar->setVisible(false);
                mLastPauseResuemtTransferIconName.clear();

                mUi->sStatus->setCurrentWidget(mUi->pActive);
            }
            speedString = Utilities::getSizeString(getData()->mSpeed) + QLatin1Literal("/s");
            timeString = getData()->getFormattedFinishedTime();
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

    // Done label
    auto transferedB (getData()->mTransferredBytes);
    auto totalB (getData()->mTotalSize);
    mUi->lDone->setText(Utilities::getSizeStringWithoutUnits(transferedB) + QLatin1Literal("/"));

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

void TransferManagerDelegateWidget::render(const QStyleOptionViewItem &option, QPainter *painter, const QRegion &sourceRegion)
{
    bool isDragging(false);

    auto view = dynamic_cast<MegaTransferView*>(parent());
    if(view)
    {
        isDragging = view->state() == MegaTransferView::DraggingState;
    }

    if(option.state & (QStyle::State_MouseOver | QStyle::State_Selected))
    {
        QPainterPath path;
        path.addRoundedRect(QRectF(12.0,
                                   4.0,
                                   option.rect.width() - 20.0,
                                   option.rect.height() - 7.0),
                            10, 10);

        QPen pen;

        if(option.state & QStyle::State_MouseOver && option.state & QStyle::State_Selected)
        {
            pen.setColor(SELECTED_BORDER_COLOR);
            pen.setWidth(2);
            painter->fillPath(path, HOVER_COLOR);
        }
        else
        {
            if(option.state & QStyle::State_MouseOver)
            {
                pen.setColor(HOVER_COLOR);
                painter->fillPath(path, HOVER_COLOR);
            }
            else if (option.state & QStyle::State_Selected)
            {
                auto painterWidget = dynamic_cast<QWidget*>(painter->device());

                if(isDragging && painterWidget)
                {
                    painter->setOpacity(0.25);
                }
                else
                {
                    pen.setColor(SELECTED_BORDER_COLOR);
                    pen.setWidth(2);
                    painter->fillPath(path, Qt::white);
                }
            }
        }

        if(pen != QPen())
        {
            painter->setPen(pen);
            painter->drawPath(path);
        }
    }

    TransferBaseDelegateWidget::render(option, painter, sourceRegion);
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
    emit cancelClearTransfer(getData()->isFinished());
}

void TransferManagerDelegateWidget::on_tItemRetry_clicked()
{
    emit retryTransfer();
}
