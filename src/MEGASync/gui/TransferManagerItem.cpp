#include "TransferManagerItem.h"
#include "ui_TransferManagerItem.h"
#include <QMouseEvent>
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"

using namespace mega;

TransferManagerItem::TransferManagerItem(QWidget *parent) :
    TransferItem(parent),
    mUi(new Ui::TransferManagerItem)
{
    mUi->setupUi(this);
    update();
}

void TransferManagerItem::setFileName(QString fileName)
{
    TransferItem::setFileName(fileName);

    mUi->lTransferNameCompleted->ensurePolished();
    mUi->lTransferNameCompleted->setText(mUi->lTransferNameCompleted->fontMetrics()
                                         .elidedText(fileName, Qt::ElideMiddle,
                                                     mUi->lTransferNameCompleted->width()));
    mUi->lTransferNameCompleted->setToolTip(fileName);

    mUi->lTransferName->ensurePolished();
    mUi->lTransferName->setText(mUi->lTransferName->fontMetrics()
                                .elidedText(fileName, Qt::ElideMiddle,
                                            mUi->lTransferName->width()));
    mUi->lTransferName->setToolTip(fileName);

    QIcon icon = Utilities::getExtensionPixmapSmall(fileName);
    mUi->tFileType->setIcon(icon);
    mUi->lFileTypeCompleted->setIcon(icon);
    mUi->lFileTypeCompleted->setIconSize(QSize(24, 24));
}

void TransferManagerItem::setStateLabel(QString labelState)
{
    mUi->lStatus->setText(labelState);
    mUi->lTime->setText(QString());
}

QString TransferManagerItem::getTransferName()
{
    if (mUi->sTransferState->currentWidget() == mUi->stateCompleted)
    {
        return mUi->lTransferNameCompleted->text();
    }
    else
    {
        return mUi->lTransferName->text();
    }
}

void TransferManagerItem::setType(int type, bool isSyncTransfer)
{
    TransferItem::setType(type, isSyncTransfer);
    QIcon icon;

    qreal ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;

    if (isSyncTransfer)
    {
        mLoadIconResource = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/sync_item_ico.png")
                                                   : QString::fromUtf8(":/images/sync_item_ico@2x.png"));
    }
    else
    {
        mLoadIconResource = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_item_ico.png")
                                                   : QString::fromUtf8(":/images/cloud_item_ico@2x.png"));
    }
    mUi->lActionTypeCompleted->setPixmap(mLoadIconResource);

    switch (type)
    {
        case MegaTransfer::TYPE_UPLOAD:
        {
                       icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/upload_item_ico.png"));
//            mUi->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
//                                                            "QProgressBar#pbTransfer::chunk {background-color: #2ba6de;}"));
            break;
        }
        case MegaTransfer::TYPE_DOWNLOAD:
        {


            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/download_item_ico.png"));
//            mUi->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
//                                                            "QProgressBar#pbTransfer::chunk {background-color: #31b500;}"));
            break;
        }
        default:
            break;
    }

    mUi->bSpeed->setIcon(icon);
    mUi->lTransferTypeCompleted->setIcon(icon);
    mUi->lTransferTypeCompleted->setIconSize(QSize(12, 12));
}


TransferManagerItem::~TransferManagerItem()
{
    delete mUi;
}

void TransferManagerItem::setTransferState(int value)
{
    TransferItem::setTransferState(value);
    switch (mTransferState)
    {
        case MegaTransfer::STATE_COMPLETED:
        case MegaTransfer::STATE_FAILED:
            finishTransfer();
            break;
        case MegaTransfer::STATE_CANCELLED:
            break;
        default:
            updateTransfer();
            break;
    }
}

void TransferManagerItem::finishTransfer()
{
    mUi->lCompleted->setIconSize(QSize(12, 12));
    mUi->sTransferState->setCurrentWidget(mUi->stateCompleted);
    if (mTransferError < 0)
    {
        mUi->lCompleted->setIcon(QIcon(QString::fromUtf8(":/images/import_error_ico.png")));
        updateFinishedIco(true);
    }
    else
    {
        mUi->lCompleted->setIcon(QIcon(QString::fromUtf8(":/images/completed_item_ico.png")));
        updateFinishedIco(false);
    }

    mUi->lTotalCompleted->setText(Utilities::getSizeString(mTotalSize));
}

