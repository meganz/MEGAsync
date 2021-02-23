#include "TransferManagerItem2.h"
#include "ui_TransferManagerItem.h"
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"

#include <QMouseEvent>

using namespace mega;

TransferManagerItem2::TransferManagerItem2(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::TransferManagerItem)
{ 
    mUi->setupUi(this);
}

void TransferManagerItem2::updateUi(const TransferItem2& transferItem)
{
    auto d (transferItem.getTransferData());
    QString statusString;

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
            break;
        }
        case MegaTransfer::TYPE_UPLOAD:
        {
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/upload_item_ico.png"));
            break;
        }
    }

    mUi->bSpeed->setIcon(icon);

    switch (d->mType)
    {
        case MegaTransfer::TYPE_DOWNLOAD:
        case MegaTransfer::TYPE_LOCAL_TCP_DOWNLOAD:
        {
            statusString = QObject::tr("Downloading");
            break;
        }
        case MegaTransfer::TYPE_UPLOAD:
        {
            statusString = QObject::tr("Uploading");
            break;
        }
    }

    // File name
    mUi->lTransferName->setText(mUi->lTransferName->fontMetrics()
                                .elidedText(d->mFilename, Qt::ElideMiddle,
                                            mUi->lTransferName->width()));
    mUi->lTransferName->setToolTip(d->mFilename);

    // Amount transfered
    mUi->lDone->setText(Utilities::getSizeString(d->mTransferredBytes));

    QString remTimeString;
    QString speedString;
    bool isQueued (false);
    QIcon pauseResumeIcon;

    switch (d->mState)
    {
        case MegaTransfer::STATE_ACTIVE:
        {
            remTimeString = Utilities::getTimeString(d->mRemainingTime);
            speedString = Utilities::getSizeString(d->mSpeed) + QLatin1Literal("/s");
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/ico_pause_transfers_state.png"));
            break;
        }
        case MegaTransfer::STATE_PAUSED:
        {
            statusString = QObject::tr("Paused");
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/ico_resume_transfers_state.png"));
            break;
        }
        case MegaTransfer::STATE_QUEUED:
        {
            isQueued = true;
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/ico_pause_transfers_state.png"));
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
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/ico_pause_transfers_state.png"));
            break;
        }
        case MegaTransfer::STATE_COMPLETED:
        {
            statusString = QObject::tr("Completed");
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":/ico_pause_transfers_state.png"));
            break;
        }
    }

    // Queued state
    mUi->lQueued->setVisible(isQueued);

    // Status
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

    update();
}
