#include "InfoDialogTransferDelegateWidget.h"
#include "ui_InfoDialogTransferDelegateWidget.h"
#include "MegaApplication.h"
#include "control/Utilities.h"
#include "platform/Platform.h"
#include <TransferItem.h>

#include <QImageReader>
#include <QtConcurrent/QtConcurrent>


using namespace mega;

const QRect InfoDialogTransferDelegateWidget::fullRect = QRect(0,0,400,60);
const QRect InfoDialogTransferDelegateWidget::innerRect = QRect(60,10,InfoDialogTransferDelegateWidget::fullRect.width()-120,
                                                  InfoDialogTransferDelegateWidget::fullRect.height()-20);

InfoDialogTransferDelegateWidget::InfoDialogTransferDelegateWidget(QWidget *parent) :
    TransferBaseDelegateWidget(parent),
    mUi(new Ui::InfoDialogTransferDelegateWidget),
    mActionButtonsEnabled(false),
    mMegaApi(((MegaApplication *)qApp)->getMegaApi())
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
}

InfoDialogTransferDelegateWidget::~InfoDialogTransferDelegateWidget()
{
    delete mUi;
}

void InfoDialogTransferDelegateWidget::updateTransferState()
{
    if(stateHasChanged())
    {
        if (getData()->mState == TransferData::TransferState::TRANSFER_COMPLETED
                || getData()->mState == TransferData::TransferState::TRANSFER_FAILED)
        {
            mUi->sTransferState->setCurrentWidget(mUi->completedTransfer);
        }
        else
        {
            mUi->sTransferState->setCurrentWidget(mUi->activeTransfer);
        }
    }

    switch (getData()->mState)
    {
        case TransferData::TransferState::TRANSFER_COMPLETED:
        case TransferData::TransferState::TRANSFER_FAILED:
            if(stateHasChanged())
            {
                finishTransfer();
            }

            updateFinishedTime();
            break;
        case TransferData::TransferState::TRANSFER_ACTIVE:
        {
            // Update remaining time
            long long remainingBytes = getData()->mTotalSize - getData()->mTransferredBytes;
            TransferRemainingTime remainingTimeCalculator;
            const auto totalRemainingSeconds = remainingTimeCalculator.calculateRemainingTimeSeconds(getData()->mSpeed, remainingBytes);


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

            if (!getData()->mTransferredBytes)
            {
                downloadString = QString::fromUtf8("%1").arg(tr("starting..."));
            }
            else
            {
                QString pattern(QString::fromUtf8("%1/s"));
                downloadString = pattern.arg(Utilities::getSizeString(getData()->mSpeed));
            }

            mUi->lSpeed->setText(downloadString);
            break;
        }
        case TransferData::TransferState::TRANSFER_PAUSED:
            if(stateHasChanged())
            {
                mUi->lSpeed->setText(QString::fromUtf8("%1").arg(tr("PAUSED")));
                mUi->bClockDown->setVisible(false);
                mUi->lRemainingTime->setText(QString::fromUtf8(""));
            }
            break;
        case TransferData::TransferState::TRANSFER_QUEUED:
            if(stateHasChanged())
            {
                mUi->lSpeed->setText(QString::fromUtf8("%1").arg(tr("queued")));
                mUi->bClockDown->setVisible(false);
                mUi->lRemainingTime->setText(QString::fromUtf8(""));
            }
            break;
        case TransferData::TransferState::TRANSFER_RETRYING:

            if (getData()->mErrorCode == MegaError::API_EOVERQUOTA)
            {
                if (getData()->mErrorValue)
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
        case TransferData::TransferState::TRANSFER_COMPLETING:
            if(stateHasChanged())
            {
                mUi->lSpeed->setText(QString::fromUtf8("%1").arg(tr("completing...")));
                mUi->bClockDown->setVisible(false);
                mUi->lRemainingTime->setText(QString::fromUtf8(""));
            }
            break;
        default:
            if(stateHasChanged())
            {
                mUi->lSpeed->setText(QString::fromUtf8(""));
                mUi->lRemainingTime->setText(QString::fromUtf8(""));
                mUi->bClockDown->setVisible(false);
            }
            break;
    }

    // Update progress bar
    unsigned int permil = (getData()->mTotalSize > 0) ? ((1000 * getData()->mTransferredBytes) / getData()->mTotalSize) : 0;
    mUi->pbTransfer->setValue(permil);
}

void InfoDialogTransferDelegateWidget::setFileNameAndType()
{
    mUi->lFileName->ensurePolished();
    mUi->lFileName->setText(mUi->lFileName->fontMetrics()
                            .elidedText(getData()->mFilename, Qt::ElideMiddle, mUi->lFileName->width()));
    mUi->lFileName->setToolTip(getData()->mFilename);

    mUi->lFileNameCompleted->ensurePolished();
    mUi->lFileNameCompleted->setText(mUi->lFileNameCompleted->fontMetrics()
                                     .elidedText(getData()->mFilename, Qt::ElideMiddle,
                                                 mUi->lFileNameCompleted->width()));
    mUi->lFileNameCompleted->setToolTip(getData()->mFilename);

    QIcon icon = Utilities::getExtensionPixmapMedium(getData()->mFilename);
    mUi->lFileType->setIcon(icon);
    mUi->lFileType->setIconSize(QSize(48, 48));
    mUi->lFileTypeCompleted->setIcon(icon);
    mUi->lFileTypeCompleted->setIconSize(QSize(48, 48));
}

void InfoDialogTransferDelegateWidget::setType()
{
    QIcon icon;

    switch (getData()->mType)
    {
        case TransferData::TransferType::TRANSFER_UPLOAD:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/upload_item_ico.png"));
            mUi->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: transparent;}"
                                                            "QProgressBar#pbTransfer::chunk {background-color: #2ba6de;}"));
            break;
        case TransferData::TransferType::TRANSFER_DOWNLOAD:
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

