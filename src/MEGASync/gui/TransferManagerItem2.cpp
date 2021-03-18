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
    mTransferTag(-1),
    mIsPaused(false),
    mIsFinished(false),
    mRow(0)
{
    mUi->setupUi(this);
    mAreDlPaused = mPreferences->getDownloadsPaused();
    mAreUlPaused = mPreferences->getUploadsPaused();

    // Connect to pause state change signal
    connect(qobject_cast<MegaApplication*>(qApp), &MegaApplication::pauseStateChanged,
            this, &TransferManagerItem2::onPauseStateChanged);
}

TransferManagerItem2::~TransferManagerItem2()
{
    delete mUi;
}

void TransferManagerItem2::updateUi(QExplicitlySharedDataPointer<TransferData> data, const int row)
{
    // Update members
    mRow = row;
    mMegaApi = data->mMegaApi;
    mIsPaused = false;
    mIsFinished = false;

    QIcon icon;

    if (mTransferTag != data->mTag)
    {
        mTransferTag = data->mTag;

        // File type icon
        icon = Utilities::getCachedPixmap(
                        Utilities::getExtensionPixmapName(data->mFilename,
                                                          QLatin1Literal(":/images/small_")));
        mUi->tFileType->setIcon(icon);
        // Total size
        mUi->lTotal->setText(Utilities::getSizeString(data->mTotalSize));
        // File name
        mUi->lTransferName->setToolTip(data->mFilename);

        // Transfer type icon
        switch (data->mType)
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
        mUi->bItemSpeed->setIcon(icon);
    }

    QString statusString;
    bool isGlobalPaused(false);
    QString timeString;
    QString speedString;
    QString pauseResumeTooltip;
    QString cancelClearTooltip;
    bool showTPauseResume(true);
    bool showTCancelClear(true);

    // Amount transfered
    mUi->lDone->setText(Utilities::getSizeString(data->mTransferredBytes));
    // Progress bar
    int permil = data->mState == MegaTransfer::STATE_COMPLETED
                  || data->mState == MegaTransfer::STATE_COMPLETING ?
                     1000
                   : data->mTotalSize > 0 ?
                         (1000 * data->mTransferredBytes) / data->mTotalSize
                       : 0;
    mUi->pbTransfer->setValue(permil);
    // File name
    mUi->lTransferName->setText(mUi->lTransferName->fontMetrics()
                                .elidedText(data->mFilename, Qt::ElideMiddle,
                                            mUi->wName->width()-24));

    // Transfer type text
    switch (data->mType)
    {
        case MegaTransfer::TYPE_DOWNLOAD:
        case MegaTransfer::TYPE_LOCAL_TCP_DOWNLOAD:
        {
            statusString = QObject::tr("Downloading");
            isGlobalPaused = mAreDlPaused;
            break;
        }
        case MegaTransfer::TYPE_UPLOAD:
        {
            statusString = QObject::tr("Uploading");
            isGlobalPaused = mAreUlPaused;
            break;
        }
    }

    // Set values according to transfer state
    switch (data->mState)
    {
        case MegaTransfer::STATE_ACTIVE:
        {
            long long httpSpeed(data->mMegaApi->getCurrentSpeed(data->mType));
            switch (data->mType)
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
            // Override speed if http speed is lower
            timeString = (httpSpeed == 0 || data->mSpeed == 0) ?
                             timeString
                           :  Utilities::getTimeString(data->mRemainingTime);
            speedString = Utilities::getSizeString(std::min(data->mSpeed, httpSpeed))
                          + QLatin1Literal("/s");
            icon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_pause_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Pause transfer");
            cancelClearTooltip = QObject::tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            break;
        }
        case MegaTransfer::STATE_PAUSED:
        {
            icon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_resume_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Resume transfer");
            cancelClearTooltip = QObject::tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pPaused);
            mIsPaused = true;
            break;
        }
        case MegaTransfer::STATE_QUEUED:
        {
            icon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_pause_transfers_state.png"));
            pauseResumeTooltip = QObject::tr("Pause transfer");
            cancelClearTooltip = QObject::tr("Cancel transfer");
            mUi->sStatus->setCurrentWidget(mUi->pQueued);
            break;
        }
        case MegaTransfer::STATE_CANCELLED:
        {
            statusString = QObject::tr("Canceled");
            cancelClearTooltip = QObject::tr("Clear transfer");
            timeString = QDateTime::fromSecsSinceEpoch(data->mFinishedTime)
                         .toString(QLatin1String("hh:mm"));
            speedString = Utilities::getSizeString(data->mMeanSpeed) + QLatin1Literal("/s");
            showTPauseResume = false;
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            mIsFinished = true;
            break;
        }
        case MegaTransfer::STATE_COMPLETING:
        {
            statusString = QObject::tr("Completing");
            speedString = Utilities::getSizeString(data->mMeanSpeed) + QLatin1Literal("/s");
            showTPauseResume = false;
            showTCancelClear = false;
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            break;
        }
        case MegaTransfer::STATE_FAILED:
        {
            mUi->sStatus->setCurrentWidget(mUi->pFailed);
            cancelClearTooltip = QObject::tr("Clear transfer");
            timeString = QDateTime::fromSecsSinceEpoch(data->mFinishedTime)
                         .toString(QLatin1String("hh:mm"));
            speedString = Utilities::getSizeString(data->mMeanSpeed) + QLatin1Literal("/s");
            showTPauseResume = false;
            mIsFinished = true;
            mUi->tItemRetry->setToolTip(tr(MegaError::getErrorString(data->mErrorCode)));
            break;
        }
        case MegaTransfer::STATE_RETRYING:
        {
            statusString = QObject::tr("Retrying");
            icon = Utilities::getCachedPixmap(QLatin1Literal(":images/ico_pause_transfers_state.png"));
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
            speedString = Utilities::getSizeString(data->mMeanSpeed) + QLatin1Literal("/s");
            timeString = QDateTime::fromSecsSinceEpoch(data->mFinishedTime)
                         .toString(QLatin1String("hh:mm"));
//            .toString("dddd d MMMM yyyy hh:mm");
            mUi->sStatus->setCurrentWidget(mUi->pActive);
            mIsFinished = true;
            break;
        }
    }

    // Override if global/ul/dl transfers are paused
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
    mUi->lItemStatus->setText(statusString);
    // Speed
    mUi->bItemSpeed->setText(speedString);
    // Remaining time
    mUi->lItemTime->setText(timeString);
    // Pause/Resume button
    if (showTPauseResume)
    {
        mUi->tPauseResumeTransfer->setIcon(icon);
        mUi->tPauseResumeTransfer->setToolTip(pauseResumeTooltip);
    }
    mUi->tPauseResumeTransfer->setVisible(showTPauseResume);

    // Cancel/Clear Button
    if (showTCancelClear)
    {
        mUi->tCancelClearTransfer->setToolTip(cancelClearTooltip);
    }
    mUi->tCancelClearTransfer->setVisible(showTCancelClear);

    update();
}

void TransferManagerItem2::on_tPauseResumeTransfer_clicked()
{
    mMegaApi->pauseTransferByTag(mTransferTag, !mIsPaused);
}

void TransferManagerItem2::on_tCancelClearTransfer_clicked()
{
    emit cancelClearTransfers(mRow, 1);
}

void TransferManagerItem2::forwardMouseEvent(QMouseEvent *me)
{
    auto w (childAt(me->pos() - pos()));
    if (w)
    {
        auto t (qobject_cast<QToolButton*>(w));
        if (t)
        {
            t->click();
        }
    }
}

void TransferManagerItem2::on_tItemRetry_clicked()
{
    emit retryTransfer(mTransferTag);
}

void TransferManagerItem2::onPauseStateChanged()
{
    mAreDlPaused = mPreferences->getDownloadsPaused();
    mAreUlPaused = mPreferences->getUploadsPaused();
}
