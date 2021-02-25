#include "TransferManagerItem2.h"
#include "ui_TransferManagerItem.h"
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"

#include <QMouseEvent>

using namespace mega;

TransferManagerItem2::TransferManagerItem2(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::TransferManagerItem),
    mMegaApi(nullptr),
    mTransferTag(0),
    mIsPaused(false),
    mIsFinished(false),
    mRow(0)
{
    mUi->setupUi(this);
}

void TransferManagerItem2::updateUi(const TransferItem2& transferItem, const int row)
{
    auto d (transferItem.getTransferData());
    QString statusString;

    mRow = row;
    mMegaApi = d->mMegaApi;
    mTransferTag = d->mTag;

    // Set fixed stuff
    QIcon icon (Utilities::getCachedPixmap(
                     Utilities::getExtensionPixmapName(d->mFilename, QLatin1Literal(":/images/small_"))));

    mUi->tFileType->setIcon(icon);
    mUi->lTotal->setText(Utilities::getSizeString(d->mTotalSize));


    switch (d->mType)
    {
        case MegaTransfer::TYPE_DOWNLOAD:
        case MegaTransfer::TYPE_LOCAL_TCP_DOWNLOAD:
        {
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/download_item_ico.png"));
            statusString = QObject::tr("Downloading");
            break;
        }
        case MegaTransfer::TYPE_UPLOAD:
        {
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/upload_item_ico.png"));
            statusString = QObject::tr("Uploading");
            break;
        }
    }
    mUi->bSpeed->setIcon(icon);

    // File name
    mUi->lTransferName->setText(mUi->lTransferName->fontMetrics()
                                .elidedText(d->mFilename, Qt::ElideMiddle,
                                            mUi->wName->width()-24));
    mUi->lTransferName->setToolTip(d->mFilename);

    // Amount transfered
    mUi->lDone->setText(Utilities::getSizeString(d->mTransferredBytes));

    QString timeString;
    QString speedString;
    QIcon pauseResumeIcon;
    QString pauseResumeTooltip;
    QString cancelClearTooltip;

    // Update paused state
    mIsPaused = false;
    mIsFinished = false;

    bool showTPauseResume(true);
    bool showTCancelClear(true);

    switch (d->mState)
    {
        case MegaTransfer::STATE_ACTIVE:
        {
            timeString = Utilities::getTimeString(d->mRemainingTime);
            speedString = Utilities::getSizeString(d->mSpeed) + QLatin1Literal("/s");
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_pause_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Pause transfer");
            cancelClearTooltip = QObject::tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            mUi->tPauseResumeTransfer->show();
            break;
        }
        case MegaTransfer::STATE_PAUSED:
        {
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_resume_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Resume transfer");
            cancelClearTooltip = QObject::tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pPaused);
            mUi->tPauseResumeTransfer->show();
            mIsPaused = true;
            break;
        }
        case MegaTransfer::STATE_QUEUED:
        {
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_pause_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Pause transfer");
            cancelClearTooltip = QObject::tr("Cancel transfer");
            mUi->tPauseResumeTransfer->show();
            mUi->sStatus->setCurrentWidget(mUi->pQueued);
            break;
        }
        case MegaTransfer::STATE_CANCELLED:
        {
            statusString = QObject::tr("Canceled");
            cancelClearTooltip = QObject::tr("Clear transfer");
            mUi->tPauseResumeTransfer->hide();
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            mIsFinished = true;
            break;
        }
        case MegaTransfer::STATE_COMPLETING:
        {
            statusString = QObject::tr("Completing");
            mUi->tPauseResumeTransfer->hide();
            mUi->tCancelClearTransfer->hide();
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            break;
        }
        case MegaTransfer::STATE_FAILED:
        {
            mUi->sStatus->setCurrentWidget(mUi->pFailed);
            cancelClearTooltip = QObject::tr("Cancel transfer");
            break;
        }
        case MegaTransfer::STATE_RETRYING:
        {
            statusString = QObject::tr("Retrying");
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_pause_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Pause transfer");
            cancelClearTooltip = QObject::tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            break;
        }
        case MegaTransfer::STATE_COMPLETED:
        {
            statusString = QObject::tr("Completed");
            cancelClearTooltip = QObject::tr("Clear transfer");
            mUi->tPauseResumeTransfer->hide();
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            mIsFinished = true;
            break;
        }
    }

    // Status string
    mUi->lStatus->setText(statusString);

    // Progress bar
    int permil = (d->mTotalSize > 0) ?
                   ((1000 *d->mTransferredBytes) / d->mTotalSize)
                   : 0;
    mUi->pbTransfer->setValue(permil);

    // Remaining time
    mUi->lTime->setText(timeString);

    // Speed
    mUi->bSpeed->setText(speedString);

    // Pause/resume button
    mUi->tPauseResumeTransfer->setIcon(pauseResumeIcon);
    mUi->tPauseResumeTransfer->setToolTip(pauseResumeTooltip);
    mUi->tPauseResumeTransfer->setVisible(showTPauseResume);

    // Cancel Button
    mUi->tCancelClearTransfer->setToolTip(cancelClearTooltip);
    mUi->tCancelClearTransfer->setVisible(showTCancelClear);

    update();
}

void TransferManagerItem2::on_tPauseResumeTransfer_clicked()
{
    mMegaApi->pauseTransferByTag(mTransferTag, !mIsPaused);
}

void TransferManagerItem2::on_tCancelClearTransfer_clicked()
{
    if (mIsFinished)
    {
        // Clear
        emit clearTransfer(mRow);
    }
    else
    {
        // Cancel
        mMegaApi->cancelTransferByTag(mTransferTag);
    }
}

void TransferManagerItem2::forwardMouseEvent(QMouseEvent *me)
{
    auto w (childAt(me->pos() - pos()));

    if (w)
    {
        if (qobject_cast<QToolButton*>(w))
        {
            static_cast<QToolButton*>(w)->click();
        }
    }
}
