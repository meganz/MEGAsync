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
            mUi->bClockDown->setVisible(getData()->mRemainingTime > 0);
            mUi->lRemainingTime->setText(Utilities::getTimeString(getData()->mRemainingTime));

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
            updateTransferControlsOnHold(tr("PAUSED"));
            break;
        case TransferData::TransferState::TRANSFER_QUEUED:
            updateTransferControlsOnHold(tr("queued"));
            break;
        case TransferData::TransferState::TRANSFER_RETRYING:
            if (getData()->mErrorCode == MegaError::API_EOVERQUOTA)
            {
                if (getData()->mErrorValue)
                {
                    updateTransferControlsOnHold(tr("Transfer quota exceeded"));
                }
                else
                {
                    updateTransferControlsOnHold(tr("Out of storage space"));
                }
            }
            else
            {
                updateTransferControlsOnHold(tr("retrying..."));
            }
            break;
        case TransferData::TransferState::TRANSFER_COMPLETING:
            updateTransferControlsOnHold(tr("completing..."));
            break;
        default:
            updateTransferControlsOnHold(QString());
            break;
    }

    // Update progress bar
    unsigned int permil = (getData()->mTotalSize > 0) ? ((1000 * getData()->mTransferredBytes) / getData()->mTotalSize) : 0;
    mUi->pbTransfer->setValue(permil);
}

void InfoDialogTransferDelegateWidget::updateTransferControlsOnHold(const QString& speedText)
{
    if (stateHasChanged())
    {
        mUi->lSpeed->setText(speedText);
        mUi->bClockDown->setVisible(false);
        mUi->lRemainingTime->setText(QString::fromStdString(""));
    }
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

    // Assume the action widget is shown.
    mUi->lActionTransfer->show();
    mUi->lActionTransfer->setToolTip(QString());

    // Assume the show in folder is hidden
    mUi->lShowInFolder->hide();

    if(getData()->isFinished())
    {
        if (mIsHover)
        {
            mActionButtonsEnabled = true;
            if (getData()->mErrorCode < 0)
            {
                if (!getData()->isSyncTransfer())
                {
                    bool in = isMouseHoverInAction(mUi->lActionTransfer, pos);
                    mUi->lActionTransfer->setToolTip(tr("Retry"));
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
                    mUi->lActionTransfer->setToolTip(tr("Failed: %1").arg(QString::fromStdString(getData()->mFailedTransfer->getLastError().getErrorString())));
                    mActionButtonsEnabled = false;

                    if(update)
                    {
                        hoverType = ActionHoverType::HOVER_LEAVE;
                    }
                }

                update = true;
            }
            else
            {
                bool inAction(false);

                if (getData()->isPublicNode())
                {
                    inAction = isMouseHoverInAction(mUi->lActionTransfer, pos);
                    update = setActionTransferIcon(mUi->lActionTransfer,
                                                   QString::fromAscii("://images/ico_item_link%1.png").arg(QString::fromAscii(inAction?"_hover_ico":"")));
                    mUi->lActionTransfer->setToolTip(tr("Copy link to file"));
                }
                else
                {
                    mUi->lActionTransfer->hide();
                }

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
        }
        else
        {
            mActionButtonsEnabled = false;
            if (getData()->mErrorCode < 0)
            {
                update = setActionTransferIcon(mUi->lActionTransfer,
                                               QString::fromAscii("://images/error.png"));
                mUi->lActionTransfer->setIconSize(QSize(24,24));
            }
            else
            {
                update = setActionTransferIcon(mUi->lActionTransfer,
                                               QString::fromAscii("://images/success.png"));
                mUi->lActionTransfer->setIconSize(QSize(24,24));
            }

            if(update)
            {
                hoverType = ActionHoverType::HOVER_LEAVE;
            }
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
    if (getData()->mErrorCode < 0)
    {
        if (!getData()->isSyncTransfer())
        {
            //Base implementation
            onRetryTransfer();
        }
    }
    else
    {
        emit copyTransferLink();
    }
}

