#include "TransferManagerItem2.h"
#include "ui_TransferManagerItem.h"
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"

#include <QMouseEvent>

using namespace mega;

TransferManagerItem2::TransferManagerItem2()
{ 
    switch (d->mType)
    {
        case MegaTransfer::TYPE_DOWNLOAD:
        case MegaTransfer::TYPE_LOCAL_TCP_DOWNLOAD:
        {
            mActiveStatus = QObject::tr("Downloading");
            break;
        }
        case MegaTransfer::TYPE_UPLOAD:
        {
            mActiveStatus = QObject::tr("Uploading");
            break;
        }
    }
}

void TransferManagerItem2::updateUi(Ui::TransferManagerItem* ui) const
{
    // Display changing values

    // File name
    ui->lTransferName->setText(ui->lTransferName->fontMetrics()
                                .elidedText(d->mFilename, Qt::ElideMiddle,
                                            ui->lTransferName->width()));

    // Amount transfered
    ui->lDone->setText(Utilities::getSizeString(d->mTransferredBytes));

    QString remTimeString;
    QString speedString;
    QString statusString;
    bool isQueued (false);
    QIcon pauseResumeIcon;

    static int prevTransferState(MegaTransfer::STATE_NONE);

    if (d->mState != prevTransferState)
    {
        switch (d->mState) {
            case MegaTransfer::STATE_ACTIVE:
            {
                remTimeString = Utilities::getTimeString(d->mRemainingTime);
                speedString = Utilities::getSizeString(d->mSpeed) + QLatin1Literal("/s");
                statusString = mActiveStatus;
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
        }

        // Queued state
        ui->lQueued->setVisible(isQueued);

        // Status
        ui->lStatus->setText(statusString);

        // Progress bar
        int permil = (d->mTotalSize > 0) ?
                       ((1000 *d->mTransferredBytes) / d->mTotalSize)
                       : 0;
        ui->pbTransfer->setValue(permil);

        prevTransferState = d->mState;
    }

    // Remaining time
    ui->lRemainingTime->setText(remTimeString);

    // Speed
    ui->bSpeed->setText(speedString);
}

void TransferManagerItem2::setupUi(Ui::TransferManagerItem* ui, QWidget* view) const
{
    ui->setupUi(view);
    // Set fixed stuff
    ui->tFileType->setIcon(QIcon());
    ui->lTransferName->setToolTip(d->mFilename);
    ui->lTotal->setText(Utilities::getSizeString(d->mTotalSize));

    QIcon icon;

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

    ui->bSpeed->setIcon(icon);
}
