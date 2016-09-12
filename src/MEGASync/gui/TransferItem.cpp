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

    type = -1;
    totalSize = 0;
    totalTransferredBytes = 0;
    transferSpeed = 0;
    meanTransferSpeed = 0;
    speedCounter = 0;
    regular = false;
    cancelButtonEnabled = false;
    isSyncTransfer = false;
    animation = NULL;
    priority = 0;
    transferState = 0;
    transferTag = 0;

    ui->lCancelTransfer->installEventFilter(this);
    ui->lCancelTransferCompleted->installEventFilter(this);
    update();
}

void TransferItem::setFileName(QString fileName)
{
    this->fileName = fileName;
    QFont f = ui->lTransferName->font();
    QFontMetrics fm = QFontMetrics(f);
    ui->lTransferNameCompleted->setText(fm.elidedText(fileName, Qt::ElideRight,ui->lTransferNameCompleted->width()));
    ui->lTransferNameCompleted->setToolTip(fileName);
    ui->lTransferName->setText(fm.elidedText(fileName, Qt::ElideRight,ui->lTransferName->width()));
    ui->lTransferName->setToolTip(fileName);

    QIcon icon;
    icon.addFile(Utilities::getExtensionPixmapSmall(fileName), QSize(), QIcon::Normal, QIcon::Off);
    ui->lFileType->setIcon(icon);
    ui->lFileType->setIconSize(QSize(20, 22));
    ui->lFileTypeCompleted->setIcon(icon);
    ui->lFileTypeCompleted->setIconSize(QSize(20, 22));
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
    this->isSyncTransfer = isSyncTransfer;
    QIcon icon;
    QPixmap loadIconResourceCompleted;

    if (isSyncTransfer)
    {
        this->loadIconResource = QPixmap(QString::fromUtf8(":/images/sync_item_ico.png"));
        ui->lActionTypeCompleted->setPixmap(loadIconResource);
        animation = new QMovie(QString::fromUtf8(":/images/synching.gif"));
        connect(animation, SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));
    }

    switch (type)
    {
        case MegaTransfer::TYPE_UPLOAD:

            if (!isSyncTransfer)
            {
                this->loadIconResource = QPixmap(QString::fromUtf8(":/images/cloud_upload_item_ico.png"));
                loadIconResourceCompleted = QPixmap(QString::fromUtf8(":/images/cloud_item_ico.png"));
                ui->lActionTypeCompleted->setPixmap(loadIconResourceCompleted);
                animation = new QMovie(QString::fromUtf8(":/images/uploading.gif"));
                connect(animation, SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));
            }

            icon.addFile(QString::fromUtf8(":/images/upload_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
                                                            "QProgressBar#pbTransfer::chunk {background-color: #2ba6de;}"));
            break;
        case MegaTransfer::TYPE_DOWNLOAD:

            if (!isSyncTransfer)
            {
                this->loadIconResource = QPixmap(QString::fromUtf8(":/images/cloud_download_item_ico.png"));
                loadIconResourceCompleted = QPixmap(QString::fromUtf8(":/images/cloud_item_ico.png"));
                ui->lActionTypeCompleted->setPixmap(loadIconResourceCompleted);
                animation = new QMovie(QString::fromUtf8(":/images/downloading.gif"));
                connect(animation, SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));
            }

            icon.addFile(QString::fromUtf8(":/images/download_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
                                                            "QProgressBar#pbTransfer::chunk {background-color: #31b500;}"));
            break;
        default:
            break;
    }

    ui->lTransferType->setIcon(icon);
    ui->lTransferType->setIconSize(QSize(12, 12));
    ui->lTransferTypeCompleted->setIcon(icon);
    ui->lTransferTypeCompleted->setIconSize(QSize(12, 12));
}

int TransferItem::getType()
{
    return type;
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
    if (animation)
    {
        delete animation;
    }
}
int TransferItem::getTransferState()
{
    return transferState;
}