void TransferManagerItem::updateTransfer()
{
    mUi->sTransferState->setCurrentWidget(mUi->stateActive);
    switch (mTransferState)
    {
        case MegaTransfer::STATE_ACTIVE:
        {
            // Update remaining time
            long long remainingBytes = mTotalSize - mTotalTransferredBytes;
            const auto totalRemainingSeconds = mTransferRemainigTime.calculateRemainingTimeSeconds(mTransferSpeed, remainingBytes);

            QString remainingTime;
            const bool printableValue{totalRemainingSeconds.count() && totalRemainingSeconds < std::chrono::seconds::max()};
            if (printableValue)
            {
                remainingTime = Utilities::getTimeString(totalRemainingSeconds.count());
            }
            mUi->lTime->setText(remainingTime);

            // Update current transfer speed
            QString downloadString;

            if (!mTotalTransferredBytes)
            {
                downloadString = tr("starting");
            }
            else
            {
                QString pattern(QString::fromUtf8("%1/s"));
                downloadString = pattern.arg(Utilities::getSizeString(mTransferSpeed));
            }

            mUi->bSpeed->setText(downloadString);
            mUi->lQueued->hide();

            break;
        }
        case MegaTransfer::STATE_PAUSED:
        {
            mUi->lStatus->setText(tr("Paused"));
            mUi->bSpeed->setText(QString());
            mUi->lTime->setText(QString());
            break;
        }
        case MegaTransfer::STATE_QUEUED:
        {
            mUi->lTime->setText(QString());
            mUi->lQueued->show();
            break;
        }
        case MegaTransfer::STATE_RETRYING:
        {
            if (mTransferError == MegaError::API_EOVERQUOTA)
            {
                if (mTransferErrorValue)
                {
                    mUi->bSpeed->setText(QString::fromUtf8("(%1)").arg(tr("Transfer quota exceeded")));
                }
                else
                {
                    mUi->bSpeed->setText(QString::fromUtf8("(%1)").arg(tr("Out of storage space")));
                }
            }
            else
            {
                mUi->bSpeed->setText(QString::fromUtf8("(%1)").arg(tr("retrying")));
            }

            mUi->lTime->setText(QString());
            break;
        }
        case MegaTransfer::STATE_COMPLETING:
        {
            mUi->lStatus->setText(tr("completing"));
            mUi->lTime->setText(QString());
            break;
        }
        default:
        {
            mUi->bSpeed->setText(QString());
            mUi->lTime->setText(QString());
            break;
        }
    }

    // Update progress bar
    unsigned int permil = (mTotalSize > 0) ? ((1000 * mTotalTransferredBytes) / mTotalSize) : 0;
    mUi->pbTransfer->setValue(permil);

    if (mTotalTransferredBytes)
    {
        mUi->lTotal->setText(QLatin1Literal("/") + Utilities::getSizeString(mTotalSize));
        mUi->lDone->setText(Utilities::getSizeString(mTotalTransferredBytes));
    }
    else
    {
        mUi->lTotal->setText(Utilities::getSizeString(mTotalSize));
    }
}

void TransferManagerItem::updateFinishedTime()
{
    if (!mDsFinishedTime)
    {
        return;
    }

    Preferences *preferences = Preferences::instance();
    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = ( now.toMSecsSinceEpoch() / 100 - (preferences->getMsDiffTimeWithSDK() + mDsFinishedTime) ) / 10;
    mUi->lRemainingTimeCompleted->setText(Utilities::getFinishedTimeString(secs));
}

bool TransferManagerItem::cancelButtonClicked(QPoint pos)
{
    if (!mCancelButtonEnabled)
    {
        return false;
    }

    switch (mTransferState)
    {
        case MegaTransfer::STATE_COMPLETED:
        case MegaTransfer::STATE_FAILED:
        {
            if (mUi->lCancelTransferCompleted->rect().contains(mUi->lCancelTransferCompleted->mapFrom(this, pos)))
            {
                return true;
            }
            break;
        }
        case MegaTransfer::STATE_CANCELLED:
            break;
        default:
        {
            if (mUi->tCancelClearTransfer->rect().contains(mUi->tCancelClearTransfer->mapFrom(this, pos)))
            {
                return true;
            }
            break;
        }
    }

    return false;
}

bool TransferManagerItem::mouseHoverRetryingLabel(QPoint pos)
{
    switch (mTransferState)
    {
        case MegaTransfer::STATE_RETRYING:
            if (mUi->bSpeed->rect().contains(mUi->bSpeed->mapFrom(this, pos)))
            {
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

void TransferManagerItem::mouseHoverTransfer(bool isHover, const QPoint &pos)
{
    if (isHover)
    {
        if (mIsSyncTransfer && !(mTransferState == MegaTransfer::STATE_COMPLETED
                                 || mTransferSpeed == MegaTransfer::STATE_FAILED))
        {
            mUi->tCancelClearTransfer->installEventFilter(this);
            mUi->tCancelClearTransfer->update();
            return;
        }

        mCancelButtonEnabled = true;
        mUi->tCancelClearTransfer->removeEventFilter(this);
        mUi->tCancelClearTransfer->update();
        mUi->tCancelClearTransfer->removeEventFilter(this);
        mUi->tCancelClearTransfer->update();
    }
    else
    {
        mCancelButtonEnabled = false;
        mUi->tCancelClearTransfer->installEventFilter(this);
        mUi->tCancelClearTransfer->update();
        mUi->tCancelClearTransfer->installEventFilter(this);
        mUi->tCancelClearTransfer->update();
    }

    emit refreshTransfer(this->getTransferTag());
}

void TransferManagerItem::updateFinishedIco(bool transferErrors)
{
    QIcon icon;

    switch (mType)
    {
        case MegaTransfer::TYPE_UPLOAD:
            icon = Utilities::getCachedPixmap(transferErrors ? QString::fromUtf8(":/images/upload_fail_item_ico.png")
                                                             : QString::fromUtf8(":/images/uploaded_item_ico.png"));
            break;
        case MegaTransfer::TYPE_DOWNLOAD:
            icon = Utilities::getCachedPixmap(transferErrors ? QString::fromUtf8(":/images/download_fail_item_ico.png")
                                                             : QString::fromUtf8(":/images/downloaded_item_ico.png"));
            break;
        default:
            break;
    }

    mUi->lTransferTypeCompleted->setIcon(icon);
    mUi->lTransferTypeCompleted->setIconSize(mUi->lTransferTypeCompleted->size());
}

bool TransferManagerItem::eventFilter(QObject *, QEvent *ev)
{
    return ev->type() == QEvent::Paint || ev->type() == QEvent::ToolTip;
}

void TransferManagerItem::frameChanged(int)
{
    emit refreshTransfer(this->getTransferTag());
}

QSize TransferManagerItem::minimumSizeHint() const
{
    return QSize(800, 48);
}
QSize TransferManagerItem::sizeHint() const
{
    return QSize(800, 48);
}
