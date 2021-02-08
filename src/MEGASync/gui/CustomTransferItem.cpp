#include "CustomTransferItem.h"
#include "ui_CustomTransferItem.h"
#include "MegaApplication.h"
#include "control/Utilities.h"
#include "platform/Platform.h"

#include <QImageReader>
#include <QtConcurrent/QtConcurrent>


using namespace mega;

CustomTransferItem::CustomTransferItem(QWidget *parent) :
    TransferItem(parent),
    mUi(new Ui::CustomTransferItem),
    mActionButtonsEnabled(false),
    mMgaApi(((MegaApplication *)qApp)->getMegaApi())
{
    mUi->setupUi(this);

    QSizePolicy retainGetLink = mUi->lActionTransfer->sizePolicy();
    retainGetLink.setRetainSizeWhenHidden(true);
    mUi->lActionTransfer->setSizePolicy(retainGetLink);

    QSizePolicy retainShowInFolder = mUi->lShowInFolder->sizePolicy();
    retainShowInFolder.setRetainSizeWhenHidden(true);
    mUi->lShowInFolder->setSizePolicy(retainShowInFolder);

    mUi->bClockDown->setVisible(false);

    mUi->lShowInFolder->hide();
    update();
}

CustomTransferItem::~CustomTransferItem()
{
    delete mUi;
}

void CustomTransferItem::setFileName(QString fileName)
{
    TransferItem::setFileName(fileName);
    mUi->lFileName->ensurePolished();
    mUi->lFileName->setText(mUi->lFileName->fontMetrics()
                            .elidedText(fileName, Qt::ElideMiddle, mUi->lFileName->width()));
    mUi->lFileName->setToolTip(fileName);

    mUi->lFileNameCompleted->ensurePolished();
    mUi->lFileNameCompleted->setText(mUi->lFileNameCompleted->fontMetrics()
                                     .elidedText(fileName, Qt::ElideMiddle,
                                                 mUi->lFileNameCompleted->width()));
    mUi->lFileNameCompleted->setToolTip(fileName);

    QIcon icon = Utilities::getExtensionPixmapMedium(fileName);
    mUi->lFileType->setIcon(icon);
    mUi->lFileType->setIconSize(QSize(48, 48));
    mUi->lFileTypeCompleted->setIcon(icon);
    mUi->lFileTypeCompleted->setIconSize(QSize(48, 48));
}

void CustomTransferItem::setType(int type, bool isSyncTransfer)
{
    TransferItem::setType(type, isSyncTransfer);
    QIcon icon;

    switch (type)
    {
        case MegaTransfer::TYPE_UPLOAD:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/upload_item_ico.png"));
            mUi->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: transparent;}"
                                                            "QProgressBar#pbTransfer::chunk {background-color: #2ba6de;}"));
            break;
        case MegaTransfer::TYPE_DOWNLOAD:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/download_item_ico.png"));
            mUi->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: transparent;}"
                                                            "QProgressBar#pbTransfer::chunk {background-color: #31b500;}"));
            break;
        default:
            break;
    }

    mUi->lTransferType->setIcon(icon);
    mUi->lTransferType->setIconSize(mUi->lTransferType->size());
}

void CustomTransferItem::setTransferState(int value)
{
    TransferItem::setTransferState(value);
    switch (mTransferState)
    {
    case MegaTransfer::STATE_COMPLETED:
    case MegaTransfer::STATE_FAILED:
        finishTransfer();
        break;
    case MegaTransfer::STATE_CANCELLED:
    default:
        updateTransfer();
        break;
        }
}

QString CustomTransferItem::getTransferName()
{
    return mUi->lFileName->text();
}

bool CustomTransferItem::checkIsInsideButton(QPoint pos, int button)
{
    if (!mActionButtonsEnabled)
    {
        return false;
    }

    switch (mTransferState)
    {
    case MegaTransfer::STATE_COMPLETED:
    case MegaTransfer::STATE_FAILED:
    {
        if ((button == TransferItem::ACTION_BUTTON && mUi->lActionTransfer->rect().contains(mUi->lActionTransfer->mapFrom(this, pos)))
                || (button == TransferItem::SHOW_IN_FOLDER_BUTTON && mUi->lShowInFolder->rect().contains(mUi->lShowInFolder->mapFrom(this, pos))))
        {
            return true;
        }
        break;
    }
    case MegaTransfer::STATE_CANCELLED:
    default:
        break;
    }

    return false;
}


void CustomTransferItem::setActionTransferIcon(const QString &name)
{
    if (name != mLastActionTransferIconName)
    {
        mUi->lActionTransfer->setIcon(Utilities::getCachedPixmap(name));
        mUi->lActionTransfer->setIconSize(QSize(24,24));
        mLastActionTransferIconName = name;
    }
}


