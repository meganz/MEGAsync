#include "TransferItem2.h"
#include "megaapi.h"
#include "Utilities.h"

using namespace mega;

TransferItem2::TransferItem2() : d(new TransferDataRow()) {}
TransferItem2::TransferItem2(const TransferItem2& ti)  : d(new TransferDataRow(ti.d.constData())) {}
TransferItem2::TransferItem2(const TransferDataRow& dataRow) : d(new TransferDataRow(dataRow)) {}

void TransferItem2::updateValuesTransferFinished(uint64_t updateTime,
                                  int errorCode, long long errorValue,
                                  long long meanSpeed,
                                  int state, long long transferedBytes)
{
   d->mErrorCode = errorCode;
   d->mState = state;
   d->mErrorValue = errorValue;
   d->mFinishedTime = updateTime;
   d->mRemainingTime = 0;
   d->mSpeed = 0;
   d->mMeanSpeed = meanSpeed;
   d->mTransferredBytes = transferedBytes;
   d->mUpdateTime = updateTime;
}

void TransferItem2::updateValuesTransferUpdated(uint64_t updateTime,
                                 int errorCode, long long errorValue,
                                 long long meanSpeed,
                                 long long speed,
                                 unsigned long long priority,
                                 int state, long long transferedBytes)
{
    d->mErrorCode = errorCode;
    d->mState = state;
    d->mErrorValue = errorValue;
    d->mRemainingTime = 50; // Use real value
    d->mSpeed = speed;
    d->mMeanSpeed = meanSpeed;
    d->mTransferredBytes = transferedBytes;
    d->mUpdateTime = updateTime;
    d->mPriority = priority;
}

void TransferItem2::updateUi(Ui::TransferManagerItem* ui) const
{
    QString statusString;
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

    // Display changing values

    // File name
    ui->lTransferName->setText(ui->lTransferName->fontMetrics()
                                .elidedText(d->mFilename, Qt::ElideMiddle,
                                            ui->lTransferName->width()));

    // Amount transfered
    ui->lDone->setText(Utilities::getSizeString(d->mTransferredBytes));

    QString remTimeString;
    QString speedString;
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

void TransferItem2::setupUi(Ui::TransferManagerItem* ui, QWidget* view) const
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