void TransferItem::setTransferState(int value)
{
    transferState = value;
    switch (transferState)
    {
    case MegaTransfer::STATE_COMPLETED:
    case MegaTransfer::STATE_FAILED:
        finishTransfer();
        break;
    case MegaTransfer::STATE_CANCELLED:
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
    ui->sTransferState->setCurrentWidget(ui->stateCompleted);
    if (transferState == MegaTransfer::STATE_COMPLETED)
    {
        ui->lCompleted->setIcon(QIcon(QString::fromUtf8(":/images/completed_item_ico.png")));
    }
    else
    {
        ui->lCompleted->setIcon(QIcon(QString::fromUtf8(":/images/import_error_ico.png")));
    }
    ui->lCompleted->setIconSize(QSize(12, 12));
    ui->lTotalCompleted->setText(QString::fromUtf8("%1").arg(Utilities::getSizeString(totalSize)));
    ui->lRemainingTime->setText(QString::fromUtf8("00 m 00 s"));
}

void TransferItem::updateTransfer()
{
    ui->sTransferState->setCurrentWidget(ui->stateActive);
    updateAnimation();
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
            break;
        }
        case MegaTransfer::STATE_PAUSED:
            ui->lSpeed->setText(tr("(paused)"));
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        case MegaTransfer::STATE_QUEUED:
            ui->lSpeed->setText(tr("(queued)"));
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        case MegaTransfer::STATE_RETRYING:
            ui->lSpeed->setText(tr("(retrying)"));
            ui->lRemainingTime->setText(QString::fromUtf8(""));
            break;
        case MegaTransfer::STATE_COMPLETING:
            ui->lSpeed->setText(tr("(completing)"));
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

    // Update transferred bytes
    ui->lTotal->setText(QString::fromUtf8("%1%2").arg(!totalTransferredBytes ? QString::fromUtf8(""): QString::fromUtf8("%1<span style=\"color:#777777; text-decoration:none;\">&nbsp;&nbsp;of&nbsp;&nbsp;</span>").arg(Utilities::getSizeString(totalTransferredBytes)))
                        .arg(Utilities::getSizeString(totalSize)));
}

bool TransferItem::cancelButtonClicked(QPoint pos)
{
    if (!cancelButtonEnabled)
    {
        return false;
    }

    switch (transferState)
    {
    case MegaTransfer::STATE_COMPLETED:
    case MegaTransfer::STATE_FAILED:
        if (ui->lCancelTransferCompleted->rect().contains(ui->lCancelTransferCompleted->mapFrom(this, pos)))
        {
            return true;
        }
        break;
    case MegaTransfer::STATE_CANCELLED:
        break;
    default:
        if (ui->lCancelTransfer->rect().contains(ui->lCancelTransfer->mapFrom(this, pos)))
        {
            return true;
        }
        break;
    }

    return false;
}

void TransferItem::mouseHoverTransfer(bool isHover)
{
    if (isHover)
    {
        if (isSyncTransfer)
        {
            ui->lCancelTransfer->installEventFilter(this);
            ui->lCancelTransfer->update();
            return;
        }

        cancelButtonEnabled = true;
        ui->lCancelTransfer->removeEventFilter(this);
        ui->lCancelTransfer->update();
        ui->lCancelTransferCompleted->removeEventFilter(this);
        ui->lCancelTransferCompleted->update();
    }
    else
    {
        cancelButtonEnabled = false;
        ui->lCancelTransfer->installEventFilter(this);
        ui->lCancelTransfer->update();
        ui->lCancelTransferCompleted->installEventFilter(this);
        ui->lCancelTransferCompleted->update();
    }
}

bool TransferItem::eventFilter(QObject *, QEvent *ev)
{
    return ev->type() == QEvent::Paint || ev->type() == QEvent::ToolTip;
}

void TransferItem::frameChanged(int)
{
    emit animationChanged(this->getTransferTag());
}

void TransferItem::updateAnimation()
{
    if (!animation)
    {
        return;
    }

    switch (transferState)
    {
        case MegaTransfer::STATE_ACTIVE:
            if (animation->state() != QMovie::Running)
            {
                ui->lActionType->setMovie(animation);
                animation->start();
            }
            break;
        default:
            if (animation->state() != QMovie::NotRunning)
            {
                animation->stop();
                ui->lActionType->setMovie(NULL);
                ui->lActionType->setPixmap(loadIconResource);
            }
            break;
    }
}

QSize TransferItem::minimumSizeHint() const
{
    return QSize(720, 48);
}
QSize TransferItem::sizeHint() const
{
    return QSize(720, 48);
}
