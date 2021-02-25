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
    mIsPaused(false)
{ 
    mUi->setupUi(this);
}

void TransferManagerItem2::updateUi(const TransferItem2& transferItem)
{
    auto d (transferItem.getTransferData());
    QString statusString;

    mMegaApi = d->mMegaApi;
    mTransferTag = d->mTag;
    mIsPaused = false;

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

    QString remTimeString;
    QString speedString;
    bool isQueued (false);
    QIcon pauseResumeIcon;
    QString pauseResumeTooltip;

    switch (d->mState)
    {
        case MegaTransfer::STATE_ACTIVE:
        {
            remTimeString = Utilities::getTimeString(d->mRemainingTime);
            speedString = Utilities::getSizeString(d->mSpeed) + QLatin1Literal("/s");
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_pause_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Pause transfer");
            break;
        }
        case MegaTransfer::STATE_PAUSED:
        {
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_resume_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Resume transfer");
            mIsPaused = true;
            break;
        }
        case MegaTransfer::STATE_QUEUED:
        {
            isQueued = true;
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_pause_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Pause transfer");
            break;
        }
        case MegaTransfer::STATE_CANCELLED:
        {
            statusString = QObject::tr("Canceled");
            break;
        }
        case MegaTransfer::STATE_COMPLETING:
        {
            statusString = QObject::tr("Completing");
            break;
        }
        case MegaTransfer::STATE_FAILED:
        {
            statusString = QObject::tr("Failed");
            break;
        }
        case MegaTransfer::STATE_RETRYING:
        {
            statusString = QObject::tr("Retrying");
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_pause_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Pause transfer");
            break;
        }
        case MegaTransfer::STATE_COMPLETED:
        {
            statusString = QObject::tr("Completed");
            break;
        }
    }

    // Queued string
    mUi->lQueued->setVisible(isQueued);

    // Paused string
    mUi->lPaused->setVisible(mIsPaused);

    // Status
    mUi->lStatus->setVisible(!(isQueued || mIsPaused));
    mUi->lStatus->setText(statusString);

    // Progress bar
    int permil = (d->mTotalSize > 0) ?
                   ((1000 *d->mTransferredBytes) / d->mTotalSize)
                   : 0;
    mUi->pbTransfer->setValue(permil);

    // Remaining time
    mUi->lRemainingTime->setText(remTimeString);

    // Speed
    mUi->bSpeed->setText(speedString);

    // Pause/resume button
    mUi->tPauseTransfer->setIcon(pauseResumeIcon);
    mUi->tPauseTransfer->setToolTip(pauseResumeTooltip);

    // Cancel Button
    mUi->tCancelTransfer->setToolTip(tr("Cancel transfer"));

    update();
}

void TransferManagerItem2::on_tPauseTransfer_clicked()
{
    mMegaApi->pauseTransferByTag(mTransferTag, !mIsPaused);
}

void TransferManagerItem2::on_tCancelTransfer_clicked()
{
    mMegaApi->cancelTransferByTag(mTransferTag);
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