void CustomTransferItem::setShowInFolderIcon(const QString &name)
{
    if (name != mLastShowInFolderIconName)
    {
        mUi->lShowInFolder->setIcon(Utilities::getCachedPixmap(name));
        mUi->lShowInFolder->setIconSize(QSize(24,24));

        mLastShowInFolderIconName = name;
    }
}

void CustomTransferItem::updateFinishedIco(int transferType, bool transferErrors)
{
    QIcon iconCompleted;

    switch (transferType)
    {
        case MegaTransfer::TYPE_UPLOAD:
            iconCompleted = Utilities::getCachedPixmap(transferErrors ? QString::fromUtf8(":/images/upload_fail_item_ico.png")
                                                                      : QString::fromUtf8(":/images/uploaded_item_ico.png"));
            break;
        case MegaTransfer::TYPE_DOWNLOAD:
            iconCompleted = Utilities::getCachedPixmap(transferErrors ? QString::fromUtf8(":/images/download_fail_item_ico.png")
                                                                      : QString::fromUtf8(":/images/downloaded_item_ico.png"));
            break;
        default:
            break;
    }

    mUi->lTransferTypeCompleted->setIcon(iconCompleted);
    mUi->lTransferTypeCompleted->setIconSize(QSize(mUi->lTransferTypeCompleted->width(), mUi->lTransferTypeCompleted->height()));
}

void CustomTransferItem::mouseHoverTransfer(bool isHover, const QPoint &pos)
{

    if (isHover)
    {
        mActionButtonsEnabled = true;
        if (mTransferError < 0)
        {
            if (!mIsSyncTransfer)
            {
                bool in = mUi->lActionTransfer->rect().contains(mUi->lActionTransfer->mapFrom(this, pos));
                setActionTransferIcon(QString::fromAscii("://images/ico_item_retry%1.png").arg(QString::fromAscii(in?"":"_greyed")));
            }
            else
            {
                setActionTransferIcon(QString::fromAscii("://images/error.png"));
                mActionButtonsEnabled = false;
            }
            mUi->lShowInFolder->hide();
        }
        else if (mIsLinkAvailable)
        {
            bool in = mUi->lActionTransfer->rect().contains(mUi->lActionTransfer->mapFrom(this, pos));
            setActionTransferIcon(QString::fromAscii("://images/ico_item_link%1.png").arg(QString::fromAscii(in?"":"_greyed")));

            in = mUi->lShowInFolder->rect().contains(mUi->lShowInFolder->mapFrom(this, pos));
            setShowInFolderIcon(QString::fromAscii("://images/showinfolder%1.png").arg(QString::fromAscii(in?"":"_greyed")));

            mUi->lShowInFolder->show();
        }
        else
        {
            bool in = mUi->lActionTransfer->rect().contains(mUi->lActionTransfer->mapFrom(this, pos));
            setActionTransferIcon(QString::fromAscii("://images/showinfolder%1.png").arg(QString::fromAscii(in?"":"_greyed")));
        }
    }
    else
    {
        mActionButtonsEnabled = false;
        if (mTransferError < 0)
        {
            setActionTransferIcon(QString::fromAscii("://images/error.png"));
            mUi->lActionTransfer->setIconSize(QSize(24,24));
            mUi->lShowInFolder->hide();
        }
        else
        {
            setActionTransferIcon(QString::fromAscii("://images/success.png"));
            mUi->lActionTransfer->setIconSize(QSize(24,24));
            mUi->lShowInFolder->hide();
        }
    }

    emit refreshTransfer(this->getTransferTag());
}

bool CustomTransferItem::mouseHoverRetryingLabel(QPoint pos)
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

void CustomTransferItem::finishTransfer()
{
    mUi->sTransferState->setCurrentWidget(mUi->completedTransfer);
    if (mTransferError < 0)
    {
        mUi->lActionTransfer->setIcon(QIcon(QString::fromAscii("://images/error.png")));
        mUi->lActionTransfer->setIconSize(QSize(24,24));
        mUi->lElapsedTime->setStyleSheet(QString::fromUtf8("color: #F0373A"));
        mUi->lElapsedTime->setText(tr("failed:") + QString::fromUtf8(" ") + QCoreApplication::translate("MegaError",
                                                                                                       MegaError::getErrorString(mTransferError,
                                                                                                                                 this->getType() == MegaTransfer::TYPE_DOWNLOAD && !mTransferFinishedWhileBlocked
                                                                                                                                 ? MegaError::API_EC_DOWNLOAD : MegaError::API_EC_DEFAULT)));
        updateFinishedIco(mType, true);
    }
    else
    {
        mUi->lActionTransfer->setIcon(QIcon(QString::fromAscii("://images/success.png")));
        mUi->lActionTransfer->setIconSize(QSize(24,24));
        updateFinishedIco(mType, false);
    }
}

