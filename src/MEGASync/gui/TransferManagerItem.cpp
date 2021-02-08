#include "TransferManagerItem.h"
#include "ui_TransferManagerItem.h"
#include <QMouseEvent>
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"

using namespace mega;

TransferManagerItem::TransferManagerItem(QWidget *parent) :
    TransferItem(parent),
    mUi(new Ui::TransferManagerItem),
    mAnimation(nullptr)
{
    mUi->setupUi(this);

    // Choose the right icon for initial load (hdpi/normal displays)
    qreal ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
    mLoadIconResource = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_item_ico.png")
                                         : QString::fromUtf8(":/images/cloud_item_ico@2x.png"));
    mUi->lActionType->setPixmap(mLoadIconResource);

    mUi->lCancelTransfer->installEventFilter(this);
    mUi->lCancelTransferCompleted->installEventFilter(this);
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
    mUi->lFileType->setIcon(icon);
    mUi->lFileType->setIconSize(QSize(24, 24));
    mUi->lFileTypeCompleted->setIcon(icon);
    mUi->lFileTypeCompleted->setIconSize(QSize(24, 24));
}

void TransferManagerItem::setStateLabel(QString labelState)
{
    mUi->lSpeed->setText(QString::fromUtf8("(%1)").arg(labelState));
    mUi->lRemainingTime->setText(QString());
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
        delete mAnimation;
        mAnimation = new QMovie(ratio < 2 ? QString::fromUtf8(":/images/synching.gif")
                                         : QString::fromUtf8(":/images/synching@2x.gif"));
        connect(mAnimation, SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));
    }
    else
    {
        mLoadIconResource = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_item_ico.png")
                                                   : QString::fromUtf8(":/images/cloud_item_ico@2x.png"));
    }
    mUi->lActionTypeCompleted->setPixmap(mLoadIconResource);
    mUi->lActionType->setPixmap(mLoadIconResource);

    switch (type)
    {
        case MegaTransfer::TYPE_UPLOAD:
        {
            if (!isSyncTransfer)
            {
                delete mAnimation;
                mAnimation = new QMovie(ratio < 2 ? QString::fromUtf8(":/images/uploading.gif")
                                                 : QString::fromUtf8(":/images/uploading@2x.gif"));
                connect(mAnimation, SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));
            }

            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/upload_item_ico.png"));
            mUi->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
                                                            "QProgressBar#pbTransfer::chunk {background-color: #2ba6de;}"));
            break;
        }
        case MegaTransfer::TYPE_DOWNLOAD:
        {
            if (!isSyncTransfer)
            {
                delete mAnimation;
                mAnimation = new QMovie(ratio < 2 ? QString::fromUtf8(":/images/downloading.gif")
                                                 : QString::fromUtf8(":/images/downloading@2x.gif"));
                connect(mAnimation, SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));
            }

            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/download_item_ico.png"));
            mUi->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
                                                            "QProgressBar#pbTransfer::chunk {background-color: #31b500;}"));
            break;
        }
        default:
            break;
    }

    mUi->lTransferType->setIcon(icon);
    mUi->lTransferType->setIconSize(QSize(12, 12));
    mUi->lTransferTypeCompleted->setIcon(icon);
    mUi->lTransferTypeCompleted->setIconSize(QSize(12, 12));
}


