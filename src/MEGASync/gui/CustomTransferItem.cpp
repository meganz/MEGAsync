#include "CustomTransferItem.h"
#include "ui_CustomTransferItem.h"
#include "MegaApplication.h"
#include "control/Utilities.h"
#include "platform/Platform.h"

#include <QImageReader>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

CustomTransferItem::CustomTransferItem(QWidget *parent) :
    TransferItem(parent),
    ui(new Ui::CustomTransferItem)
{
    ui->setupUi(this);

#if QT_VERSION > 0x050200
    QSizePolicy retainGetLink = ui->lActionTransfer->sizePolicy();
    retainGetLink.setRetainSizeWhenHidden(true);
    ui->lActionTransfer->setSizePolicy(retainGetLink);

    QSizePolicy retainShowInFolder = ui->lShowInFolder->sizePolicy();
    retainShowInFolder.setRetainSizeWhenHidden(true);
    ui->lShowInFolder->setSizePolicy(retainShowInFolder);
#endif

    ui->bClockDown->setVisible(false);

    actionButtonsEnabled = false;
    dsFinishedTime = 0;
    mTransferFinishedWhileBlocked = false;

    megaApi = ((MegaApplication *)qApp)->getMegaApi();
    ui->lShowInFolder->hide();
    update();
}

CustomTransferItem::~CustomTransferItem()
{
    delete ui;
}

void CustomTransferItem::setFileName(QString fileName)
{
    TransferItem::setFileName(fileName);
    ui->lFileName->ensurePolished();
    ui->lFileName->setText(ui->lFileName->fontMetrics().elidedText(fileName, Qt::ElideMiddle,ui->lFileName->width()));
    ui->lFileName->setToolTip(fileName);

    ui->lFileNameCompleted->ensurePolished();
    ui->lFileNameCompleted->setText(ui->lFileNameCompleted->fontMetrics().elidedText(fileName, Qt::ElideMiddle,ui->lFileNameCompleted->width()));
    ui->lFileNameCompleted->setToolTip(fileName);

    QIcon icon = Utilities::getExtensionPixmapMedium(fileName);
    ui->lFileType->setIcon(icon);
    ui->lFileType->setIconSize(QSize(48, 48));
    ui->lFileTypeCompleted->setIcon(icon);
    ui->lFileTypeCompleted->setIconSize(QSize(48, 48));
}

void CustomTransferItem::setType(int type, bool isSyncTransfer)
{
    TransferItem::setType(type, isSyncTransfer);
    QIcon icon, iconCompleted;

    switch (type)
    {
        case MegaTransfer::TYPE_UPLOAD:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/upload_item_ico.png"));
            ui->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: transparent;}"
                                                            "QProgressBar#pbTransfer::chunk {background-color: #2ba6de;}"));
            break;
        case MegaTransfer::TYPE_DOWNLOAD:
            icon = Utilities::getCachedPixmap(QString::fromUtf8(":/images/download_item_ico.png"));
            ui->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: transparent;}"
                                                            "QProgressBar#pbTransfer::chunk {background-color: #31b500;}"));
            break;
        default:
            break;
    }

    ui->lTransferType->setIcon(icon);
    ui->lTransferType->setIconSize(QSize(ui->lTransferType->width(), ui->lTransferType->height()));
}

void CustomTransferItem::setTransferState(int value)
{
    TransferItem::setTransferState(value);
    switch (transferState)
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
    return ui->lFileName->text();
}