void CustomTransferItem::updateTransfer()
{
    if (mTransferState == MegaTransfer::STATE_COMPLETED || mTransferState == MegaTransfer::STATE_FAILED)
    {
        mUi->sTransferState->setCurrentWidget(mUi->completedTransfer);
    }
    else
    {
        mUi->sTransferState->setCurrentWidget(mUi->activeTransfer);
    }

    switch (mTransferState)
    {
        case MegaTransfer::STATE_ACTIVE:
        {
            // Update remaining time
            long long remainingBytes = mTotalSize - mTotalTransferredBytes;
            const auto totalRemainingSeconds = mTransferRemainingTime.calculateRemainingTimeSeconds(mTransferSpeed, remainingBytes);

            QString remainingTime;
            const bool printableValue{totalRemainingSeconds.count() && totalRemainingSeconds < std::chrono::seconds::max()};
            if (printableValue)
            {
                remainingTime = Utilities::getTimeString(totalRemainingSeconds.count());
                mUi->bClockDown->setVisible(true);
            }
            else
            {
                mUi->bClockDown->setVisible(false);
            }
            mUi->lRemainingTime->setText(remainingTime);

            // Update current transfer speed
            QString downloadString;

            if (!mTotalTransferredBytes)
            {
                downloadString = QString::fromUtf8("%1").arg(tr("starting..."));
            }
            else
            {
                QString pattern(QString::fromUtf8("%1/s"));
                downloadString = pattern.arg(Utilities::getSizeString(mTransferSpeed));
            }

            mUi->lSpeed->setText(downloadString);
            break;
        }
        case MegaTransfer::STATE_PAUSED:
            mUi->lSpeed->setText(QString::fromUtf8("%1").arg(tr("PAUSED")));
            mUi->bClockDown->setVisible(false);
            mUi->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        case MegaTransfer::STATE_QUEUED:
            mUi->lSpeed->setText(QString::fromUtf8("%1").arg(tr("queued")));
            mUi->bClockDown->setVisible(false);
            mUi->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        case MegaTransfer::STATE_RETRYING:
            if (mTransferError == MegaError::API_EOVERQUOTA)
            {
                if (mTransferErrorValue)
                {
                    mUi->lSpeed->setText(QString::fromUtf8("%1").arg(tr("Transfer quota exceeded")));
                }
                else
                {
                    mUi->lSpeed->setText(QString::fromUtf8("%1").arg(tr("Out of storage space")));
                }
            }
            else
            {
                mUi->lSpeed->setText(QString::fromUtf8("%1").arg(tr("retrying...")));
            }

            mUi->bClockDown->setVisible(false);
            mUi->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        case MegaTransfer::STATE_COMPLETING:
            mUi->lSpeed->setText(QString::fromUtf8("%1").arg(tr("completing...")));
            mUi->bClockDown->setVisible(false);
            mUi->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        default:
            mUi->lSpeed->setText(QString::fromUtf8(""));
            mUi->lRemainingTime->setText(QString::fromUtf8(""));
            mUi->bClockDown->setVisible(false);
            break;
    }

    // Update progress bar
    unsigned int permil = (mTotalSize > 0) ? ((1000 * mTotalTransferredBytes) / mTotalSize) : 0;
    mUi->pbTransfer->setValue(permil);
}

void CustomTransferItem::updateFinishedTime()
{
    if (!mDsFinishedTime || mTransferError < 0)
    {
        return;
    }

    Preferences *preferences = Preferences::instance();
    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = ( now.toMSecsSinceEpoch() / 100 - (preferences->getMsDiffTimeWithSDK() + mDsFinishedTime) ) / 10;

    mUi->lElapsedTime->setStyleSheet(QString::fromUtf8("color: #999999"));
    mUi->lElapsedTime->setText(tr("Added [A]").replace(QString::fromUtf8("[A]"), Utilities::getFinishedTimeString(secs)));
}

void CustomTransferItem::setStateLabel(QString labelState)
{
    mUi->lSpeed->setText(QString::fromUtf8("%1").arg(labelState));
    mUi->lRemainingTime->setText(QString::fromUtf8(""));
    mUi->bClockDown->setVisible(false);
}

bool CustomTransferItem::eventFilter(QObject *, QEvent *ev)
{
    return ev->type() == QEvent::Paint || ev->type() == QEvent::ToolTip;
}

QSize CustomTransferItem::minimumSizeHint() const
{
    return QSize(400, 60);
}
QSize CustomTransferItem::sizeHint() const
{
    return QSize(400, 60);
}