TransferManagerItem::~TransferManagerItem()
{
    delete mUi;
    delete mAnimation;
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
            mUi->lRemainingTime->setText(remainingTime);

            // Update current transfer speed
            QString downloadString;

            if (!mTotalTransferredBytes)
            {
                downloadString = QString::fromUtf8("(%1)").arg(tr("starting"));
            }
            else
            {
                QString pattern(QString::fromUtf8("(%1/s)"));
                downloadString = pattern.arg(Utilities::getSizeString(mTransferSpeed));
            }

            mUi->lSpeed->setText(downloadString);
            break;
        }
        case MegaTransfer::STATE_PAUSED:
        {
            mUi->lSpeed->setText(QString::fromUtf8("(%1)").arg(tr("paused")));
            mUi->lRemainingTime->setText(QString());
            break;
        }
        case MegaTransfer::STATE_QUEUED:
        {
            mUi->lSpeed->setText(QString::fromUtf8("(%1)").arg(tr("queued")));
            mUi->lRemainingTime->setText(QString());
            break;
        }
        case MegaTransfer::STATE_RETRYING:
        {
            if (mTransferError == MegaError::API_EOVERQUOTA)
            {
                if (mTransferErrorValue)
                {
                    mUi->lSpeed->setText(QString::fromUtf8("(%1)").arg(tr("Transfer quota exceeded")));
                }
                else
                {
                    mUi->lSpeed->setText(QString::fromUtf8("(%1)").arg(tr("Out of storage space")));
                }
            }
            else
            {
                mUi->lSpeed->setText(QString::fromUtf8("(%1)").arg(tr("retrying")));
            }

            mUi->lRemainingTime->setText(QString());
            break;
        }
        case MegaTransfer::STATE_COMPLETING:
        {
            mUi->lSpeed->setText(QString::fromUtf8("(%1)").arg(tr("completing")));
            mUi->lRemainingTime->setText(QString());
            break;
        }
        default:
        {
            mUi->lSpeed->setText(QString::fromUtf8(""));
            mUi->lRemainingTime->setText(QString());
            break;
        }
    }

    // Update progress bar
    unsigned int permil = (mTotalSize > 0) ? ((1000 * mTotalTransferredBytes) / mTotalSize) : 0;
    mUi->pbTransfer->setValue(permil);

    // Update transferred bytes
    const QString totalBytesText{Utilities::getSizeString(mTotalSize)};
    const QString totalBytesStyled{QStringLiteral("<span style=\"color:#333333;"
                                                    "text-decoration:none;"
                                                    "\">%1</span>").arg(totalBytesText)};
    if (mTotalTransferredBytes)
    {
        const QString totalTransferredBytesText{Utilities::getSizeString(mTotalTransferredBytes)};
        const QString totalTransferredBytesStyled{QStringLiteral("<span style=\"color:#333333;"
                                                                  "text-decoration:none;"
                                                                  "\">%1</span>").arg(totalTransferredBytesText)};
        QString transferredBytesText{tr("%1 of %2")};
        transferredBytesText.replace(QStringLiteral("%1"), totalTransferredBytesStyled);
        transferredBytesText.replace(QStringLiteral("%2"), totalBytesStyled);
        mUi->lTotal->setText(transferredBytesText);
    }
    else
    {
        mUi->lTotal->setText(totalBytesStyled);
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
            if (mUi->lCancelTransfer->rect().contains(mUi->lCancelTransfer->mapFrom(this, pos)))
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
            if (mUi->lSpeed->rect().contains(mUi->lSpeed->mapFrom(this, pos)))
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
            mUi->lCancelTransfer->installEventFilter(this);
            mUi->lCancelTransfer->update();
            return;
        }

        mCancelButtonEnabled = true;
        mUi->lCancelTransfer->removeEventFilter(this);
        mUi->lCancelTransfer->update();
        mUi->lCancelTransferCompleted->removeEventFilter(this);
        mUi->lCancelTransferCompleted->update();
    }
    else
    {
        mCancelButtonEnabled = false;
        mUi->lCancelTransfer->installEventFilter(this);
        mUi->lCancelTransfer->update();
        mUi->lCancelTransferCompleted->installEventFilter(this);
        mUi->lCancelTransferCompleted->update();
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

void TransferManagerItem::loadDefaultTransferIcon()
{
    if (mAnimation && mAnimation->state() != QMovie::NotRunning)
    {
        mAnimation->stop();
        mUi->lActionType->setMovie(nullptr);
        mUi->lActionType->setPixmap(mLoadIconResource);
    }
}

bool TransferManagerItem::eventFilter(QObject *, QEvent *ev)
{
    return ev->type() == QEvent::Paint || ev->type() == QEvent::ToolTip;
}

void TransferManagerItem::frameChanged(int)
{
    emit refreshTransfer(this->getTransferTag());
}

void TransferManagerItem::updateAnimation()
{
    if (!mAnimation)
    {
        return;
    }

    switch (mTransferState)
    {
        case MegaTransfer::STATE_ACTIVE:
            if (mAnimation->state() != QMovie::Running)
            {
                mUi->lActionType->setMovie(mAnimation);
                mAnimation->start();
            }
            break;
        default:
            loadDefaultTransferIcon();
            break;
    }
}

QSize TransferManagerItem::minimumSizeHint() const
{
    return QSize(800, 48);
}
QSize TransferManagerItem::sizeHint() const
{
    return QSize(800, 48);
}
