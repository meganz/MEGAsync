#include "RecentFile.h"
#include "ui_RecentFile.h"
#include "MegaApplication.h"
#include "control/Utilities.h"
#include "platform/Platform.h"

#include <QImageReader>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

RecentFile::RecentFile(QWidget *parent) :
    TransferItem(parent),
    ui(new Ui::RecentFile)
{
    ui->setupUi(this);

    getLinkButtonEnabled = false;

    ui->lGetLink->installEventFilter(this);
    update();
}

RecentFile::~RecentFile()
{
    delete ui;
}

void RecentFile::setFileName(QString fileName)
{
    TransferItem::setFileName(fileName);

    QFont f = ui->lFileName->font();
    QFontMetrics fm = QFontMetrics(f);
    ui->lFileName->setText(fm.elidedText(fileName, Qt::ElideRight,ui->lFileName->width()));
    ui->lFileName->setToolTip(fileName);
    ui->lFileNameCompleted->setText(fm.elidedText(fileName, Qt::ElideRight,ui->lFileName->width()));
    ui->lFileNameCompleted->setToolTip(fileName);


    QIcon icon;
    icon.addFile(Utilities::getExtensionPixmapMedium(fileName), QSize(), QIcon::Normal, QIcon::Off);
    ui->lFileType->setIcon(icon);
    ui->lFileType->setIconSize(QSize(48, 48));
    ui->lFileTypeCompleted->setIcon(icon);
    ui->lFileTypeCompleted->setIconSize(QSize(48, 48));
}

void RecentFile::setType(int type, bool isSyncTransfer)
{
    TransferItem::setType(type, isSyncTransfer);
    QIcon icon;

    qreal ratio = 1.0;
#if QT_VERSION >= 0x050000
    ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
#endif

    switch (type)
    {
        case MegaTransfer::TYPE_UPLOAD:
            icon.addFile(QString::fromUtf8(":/images/upload_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
            break;
        case MegaTransfer::TYPE_DOWNLOAD:
            icon.addFile(QString::fromUtf8(":/images/download_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
            break;
        default:
            break;
    }

    ui->lTransferType->setIcon(icon);
    ui->lTransferType->setIconSize(QSize(12, 12));
    ui->lTransferTypeCompleted->setIcon(icon);
    ui->lTransferTypeCompleted->setIconSize(QSize(12, 12));
}

void RecentFile::setTransferState(int value)
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

QString RecentFile::getTransferName()
{
    return ui->lFileName->text();
}

bool RecentFile::getLinkButtonClicked(QPoint pos)
{
    if (!getLinkButtonEnabled)
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

void RecentFile::mouseHoverTransfer(bool isHover)
{
    if (isHover)
    {
        getLinkButtonEnabled = true;
        ui->lGetLink->removeEventFilter(this);
        ui->lGetLink->update();
    }
    else
    {
        getLinkButtonEnabled = false;
        ui->lGetLink->installEventFilter(this);
        ui->lGetLink->update();
    }

    emit refreshTransfer(this->getTransferTag());
}

void RecentFile::finishTransfer()
{
    ui->sTransferState->setCurrentWidget(ui->completedTransfer);
}

void RecentFile::updateTransfer()
{
    if (transferState != MegaTransfer::STATE_ACTIVE)
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
            }
            else
            {
                remainingTime = QString::fromAscii("");
            }
            ui->lRemainingTime->setText(remainingTime);

            // Update current transfer speed
            QString downloadString;

            if (!totalTransferredBytes)
            {
                downloadString = QString::fromUtf8("(%1)").arg(tr("starting"));
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
            ui->lSpeed->setText(QString::fromUtf8("(%1)").arg(tr("paused")));
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        case MegaTransfer::STATE_QUEUED:
            ui->lSpeed->setText(QString::fromUtf8("(%1)").arg(tr("queued")));
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        case MegaTransfer::STATE_RETRYING:
            ui->lSpeed->setText(QString::fromUtf8("(%1)").arg(tr("retrying")));
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        case MegaTransfer::STATE_COMPLETING:
            ui->lSpeed->setText(QString::fromUtf8("(%1)").arg(tr("completing")));
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        default:
            ui->lSpeed->setText(QString::fromUtf8(""));
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            break;
    }

    // Update progress bar
    unsigned int permil = (totalSize > 0) ? ((1000 * totalTransferredBytes) / totalSize) : 0;
    ui->pbTransfer->setValue(permil);
}

bool RecentFile::eventFilter(QObject *, QEvent *ev)
{
    return ev->type() == QEvent::Paint || ev->type() == QEvent::ToolTip;
}

QSize RecentFile::minimumSizeHint() const
{
    return QSize(400, 60);
}
QSize RecentFile::sizeHint() const
{
    return QSize(400, 60);
}

