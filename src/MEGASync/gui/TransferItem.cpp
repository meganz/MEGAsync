#include "TransferItem.h"
#include "ui_TransferItem.h"
#include "control/Utilities.h"
#include <QMouseEvent>
#include "megaapi.h"

using namespace mega;

TransferItem::TransferItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransferItem)
{
    ui->setupUi(this);
    ui->lCancelTransfer->setHidden(true);

    totalSize = totalTransferredBytes = elapsedTransferTime = 0;
    transferSpeed = 0;
    effectiveTransferSpeed = 200000;
    regular = false;
    lastUpdate = QDateTime::currentMSecsSinceEpoch();
}

void TransferItem::setFileName(QString fileName)
{
    this->fileName = fileName;
    QFont f = ui->lTransferName->font();
    QFontMetrics fm = QFontMetrics(f);
    ui->lTransferName->setText(fm.elidedText(fileName, Qt::ElideRight,ui->lTransferName->width()));
    ui->lTransferName->setToolTip(fileName);

    QIcon icon;
    icon.addFile(Utilities::getExtensionPixmapSmall(fileName), QSize(), QIcon::Normal, QIcon::Off);
    ui->lFileType->setIcon(icon);
    ui->lFileType->setIconSize(QSize(20, 22));
}

void TransferItem::setTransferredBytes(long long totalTransferredBytes, bool cancellable)
{
    this->totalTransferredBytes = totalTransferredBytes;
    regular = cancellable;
}

void TransferItem::setSpeed(long long transferSpeed)
{
    this->transferSpeed = transferSpeed;
}

void TransferItem::setTotalSize(long long size)
{
    this->totalSize = size;
}

void TransferItem::setType(int type, bool isSyncTransfer)
{
    this->type = type;
    QIcon icon;

    if (type)
    {
        icon.addFile(QString::fromUtf8(":/images/upload_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
        QString loadIconResource = isSyncTransfer ? QString::fromUtf8(":/images/sync_item_ico.png")
                                                  : QString::fromUtf8(":/images/cloud_upload_item_ico.png");

        ui->lActionType->setIcon(QIcon(loadIconResource));
        ui->lActionType->setIconSize(QSize(32, 32));

        ui->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
                                                        "QProgressBar#pbTransfer::chunk {background-color: #2ba6de;}"));

    }
    else
    {
        icon.addFile(QString::fromUtf8(":/images/download_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
        QString loadIconResource = isSyncTransfer ? QString::fromUtf8(":/images/sync_item_ico.png")
                                                  : QString::fromUtf8(":/images/cloud_download_item_ico.png");

        ui->lActionType->setIcon(QIcon(loadIconResource));
        ui->lActionType->setIconSize(QSize(32, 32));

        ui->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
                                                        "QProgressBar#pbTransfer::chunk {background-color: #31b500;}"));
    }

#ifndef Q_OS_LINUX
    ui->lTransferType->setIcon(icon);
    ui->lTransferType->setIconSize(QSize(12, 12));
#else
    ui->lTransferType->setPixmap(icon.pixmap(QSize(12, 12)));
#endif

}

TransferItem::~TransferItem()
{
    delete ui;
}

void TransferItem::finishTransfer()
{

    ui->lTotal->setText(QString::fromUtf8("%1").arg(Utilities::getSizeString(totalSize)));
    ui->lCompleted->setIcon(QIcon(QString::fromUtf8(":/images/completed_item_ico.png")));
    ui->lCompleted->setIconSize(QSize(12, 12));
    ui->pbTransfer->setVisible(false);
    ui->lSpeed->setEnabled(false);
    ui->lRemainingTime->setText(QString::fromUtf8(""));

}
void TransferItem::updateTransfer()
{
    // Update remaining time
    long long remainingBytes = totalSize - totalTransferredBytes;
    if (remainingBytes < 0)
    {
        remainingBytes = 0;
    }

    unsigned long long timeIncrement = QDateTime::currentMSecsSinceEpoch()-lastUpdate;
    if (timeIncrement < 1000)
    {
        elapsedTransferTime += timeIncrement;
    }

    double effectiveSpeed = effectiveTransferSpeed;
    double elapsedDownloadTimeSecs = elapsedTransferTime/1000.0;
    if (elapsedDownloadTimeSecs)
    {
        effectiveSpeed = totalTransferredBytes/elapsedDownloadTimeSecs;
    }

    effectiveTransferSpeed += (effectiveSpeed-effectiveTransferSpeed)/3; //Smooth the effective speed

    int totalRemainingSeconds = (effectiveTransferSpeed) ? remainingBytes/effectiveTransferSpeed : 0;

    QString remainingTime;
    if (totalRemainingSeconds)
    {
        remainingTime = Utilities::getTimeString(totalRemainingSeconds);
    }
    else
    {
        remainingTime = QString::fromAscii("--:--:--");
    }

    ui->lRemainingTime->setText(remainingTime);

    // Update current transfer speed
    QString pattern(tr("(%1/s)"));
    QString pausedPattern(tr("(paused)"));
    QString invalidSpeedPattern(tr(""));
    QString downloadString;

    if (transferSpeed >= 20000)
    {
        downloadString = pattern.arg(Utilities::getSizeString(transferSpeed));
    }
    else if (transferSpeed >= 0)
    {
        downloadString = invalidSpeedPattern;
    }
    else
    {
        downloadString = pausedPattern;
    }

    ui->lSpeed->setText(downloadString);

    lastUpdate = QDateTime::currentMSecsSinceEpoch();

    // Update progress bar
    unsigned int permil = 0;

    if ((totalSize > 0) && (totalTransferredBytes > 0))
    {
        permil = (1000 * totalTransferredBytes) / totalSize;
    }

    if (permil > 1000)
    {
        permil = 1000;
    }

    ui->pbTransfer->setValue(permil);

    // Update transferred bytes
    ui->lTotal->setText(QString::fromUtf8("%1   <span style=\"color:#777777; text-decoration:none;\">of   </span>%2").arg(Utilities::getSizeString(totalTransferredBytes))
                        .arg(Utilities::getSizeString(totalSize)));
}

void TransferItem::mouseEventClicked(QPoint pos, bool rightClick)
{
}

void TransferItem::mouseHoverTransfer(bool isHover)
{
    ui->lCancelTransfer->setHidden(!isHover);
}

QSize TransferItem::minimumSizeHint() const
{
    return QSize(720, 48);
}
QSize TransferItem::sizeHint() const
{
    return QSize(720, 48);
}