bool CustomTransferItem::checkIsInsideButton(QPoint pos, int button)
{
    if (!actionButtonsEnabled)
    {
        return false;
    }

    switch (transferState)
    {
    case MegaTransfer::STATE_COMPLETED:
    case MegaTransfer::STATE_FAILED:
    {
        if ((button == TransferItem::ACTION_BUTTON && ui->lActionTransfer->rect().contains(ui->lActionTransfer->mapFrom(this, pos)))
                || (button == TransferItem::SHOW_IN_FOLDER_BUTTON && ui->lShowInFolder->rect().contains(ui->lShowInFolder->mapFrom(this, pos))))
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
    if (name != lastActionTransferIconName)
    {
        ui->lActionTransfer->setIcon(Utilities::getCachedPixmap(name));
        ui->lActionTransfer->setIconSize(QSize(24,24));
        lastActionTransferIconName = name;
    }
}


void CustomTransferItem::setShowInFolderIcon(const QString &name)
{
    if (name != lastShowInFolderIconName)
    {
        ui->lShowInFolder->setIcon(Utilities::getCachedPixmap(name));
        ui->lShowInFolder->setIconSize(QSize(24,24));

        lastShowInFolderIconName = name;
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

    ui->lTransferTypeCompleted->setIcon(iconCompleted);
    ui->lTransferTypeCompleted->setIconSize(QSize(ui->lTransferTypeCompleted->width(), ui->lTransferTypeCompleted->height()));
}

void CustomTransferItem::mouseHoverTransfer(bool isHover, const QPoint &pos)
{

    if (isHover)
    {
        actionButtonsEnabled = true;
        bool transferedFileStillExists = QFile(fullpath).exists();
        if (transferError < 0)
        {
            if (!isSyncTransfer)
            {
                bool in = ui->lActionTransfer->rect().contains(ui->lActionTransfer->mapFrom(this, pos));
                setActionTransferIcon(QString::fromAscii("://images/ico_item_retry%1.png").arg(QString::fromAscii(in?"":"_greyed")));
            }
            else
            {
                setActionTransferIcon(QString::fromAscii("://images/error.png"));
                actionButtonsEnabled = false;
            }
            ui->lShowInFolder->hide();
        }
        else if (isLinkAvailable)
        {
            bool in = ui->lActionTransfer->rect().contains(ui->lActionTransfer->mapFrom(this, pos));
            setActionTransferIcon(QString::fromAscii("://images/ico_item_link%1.png").arg(QString::fromAscii(in?"":"_greyed")));

            if (transferedFileStillExists)
            {
                in = ui->lShowInFolder->rect().contains(ui->lShowInFolder->mapFrom(this, pos));
                setShowInFolderIcon(QString::fromAscii("://images/showinfolder%1.png").arg(QString::fromAscii(in?"":"_greyed")));
                ui->lShowInFolder->show();
            }
        }
        else if (transferedFileStillExists)
        {
            bool in = ui->lActionTransfer->rect().contains(ui->lActionTransfer->mapFrom(this, pos));
            setActionTransferIcon(QString::fromAscii("://images/showinfolder%1.png").arg(QString::fromAscii(in?"":"_greyed")));
        }
    }
    else
    {
        actionButtonsEnabled = false;
        if (transferError < 0)
        {
            setActionTransferIcon(QString::fromAscii("://images/error.png"));
            ui->lActionTransfer->setIconSize(QSize(24,24));
            ui->lShowInFolder->hide();
        }
        else
        {
            setActionTransferIcon(QString::fromAscii("://images/success.png"));
            ui->lActionTransfer->setIconSize(QSize(24,24));
            ui->lShowInFolder->hide();
        }
    }

    emit refreshTransfer(this->getTransferTag());
}

bool CustomTransferItem::mouseHoverRetryingLabel(QPoint pos)
{
    switch (transferState)
    {
        case MegaTransfer::STATE_RETRYING:
            if (ui->lSpeed->rect().contains(ui->lSpeed->mapFrom(this, pos)))
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
    ui->sTransferState->setCurrentWidget(ui->completedTransfer);
    if (transferError < 0)
    {
        ui->lActionTransfer->setIcon(QIcon(QString::fromAscii("://images/error.png")));
        ui->lActionTransfer->setIconSize(QSize(24,24));
        ui->lElapsedTime->setStyleSheet(QString::fromUtf8("color: #F0373A"));
        ui->lElapsedTime->setText(tr("failed:") + QString::fromUtf8(" ") + QCoreApplication::translate("MegaError",
                                                                                                       MegaError::getErrorString(transferError,
                                                                                                                                 this->getType() == MegaTransfer::TYPE_DOWNLOAD && !mTransferFinishedWhileBlocked
                                                                                                                                 ? MegaError::API_EC_DOWNLOAD : MegaError::API_EC_DEFAULT)));
        updateFinishedIco(type, true);
    }
    else
    {
        ui->lActionTransfer->setIcon(QIcon(QString::fromAscii("://images/success.png")));
        ui->lActionTransfer->setIconSize(QSize(24,24));
        updateFinishedIco(type, false);
    }
}

void CustomTransferItem::updateTransfer()
{
    if (transferState == MegaTransfer::STATE_COMPLETED || transferState == MegaTransfer::STATE_FAILED)
    {
        ui->sTransferState->setCurrentWidget(ui->completedTransfer);
    }
    else
    {
        ui->sTransferState->setCurrentWidget(ui->activeTransfer);
    }

    switch (transferState)
    {
        case MegaTransfer::STATE_ACTIVE:
        {
            // Update remaining time
            long long remainingBytes = totalSize - totalTransferredBytes;
            const auto totalRemainingSeconds = mTransferRemainingTime.calculateRemainingTimeSeconds(transferSpeed, remainingBytes);

            QString remainingTime;
            const bool printableValue{totalRemainingSeconds.count() && totalRemainingSeconds < std::chrono::seconds::max()};
            if (printableValue)
            {
                remainingTime = Utilities::getTimeString(totalRemainingSeconds.count());
                ui->bClockDown->setVisible(true);
            }
            else
            {
                ui->bClockDown->setVisible(false);
            }
            ui->lRemainingTime->setText(remainingTime);

            // Update current transfer speed
            QString downloadString;

            if (!totalTransferredBytes)
            {
                downloadString = QString::fromUtf8("%1").arg(tr("starting..."));
            }
            else
            {
                QString pattern(QString::fromUtf8("%1/s"));
                downloadString = pattern.arg(Utilities::getSizeString(transferSpeed));
            }

            ui->lSpeed->setText(downloadString);
            break;
        }
        case MegaTransfer::STATE_PAUSED:
            ui->lSpeed->setText(QString::fromUtf8("%1").arg(tr("PAUSED")));
            ui->bClockDown->setVisible(false);
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        case MegaTransfer::STATE_QUEUED:
            ui->lSpeed->setText(QString::fromUtf8("%1").arg(tr("queued")));
            ui->bClockDown->setVisible(false);
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        case MegaTransfer::STATE_RETRYING:
            if (transferError == MegaError::API_EOVERQUOTA)
            {
                if (transferErrorValue)
                {
                    ui->lSpeed->setText(QString::fromUtf8("%1").arg(tr("Transfer quota exceeded")));
                }
                else
                {
                    ui->lSpeed->setText(QString::fromUtf8("%1").arg(tr("Out of storage space")));
                }
            }
            else
            {
                ui->lSpeed->setText(QString::fromUtf8("%1").arg(tr("retrying...")));
            }

            ui->bClockDown->setVisible(false);
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        case MegaTransfer::STATE_COMPLETING:
            ui->lSpeed->setText(QString::fromUtf8("%1").arg(tr("completing...")));
            ui->bClockDown->setVisible(false);
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        default:
            ui->lSpeed->setText(QString::fromUtf8(""));
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            ui->bClockDown->setVisible(false);
            break;
    }

    // Update progress bar
    unsigned int permil = (totalSize > 0) ? static_cast<unsigned int>((1000 * totalTransferredBytes) / totalSize) : 0;
    ui->pbTransfer->setValue(permil);
}

void CustomTransferItem::updateFinishedTime()
{
    if (!dsFinishedTime || transferError < 0)
    {
        return;
    }

    auto preferences = Preferences::instance();
    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = ( now.toMSecsSinceEpoch() / 100 - (preferences->getMsDiffTimeWithSDK() + dsFinishedTime) ) / 10;

    ui->lElapsedTime->setStyleSheet(QString::fromUtf8("color: #999999"));
    ui->lElapsedTime->setText(tr("Added [A]").replace(QString::fromUtf8("[A]"), Utilities::getFinishedTimeString(secs)));
}

void CustomTransferItem::setStateLabel(QString labelState)
{
    ui->lSpeed->setText(QString::fromUtf8("%1").arg(labelState));
    ui->lRemainingTime->setText(QString::fromUtf8(""));
    ui->bClockDown->setVisible(false);
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

