#include "TransferItem.h"
#include "ui_TransferItem.h"
#include "control/Utilities.h"
#include <QMouseEvent>
#include <QDebug>

TransferItem::TransferItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransferItem)
{
    ui->setupUi(this);
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
    ui->lTransferName->setStyleSheet(QString::fromUtf8("color: grey;"));
    ui->lTransferName->setText(fm.elidedText(fileName, Qt::ElideRight,ui->lTransferName->width()));

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

void TransferItem::setType(int type)
{
    this->type = type;
    QIcon icon;
    if (type)
    {
        icon.addFile(QString::fromUtf8(":/images/tray_upload_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
    }
    else
    {
        icon.addFile(QString::fromUtf8(":/images/tray_download_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
    }

#ifndef Q_OS_LINUX
    ui->lTransferType->setIcon(icon);
    ui->lTransferType->setIconSize(QSize(16, 16));
#else
    ui->lTransferType->setPixmap(icon.pixmap(QSize(16, 16)));
#endif

}

TransferItem::~TransferItem()
{
    delete ui;
}

void TransferItem::finishTransfer()
{

    ui->lTotal->setText(QString::fromUtf8("%1").arg(Utilities::getSizeString(totalSize)));
    ui->lSpeed->setText(tr("completed"));
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
    int remainingHours = totalRemainingSeconds/3600;
    if ((remainingHours<0) || (remainingHours>99))
    {
        totalRemainingSeconds = 0;
    }

    int remainingMinutes = (totalRemainingSeconds%3600)/60;
    int remainingSeconds =  (totalRemainingSeconds%60);
    QString remainingTime;
    if (totalRemainingSeconds)
    {
        remainingTime = QString::fromAscii("%1:%2:%3").arg(remainingHours, 2, 10, QChar::fromAscii('0'))
            .arg(remainingMinutes, 2, 10, QChar::fromAscii('0'))
            .arg(remainingSeconds, 2, 10, QChar::fromAscii('0'));
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
    ui->lTotal->setText(QString::fromUtf8("%1 of %2").arg(Utilities::getSizeString(totalTransferredBytes))
                        .arg(Utilities::getSizeString(totalSize)));
}

void TransferItem::mouseEventClicked(QPoint pos, bool rightClick)
{
}

