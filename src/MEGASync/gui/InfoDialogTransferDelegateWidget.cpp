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

    connect(mUi->lShowInFolder, &QPushButton::clicked, this, &InfoDialogTransferDelegateWidget::onShowFolderClicked);
    connect(mUi->lActionTransfer, &QPushButton::clicked, this, &InfoDialogTransferDelegateWidget::onActionClicked);
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
            const auto totalRemainingSeconds = mTransferRemainingTime.calculateRemainingTimeSeconds(getData()->mSpeed, remainingBytes);

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

bool InfoDialogTransferDelegateWidget::setActionTransferIcon(const QString &name)
{
    bool update(false);

    if (name != mLastActionTransferIconName)
    {
        mUi->lActionTransfer->setIcon(Utilities::getCachedPixmap(name));
        mUi->lActionTransfer->setIconSize(QSize(24,24));
        mLastActionTransferIconName = name;

        update = true;
    }

    return update;
}

bool InfoDialogTransferDelegateWidget::setShowInFolderIcon(const QString &name)
{
    bool update(false);

    if (name != mLastShowInFolderIconName)
    {
        mUi->lShowInFolder->setIcon(Utilities::getCachedPixmap(name));
        mUi->lShowInFolder->setIconSize(QSize(24,24));

        mLastShowInFolderIconName = name;

        update = true;
    }

    return update;
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

bool InfoDialogTransferDelegateWidget::mouseHoverTransfer(bool isHover, const QPoint &pos)
{
    bool update(false);

    if(!getData())
    {
        return false;
    }

    mIsHover = isHover;

    if (mIsHover)
    {
        mActionButtonsEnabled = true;
        if (getData()->mErrorCode < 0)
        {
            if (!getData()->mIsSyncTransfer)
            {
                auto actionGlobalPos = mUi->lActionTransfer->mapTo(this, QPoint(0,0));
                QRect actionGeometry(actionGlobalPos, mUi->lActionTransfer->size());

                bool in = actionGeometry.contains(/*mUi->lActionTransfer->mapFrom(this, pos)*/pos);
                update = setActionTransferIcon(QString::fromAscii("://images/ico_item_retry%1.png").arg(QString::fromAscii(in?"":"_greyed")));
            }
            else
            {
                update = setActionTransferIcon(QString::fromAscii("://images/error.png"));
                mActionButtonsEnabled = false;
            }
            mUi->lShowInFolder->hide();
            update = true;
        }
        else if (getData()->mIsPublicNode)
        {
            auto actionGlobalPos = mUi->lActionTransfer->mapTo(this, QPoint(0,0));
            QRect actionGeometry(actionGlobalPos, mUi->lActionTransfer->size());

            bool inAction = actionGeometry.contains(pos);

            update = setActionTransferIcon(QString::fromAscii("://images/ico_item_link%1.png").arg(QString::fromAscii(inAction?"":"_greyed")));

            auto showInFolderGlobalPos = mUi->lShowInFolder->mapTo(this, QPoint(0,0));
            QRect showInFolderGeometry(showInFolderGlobalPos, mUi->lShowInFolder->size());
            bool inShowFolder = showInFolderGeometry.contains(pos);

            update |= setShowInFolderIcon(QString::fromAscii("://images/showinfolder%1.png").arg(QString::fromAscii(inShowFolder?"":"_greyed")));

            mUi->lShowInFolder->show();
        }
        else
        {
            auto actionGlobalPos = mUi->lActionTransfer->mapTo(this, QPoint(0,0));
            QRect actionGeometry(actionGlobalPos, mUi->lActionTransfer->size());

            bool in = actionGeometry.contains(/*mUi->lActionTransfer->mapFrom(this, pos)*/pos);
            update = setActionTransferIcon(QString::fromAscii("://images/showinfolder%1.png").arg(QString::fromAscii(in?"":"_greyed")));
        }
    }
    else
    {
        mActionButtonsEnabled = false;
        if (getData()->mErrorCode < 0)
        {
            update = setActionTransferIcon(QString::fromAscii("://images/error.png"));
            mUi->lActionTransfer->setIconSize(QSize(24,24));
            mUi->lShowInFolder->hide();
        }
        else
        {
            update = setActionTransferIcon(QString::fromAscii("://images/success.png"));
            mUi->lActionTransfer->setIconSize(QSize(24,24));
            mUi->lShowInFolder->hide();
        }
    }

    return update;
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
    if (!getData()->mFinishedTime || getData()->mErrorCode < 0)
    {
        return;
    }

    //Preferences *preferences = Preferences::instance();
    QDateTime now = QDateTime::currentDateTime();
    qint64 secs= ((/*preferences->getMsDiffTimeWithSDK() +*/  now.toMSecsSinceEpoch() - (getData()->mFinishedTime*1000)))/1000;

    mUi->lElapsedTime->setStyleSheet(QString::fromUtf8("color: #999999"));
    mUi->lElapsedTime->setText(tr("Added [A]").replace(QString::fromUtf8("[A]"), Utilities::getFinishedTimeString(secs)));
}

QSize InfoDialogTransferDelegateWidget::minimumSizeHint() const
{
    return fullRect.size();
}
QSize InfoDialogTransferDelegateWidget::sizeHint() const
{
    return fullRect.size();
}

void InfoDialogTransferDelegateWidget::onShowFolderClicked()
{
    if (getData() && getData()->mState == TransferData::TransferState::TRANSFER_COMPLETED
                 && !getData()->mPath.isEmpty())
    {
        QString localPath = getData()->mPath;
        #ifdef WIN32
        if (localPath.startsWith(QString::fromAscii("\\\\?\\")))
        {
            localPath = localPath.mid(4);
        }
        #endif
        Platform::showInFolder(localPath);
    }
}

void InfoDialogTransferDelegateWidget::onActionClicked()
{
    QList<MegaHandle> exportList;
    QStringList linkList;
    auto transfer = mMegaApi->getTransferByTag(getData()->mTag);
    if(transfer)
    {
        MegaNode *node = transfer->getPublicMegaNode();
        if (!node || !node->isPublic())
        {
            exportList.push_back(transfer->getNodeHandle());
        }
        else
        {
            char *handle = node->getBase64Handle();
            char *key = node->getBase64Key();
            if (handle && key)
            {
                QString link = Preferences::BASE_URL + QString::fromUtf8("/#!%1!%2")
                        .arg(QString::fromUtf8(handle)).arg(QString::fromUtf8(key));
                linkList.append(link);
            }
            delete [] handle;
            delete [] key;
        }
        delete node;
        if (exportList.size() || linkList.size())
        {
            ((MegaApplication*)qApp)->exportNodes(exportList, linkList);
        }
    }
}

