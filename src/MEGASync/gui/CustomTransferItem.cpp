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
    ui->bClockDown->setVisible(false);

    getLinkButtonEnabled = false;
    remainingUploads = remainingDownloads = 0;
    totalUploads = totalDownloads = 0;
    dsFinishedTime = 0;

    megaApi = ((MegaApplication *)qApp)->getMegaApi();
    ui->lGetLink->installEventFilter(this);
    update();
}

CustomTransferItem::~CustomTransferItem()
{
    delete ui;
}

void CustomTransferItem::setFileName(QString fileName)
{
    TransferItem::setFileName(fileName);

    QFont f = ui->lFileName->font();
    QFontMetrics fm = QFontMetrics(f);
    ui->lFileName->setText(fm.elidedText(fileName, Qt::ElideMiddle,ui->lFileName->width()));
    ui->lFileName->setToolTip(fileName);
    ui->lFileNameCompleted->setText(fm.elidedText(fileName, Qt::ElideMiddle,ui->lFileName->width()));
    ui->lFileNameCompleted->setToolTip(fileName);


    QIcon icon;
    icon.addFile(Utilities::getExtensionPixmapMedium(fileName), QSize(), QIcon::Normal, QIcon::Off);
    ui->lFileType->setIcon(icon);
    ui->lFileType->setIconSize(QSize(48, 48));
    ui->lFileTypeCompleted->setIcon(icon);
    ui->lFileTypeCompleted->setIconSize(QSize(48, 48));
}

void CustomTransferItem::setType(int type, bool isSyncTransfer)
{
    TransferItem::setType(type, isSyncTransfer);
    QIcon icon, iconCompleted;

    qreal ratio = 1.0;
#if QT_VERSION >= 0x050000
    ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
#endif

    switch (type)
    {
        case MegaTransfer::TYPE_UPLOAD:
            icon.addFile(QString::fromUtf8(":/images/upload_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
            iconCompleted.addFile(QString::fromUtf8(":/images/uploaded_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
                                                            "QProgressBar#pbTransfer::chunk {background-color: #2ba6de;}"));
            break;
        case MegaTransfer::TYPE_DOWNLOAD:
            icon.addFile(QString::fromUtf8(":/images/download_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
            iconCompleted.addFile(QString::fromUtf8(":/images/downloaded_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
                                                            "QProgressBar#pbTransfer::chunk {background-color: #31b500;}"));
            break;
        default:
            break;
    }

    ui->lTransferType->setIcon(icon);
    ui->lTransferType->setIconSize(QSize(12, 12));
    ui->lTransferTypeCompleted->setIcon(iconCompleted);
    ui->lTransferTypeCompleted->setIconSize(QSize(12, 12));
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

bool CustomTransferItem::getLinkButtonClicked(QPoint pos)
{
    if (!getLinkButtonEnabled || !isLinkAvailable)
    {
        return false;
    }

    switch (transferState)
    {
    case MegaTransfer::STATE_COMPLETED:
    case MegaTransfer::STATE_FAILED:
        if (ui->lGetLink->rect().contains(ui->lGetLink->mapFrom(this, pos)))
        {
            return true;
        }
        break;
    case MegaTransfer::STATE_CANCELLED:
    default:
        break;
    }

    return false;
}

void CustomTransferItem::mouseHoverTransfer(bool isHover)
{
    if (isHover)
    {
        getLinkButtonEnabled = true;
        if (isLinkAvailable || transferError < 0)
        {
            ui->lGetLink->removeEventFilter(this);
            ui->lGetLink->update();
        }
    }
    else
    {
        getLinkButtonEnabled = false;
        if (isLinkAvailable || transferError < 0)
        {
            ui->lGetLink->installEventFilter(this);
            ui->lGetLink->update();
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
        ui->lGetLink->setIcon(QIcon(QString::fromAscii("://images/ico_item_retry.png")));
        ui->lGetLink->setIconSize(QSize(24,24));
        ui->lElapsedTime->setStyleSheet(QString::fromUtf8("color: #F0373A"));
        ui->lElapsedTime->setText(tr("failed:") + QString::fromUtf8(" ") + QCoreApplication::translate("MegaError", MegaError::getErrorString(transferError)));
    }
}

void CustomTransferItem::updateTransfer()
{
    int currentUpload, currentDownload;

    if (transferState == MegaTransfer::STATE_COMPLETED || transferState == MegaTransfer::STATE_FAILED)
    {
        ui->sTransferState->setCurrentWidget(ui->completedTransfer);
    }
    else
    {
        ui->sTransferState->setCurrentWidget(ui->activeTransfer);

        QString formattedValue(QString::fromUtf8("<span style=\"color:#333333; text-decoration:none;\">&nbsp;%1&nbsp;</span>"));
        QString nTransfersPattern(tr("%1 of %2"));

        switch (type)
        {
            case MegaTransfer::TYPE_UPLOAD:
                remainingUploads = megaApi->getNumPendingUploads();
                totalUploads = megaApi->getTotalUploads();

                if (totalUploads < remainingUploads)
                {
                    totalUploads = remainingUploads;
                }

                currentUpload = totalUploads - remainingUploads + 1;
                //Update current and total number of transfers
                ui->lTransfers->setText(nTransfersPattern.arg(formattedValue.arg(currentUpload)).arg(formattedValue.arg(totalUploads)));

                break;
            case MegaTransfer::TYPE_DOWNLOAD:
                remainingDownloads = megaApi->getNumPendingDownloads();
                totalDownloads = megaApi->getTotalDownloads();

                if (totalDownloads < remainingDownloads)
                {
                    totalDownloads = remainingDownloads;
                }

                currentDownload = totalDownloads - remainingDownloads + 1;
                //Update current and total number of transfers
                ui->lTransfers->setText(nTransfersPattern.arg(formattedValue.arg(currentDownload)).arg(formattedValue.arg(totalDownloads)));

                break;
            default:
                break;
        }
    }

    switch (transferState)
    {
        case MegaTransfer::STATE_ACTIVE:
        {
            // Update remaining time
            long long remainingBytes = totalSize - totalTransferredBytes;
            int totalRemainingSeconds = meanTransferSpeed ? remainingBytes / meanTransferSpeed : 0;

            QString remainingTime;
            if (totalRemainingSeconds)
            {
                if (totalRemainingSeconds < 60)
                {
                    remainingTime = QString::fromUtf8("%1 <span style=\"color:#777777; text-decoration:none;\">m</span>").arg(QString::fromUtf8("&lt; 1"));
                }
                else
                {
                    remainingTime = Utilities::getTimeString(totalRemainingSeconds, false);
                }
                ui->bClockDown->setVisible(true);
            }
            else
            {
                remainingTime = QString::fromAscii("");
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
                QString pattern(QString::fromUtf8("(%1/s)"));
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
                ui->lSpeed->setText(QString::fromUtf8("%1").arg(tr("Out of storage space")));
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
    unsigned int permil = (totalSize > 0) ? ((1000 * totalTransferredBytes) / totalSize) : 0;
    ui->pbTransfer->setValue(permil);
}

void CustomTransferItem::updateFinishedTime()
{
    if (!dsFinishedTime || transferError < 0)
    {
        return;
    }

    Preferences *preferences = Preferences::instance();
    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = ( now.toMSecsSinceEpoch() / 100 - (preferences->getMsDiffTimeWithSDK() + dsFinishedTime) ) / 10;

    ui->lGetLink->setIcon(QIcon(QString::fromAscii("://images/ico_item_link.png")));
    ui->lGetLink->setIconSize(QSize(24,24));
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

