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

    totalSize = 0;
    totalTransferredBytes = 0;
    transferSpeed = 0;
    meanTransferSpeed = 0;
    speedCounter = 0;
    regular = false;
    cancelButtonEnabled = false;
    animation = NULL;
    priority = 0;
    transferState = 0;
    transferTag = 0;

    ui->lCancelTransfer->installEventFilter(this);
    update();
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

QString TransferItem::getFileName()
{
    return fileName;
}

void TransferItem::setTransferredBytes(long long totalTransferredBytes, bool cancellable)
{
    this->totalTransferredBytes = totalTransferredBytes;
    if (this->totalTransferredBytes < 0)
    {
        this->totalTransferredBytes = 0;
    }
    if (this->totalTransferredBytes > this->totalSize)
    {
        this->totalTransferredBytes = this->totalSize;
    }
    regular = cancellable;
}

void TransferItem::setSpeed(long long transferSpeed)
{
    this->transferSpeed = transferSpeed;
    if (this->transferSpeed < 0)
    {
        this->transferSpeed = 0;
    }

    meanTransferSpeed = meanTransferSpeed * speedCounter + this->transferSpeed;
    speedCounter++;
    meanTransferSpeed /= speedCounter;
}

void TransferItem::setTotalSize(long long size)
{
    this->totalSize = size;
    if (this->totalSize < 0)
    {
        this->totalSize = 0;
    }
    if (this->totalTransferredBytes > this->totalSize)
    {
        this->totalTransferredBytes = this->totalSize;
    }
}

void TransferItem::setType(int type, bool isSyncTransfer)
{
    this->type = type;
    QIcon icon;

    if (type)
    {
        icon.addFile(QString::fromUtf8(":/images/upload_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
        QPixmap loadIconResource = isSyncTransfer ? QPixmap(QString::fromUtf8(":/images/sync_item_ico.png"))
                                                  : QPixmap(QString::fromUtf8(":/images/cloud_upload_item_ico.png"));

        ui->lActionType->setPixmap(loadIconResource);
        ui->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
                                                        "QProgressBar#pbTransfer::chunk {background-color: #2ba6de;}"));
    }
    else
    {
        if (!animation)
        {
            animation = new QMovie(QString::fromUtf8(":/images/downloading.gif"));
            ui->lActionType->setMovie(animation);
            animation->start();
        }

        icon.addFile(QString::fromUtf8(":/images/download_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
        ui->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
                                                        "QProgressBar#pbTransfer::chunk {background-color: #31b500;}"));
    }

    ui->lTransferType->setIcon(icon);
    ui->lTransferType->setIconSize(QSize(12, 12));
}

void TransferItem::setPriority(unsigned long long priority)
{
    this->priority = priority;
}

unsigned long long TransferItem::getPriority()
{
    return this->priority;
}

TransferItem::~TransferItem()
{
    delete ui;
    delete animation;
}
int TransferItem::getTransferState()
{
    return transferState;
}

void TransferItem::setTransferState(int value)
{
    transferState = value;
    switch (transferState) {
    case MegaTransfer::STATE_COMPLETED:
        finishTransfer();
        break;
    case MegaTransfer::STATE_PAUSED:
        pauseTransfer();
        break;
    default:
        updateTransfer();
        break;
    }
}
int TransferItem::getTransferTag()
{
    return transferTag;
}

void TransferItem::setTransferTag(int value)
{
    transferTag = value;
}
bool TransferItem::getRegular()
{
    return regular;
}

void TransferItem::setRegular(bool value)
{
    regular = value;
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
    int totalRemainingSeconds = meanTransferSpeed ? remainingBytes / meanTransferSpeed : 0;

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
    QString downloadString;
    if (meanTransferSpeed >= 20000)
    {
        downloadString = pattern.arg(Utilities::getSizeString(transferSpeed));
    }
    else
    {
        downloadString = QString::fromUtf8("");
    }
    ui->lSpeed->setText(downloadString);

    // Update progress bar
    unsigned int permil = (totalSize > 0) ? ((1000 * totalTransferredBytes) / totalSize) : 0;
    ui->pbTransfer->setValue(permil);

    // Update transferred bytes
    ui->lTotal->setText(QString::fromUtf8("%1   <span style=\"color:#777777; text-decoration:none;\">of   </span>%2").arg(Utilities::getSizeString(totalTransferredBytes))
                        .arg(Utilities::getSizeString(totalSize)));
}

void TransferItem::pauseTransfer()
{
    transferState == MegaTransfer::STATE_PAUSED ?  ui->lSpeed->setText(QString::fromUtf8("(paused)")) : ui->lSpeed->setText(QString::fromUtf8(""));
}

bool TransferItem::cancelButtonClicked(QPoint pos)
{
    if (cancelButtonEnabled && ui->lCancelTransfer->rect().contains(ui->lCancelTransfer->mapFrom(this, pos)))
    {
        return true;
    }
    return false;
}

void TransferItem::mouseHoverTransfer(bool isHover)
{
    if (isHover)
    {
        cancelButtonEnabled = true;
        ui->lCancelTransfer->removeEventFilter(this);
        ui->lCancelTransfer->update();
    }
    else
    {
        cancelButtonEnabled = false;
        ui->lCancelTransfer->installEventFilter(this);
        ui->lCancelTransfer->update();
    }
}

bool TransferItem::eventFilter(QObject *, QEvent *ev)
{
    return ev->type() == QEvent::Paint || ev->type() == QEvent::ToolTip;
}

QSize TransferItem::minimumSizeHint() const
{
    return QSize(720, 48);
}
QSize TransferItem::sizeHint() const
{
    return QSize(720, 48);
}
