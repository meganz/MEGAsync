#include "TransferManagerItem2.h"
#include "ui_TransferManagerItem.h"
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"
#include "MegaApplication.h"

#include <QMouseEvent>

using namespace mega;

TransferManagerItem2::TransferManagerItem2(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::TransferManagerItem),
    mPreferences(Preferences::instance()),
    mMegaApi(nullptr),
    mTransferTag(0),
    mIsPaused(false),
    mIsFinished(false),
    mRow(0)
{
    mUi->setupUi(this);
    mAreDlPaused = mPreferences->getDownloadsPaused();
    mAreUlPaused = mPreferences->getUploadsPaused();

    // Connect to pause state change signal
    QObject::connect((MegaApplication *)qApp, &MegaApplication::pauseStateChanged,
                      this, &TransferManagerItem2::onPauseStateChanged);
}

void TransferManagerItem2::updateUi(QExplicitlySharedDataPointer<TransferData> data, const int row)
{
    //auto d (transferItem->getTransferData());
    QString statusString;

    mRow = row;
    mMegaApi = data->mMegaApi;
    mTransferTag = data->mTag;

    // Set fixed stuff
    QIcon icon (Utilities::getCachedPixmap(
                     Utilities::getExtensionPixmapName(data->mFilename, QLatin1Literal(":/images/small_"))));

    mUi->tFileType->setIcon(icon);
    mUi->lTotal->setText(Utilities::getSizeString(data->mTotalSize));

    bool isGlobalPaused(false);

    switch (data->mType)
    {
        case MegaTransfer::TYPE_DOWNLOAD:
        case MegaTransfer::TYPE_LOCAL_TCP_DOWNLOAD:
        {
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/download_item_ico.png"));
            statusString = QObject::tr("Downloading");
            isGlobalPaused = mAreDlPaused;
            break;
        }
        case MegaTransfer::TYPE_UPLOAD:
        {
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/upload_item_ico.png"));
            statusString = QObject::tr("Uploading");
            isGlobalPaused = mAreUlPaused;
            break;
        }
    }
    mUi->bSpeed->setIcon(icon);

    // File name
    mUi->lTransferName->setText(mUi->lTransferName->fontMetrics()
                                .elidedText(data->mFilename, Qt::ElideMiddle,
                                            mUi->wName->width()-24));
    mUi->lTransferName->setToolTip(data->mFilename);

    // Amount transfered
    mUi->lDone->setText(Utilities::getSizeString(data->mTransferredBytes));

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

    switch (data->mState)
    {
        case MegaTransfer::STATE_ACTIVE:
        {
            timeString = Utilities::getTimeString(data->mRemainingTime);
            speedString = Utilities::getSizeString(data->mSpeed) + QLatin1Literal("/s");
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_pause_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Pause transfer");
            cancelClearTooltip = QObject::tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            break;
        }
        case MegaTransfer::STATE_PAUSED:
        {
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_resume_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Resume transfer");
            cancelClearTooltip = QObject::tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pPaused);
            mIsPaused = true;
            break;
        }
        case MegaTransfer::STATE_QUEUED:
        {
            pauseResumeIcon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_pause_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Pause transfer");
            cancelClearTooltip = QObject::tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pQueued);
            break;
        }
        case MegaTransfer::STATE_CANCELLED:
        {
            statusString = QObject::tr("Canceled");
            cancelClearTooltip = QObject::tr("Clear transfer");
            showTPauseResume = false;
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            mIsFinished = true;
            break;
        }
        case MegaTransfer::STATE_COMPLETING:
        {
            statusString = QObject::tr("Completing");
            showTPauseResume = false;
            showTCancelClear = false;
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
            showTPauseResume = false;
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            mIsFinished = true;
            break;
        }
    }

    if (isGlobalPaused)
    {
        switch (data->mState)
        {
            case MegaTransfer::STATE_ACTIVE:
            case MegaTransfer::STATE_QUEUED:
            case MegaTransfer::STATE_RETRYING:
            {
                mUi->sStatus->setCurrentWidget(mUi->pPaused);
                speedString = QString();
                timeString = QString();
            }
            default:
            {
                showTPauseResume = false;
                break;
            }
        }
    }

    // Status string
    mUi->lStatus->setText(statusString);

    // Progress bar
    int permil = (data->mTotalSize > 0) ?
                   ((1000 * data->mTransferredBytes) / data->mTotalSize)
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
        emit clearTransfers(mRow, 1);
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

void TransferManagerItem2::onPauseStateChanged()
{
    mAreDlPaused = mPreferences->getDownloadsPaused();
    mAreUlPaused = mPreferences->getUploadsPaused();
}