QString InfoDialogTransferDelegateWidget::getTransferName()
{
    return mUi->lFileName->text();
}

void InfoDialogTransferDelegateWidget::updateFinishedIco(int transferType, int errorCode)
{
    QIcon iconCompleted;

    switch (transferType)
    {
        case TransferData::TransferType::TRANSFER_UPLOAD:
            iconCompleted = Utilities::getCachedPixmap(errorCode < 0 ? QString::fromUtf8(":/images/upload_fail_item_ico.png")
                                                                      : QString::fromUtf8(":/images/uploaded_item_ico.png"));
            break;
        case TransferData::TransferType::TRANSFER_DOWNLOAD:
            iconCompleted = Utilities::getCachedPixmap(errorCode < 0 ? QString::fromUtf8(":/images/download_fail_item_ico.png")
                                                                      : QString::fromUtf8(":/images/downloaded_item_ico.png"));
            break;
        default:
            break;
    }

    mUi->lTransferTypeCompleted->setIcon(iconCompleted);
    mUi->lTransferTypeCompleted->setIconSize(QSize(mUi->lTransferTypeCompleted->width(), mUi->lTransferTypeCompleted->height()));
}

TransferBaseDelegateWidget::ActionHoverType InfoDialogTransferDelegateWidget::mouseHoverTransfer(bool isHover, const QPoint &pos)
{
    bool update(false);
    ActionHoverType hoverType(ActionHoverType::NONE);

    if(!getData())
    {
        return hoverType;
    }

    mIsHover = isHover;

    if (mIsHover)
    {
        mActionButtonsEnabled = true;
        if (getData()->mErrorCode < 0)
        {
            if (!getData()->mIsSyncTransfer)
            {
                bool in = isMouseHoverInAction(mUi->lActionTransfer, pos);
                update = setActionTransferIcon(mUi->lActionTransfer,
                                               QString::fromAscii("://images/ico_item_retry%1.png").arg(QString::fromAscii(in?"_hover_ico":"")));
                if(in)
                {
                    hoverType = ActionHoverType::HOVER_ENTER;
                }
                else if(update)
                {
                    hoverType = ActionHoverType::HOVER_LEAVE;
                }
            }
            else
            {
                update = setActionTransferIcon(mUi->lActionTransfer,
                                               QString::fromAscii("://images/error.png"));
                mActionButtonsEnabled = false;

                if(update)
                {
                    hoverType = ActionHoverType::HOVER_LEAVE;
                }
            }
            mUi->lShowInFolder->hide();
            update = true;
        }
        else if (getData()->isPublicNode())
        {
            bool inAction = isMouseHoverInAction(mUi->lActionTransfer, pos);
            update = setActionTransferIcon(mUi->lActionTransfer,
                                           QString::fromAscii("://images/ico_item_link%1.png").arg(QString::fromAscii(inAction?"_hover_ico":"")));

            mUi->lShowInFolder->show();

            bool inShowFolder = isMouseHoverInAction(mUi->lShowInFolder, pos);
            update |= setActionTransferIcon(mUi->lShowInFolder,
                                           QString::fromAscii("://images/showinfolder%1.png").arg(QString::fromAscii(inShowFolder?"_hover_ico":"")));


            if(inAction || inShowFolder)
            {
                hoverType = ActionHoverType::HOVER_ENTER;
            }
            else if(update)
            {
                hoverType = ActionHoverType::HOVER_LEAVE;
            }
        }
        else
        {
            bool inAction = isMouseHoverInAction(mUi->lActionTransfer, pos);
            update = setActionTransferIcon(mUi->lActionTransfer,
                                           QString::fromAscii("://images/showinfolder%1.png").arg(QString::fromAscii(inAction?"_hover_ico":"")));

            if(update)
            {
                hoverType = ActionHoverType::HOVER_LEAVE;
            }
        }
    }
    else
    {
        mActionButtonsEnabled = false;
        if (getData()->mErrorCode < 0)
        {
            update = setActionTransferIcon(mUi->lActionTransfer,
                                           QString::fromAscii("://images/error.png"));
            mUi->lActionTransfer->setIconSize(QSize(24,24));
            mUi->lShowInFolder->hide();
        }
        else
        {
            update = setActionTransferIcon(mUi->lActionTransfer,
                                           QString::fromAscii("://images/success.png"));
            mUi->lActionTransfer->setIconSize(QSize(24,24));
            mUi->lShowInFolder->hide();
        }

        if(update)
        {
            hoverType = ActionHoverType::HOVER_LEAVE;
        }
    }

    return hoverType;
}

