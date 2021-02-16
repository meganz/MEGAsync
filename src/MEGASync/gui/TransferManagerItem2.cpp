#include "TransferManagerItem2.h"
#include "ui_TransferManagerItem.h"
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"

#include <QMouseEvent>

using namespace mega;

TransferManagerItem2::TransferManagerItem2(TransferDataRow& transferData, QWidget *parent) :
    TransferItem2(transferData, parent),
    mUi(new Ui::TransferManagerItem)
{
    mUi->setupUi(this);

    // Set fixed stuff
    mUi->tFileType->setIcon(QIcon());
    mUi->lTransferName->setToolTip(mTransferData.mFilename);
    mUi->lTotal->setText(Utilities::getSizeString(mTransferData.mTotalSize));


    QIcon icon;

    switch (mTransferData.mType)
    {
        case MegaTransfer::TYPE_DOWNLOAD:
        case MegaTransfer::TYPE_LOCAL_TCP_DOWNLOAD:
        {
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/download_item_ico.png"));
            mActiveStatus = tr("Downloading");
            break;
        }
        case MegaTransfer::TYPE_UPLOAD:
        {
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/upload_item_ico.png"));
            mActiveStatus = tr("Uploading");
            break;
        }
    }

    mUi->bSpeed->setIcon(icon);

    update();
}

TransferManagerItem2::~TransferManagerItem2()
{
    delete mUi;
}

void TransferManagerItem2::paint(QPainter *painter, const QRect &rect) const
{
    // Display changing values

    // File name
    mUi->lTransferName->setText(mUi->lTransferName->fontMetrics()
                                .elidedText(mTransferData.mFilename, Qt::ElideMiddle,
                                            mUi->lTransferName->width()));

    // Amount transfered
    mUi->lDone->setText(Utilities::getSizeString(mTransferData.mTransferredBytes));

    QString remTimeString;
    QString speedString;
    QString statusString;

    bool isQueued (false);

    switch (mTransferData.mState) {
        case MegaTransfer::STATE_ACTIVE:
        {
            remTimeString = Utilities::getTimeString(mTransferData.mRemainingTime);
            speedString = Utilities::getSizeString(mTransferData.mSpeed) + QLatin1Literal("/s");
            statusString = mActiveStatus;
            break;
        }
        case MegaTransfer::STATE_PAUSED:
        {
            statusString = tr("Paused");
            break;
        }
        case MegaTransfer::STATE_QUEUED:
        {
            isQueued = true;
            break;
        }
        case MegaTransfer::STATE_CANCELLED:
        {
            statusString = tr("Canceled");
            break;
        }
        case MegaTransfer::STATE_COMPLETING:
        {
            statusString = tr("Completing");
            break;
        }
        case MegaTransfer::STATE_FAILED:
        {
            statusString = tr("Failed");
            break;
        }
        case MegaTransfer::STATE_RETRYING:
        {
            statusString = tr("Retrying");
            break;
        }
    }

    // Remaining time
    mUi->lRemainingTime->setText(remTimeString);

    // Speed
    mUi->bSpeed->setText(speedString);

    // Queued state
    mUi->lQueued->setVisible(isQueued);

    // Status
    mUi->lStatus->setText(statusString);

    // Progress bar
    int permil = (mTotalSize > 0) ? ((1000 * mTotalTransferredBytes) / mTotalSize) : 0;
    mUi->pbTransfer->setValue(permil);
}

void TransferManagerItem2::on_tCancelTransfer_clicked()
{
    emit transferCanceled(mTransferData.mTag);
}

void TransferManagerItem2::on_tPauseTransfer_clicked()
{
    emit transferPaused(mTransferData.mTag);
}