bool InfoDialogTransferDelegateWidget::mouseHoverRetryingLabel(QPoint pos)
{
    switch (getData()->mState)
    {
        case TransferData::TransferState::TRANSFER_RETRYING:
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

void InfoDialogTransferDelegateWidget::finishTransfer()
{
    mUi->sTransferState->setCurrentWidget(mUi->completedTransfer);
    if (getData()->mErrorCode < 0)
    {
        mUi->lActionTransfer->setIcon(QIcon(QString::fromAscii("://images/error.png")));
        mUi->lActionTransfer->setIconSize(QSize(24,24));
        mUi->lElapsedTime->setStyleSheet(QString::fromUtf8("color: #F0373A"));

        //Check if transfer finishes while the account was blocked, in order to provide the right context for failed error
        bool blockedTransfer = static_cast<MegaApplication*>(qApp)->finishedTransfersWhileBlocked(getData()->mTag);
        if (blockedTransfer)
        {
            static_cast<MegaApplication*>(qApp)->removeFinishedBlockedTransfer(getData()->mTag);
        }

        mUi->lElapsedTime->setText(tr("failed:") + QString::fromUtf8(" ") + QCoreApplication::translate("MegaError",
                                                                                                       MegaError::getErrorString(getData()->mErrorCode,
                                                                                                                                 getData()->mType == TransferData::TransferType::TRANSFER_DOWNLOAD && !blockedTransfer
                                                                                                                                 ? MegaError::API_EC_DOWNLOAD : MegaError::API_EC_DEFAULT)));
        updateFinishedIco(getData()->mType, true);
    }
    else
    {
        mUi->lActionTransfer->setIcon(QIcon(QString::fromAscii("://images/success.png")));
        mUi->lActionTransfer->setIconSize(QSize(24,24));
        updateFinishedIco(getData()->mType, false);
    }
}

void InfoDialogTransferDelegateWidget::updateFinishedTime()
{
    auto finishedTime = getData()->getFinishedTime();

    if (!finishedTime || getData()->mErrorCode < 0)
    {
        return;
    }

    mUi->lElapsedTime->setStyleSheet(QString::fromUtf8("color: #999999"));
    mUi->lElapsedTime->setText(tr("Added [A]").replace(QString::fromUtf8("[A]"), Utilities::getFinishedTimeString(finishedTime)));
}

QSize InfoDialogTransferDelegateWidget::minimumSizeHint() const
{
    return fullRect.size();
}
QSize InfoDialogTransferDelegateWidget::sizeHint() const
{
    return fullRect.size();
}

void InfoDialogTransferDelegateWidget::on_lShowInFolder_clicked()
{ 
    emit openTransferFolder();
}

void InfoDialogTransferDelegateWidget::on_lActionTransfer_clicked()
{
    emit copyTransferLink();
}

