#include "TransferItem.h"
#include "ui_TransferItem.h"
#include <QMouseEvent>
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"

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
    dsFinishedTime = 0;

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

void TransferItem::setFinishedTime(long long time)
{
    dsFinishedTime = time;
}

void TransferItem::setType(int type, bool isSyncTransfer)
{
    this->type = type;
    this->isSyncTransfer = isSyncTransfer;
    QIcon icon;
    QPixmap loadIconResourceCompleted;

    qreal ratio = 1.0;
#if QT_VERSION >= 0x050000
    ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
#endif

    if (isSyncTransfer)
    {
        this->loadIconResource = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/sync_item_ico.png")
                                                   : QString::fromUtf8(":/images/sync_item_ico@2x.png"));
        ui->lActionTypeCompleted->setPixmap(loadIconResource);

        delete animation;
        animation = new QMovie(ratio < 2 ? QString::fromUtf8(":/images/synching.gif")
                                         : QString::fromUtf8(":/images/synching@2x.gif"));
        connect(animation, SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));
    }

    switch (type)
    {
        case MegaTransfer::TYPE_UPLOAD:

            if (!isSyncTransfer)
            {
                this->loadIconResource = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_upload_item_ico.png")
                                                           : QString::fromUtf8(":/images/cloud_upload_item_ico@2x.png"));
                loadIconResourceCompleted = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_item_ico.png")
                                                              : QString::fromUtf8(":/images/cloud_item_ico@2x.png"));
                ui->lActionTypeCompleted->setPixmap(loadIconResourceCompleted);

                delete animation;
                animation = new QMovie(ratio < 2 ? QString::fromUtf8(":/images/uploading.gif")
                                                 : QString::fromUtf8(":/images/uploading@2x.gif"));
                connect(animation, SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));
            }

            icon.addFile(QString::fromUtf8(":/images/upload_item_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
            ui->pbTransfer->setStyleSheet(QString::fromUtf8("QProgressBar#pbTransfer{background-color: #ececec;}"
                                                            "QProgressBar#pbTransfer::chunk {background-color: #2ba6de;}"));
            break;
        case MegaTransfer::TYPE_DOWNLOAD:

            if (!isSyncTransfer)
            {
                this->loadIconResource = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_download_item_ico.png")
                                                           : QString::fromUtf8(":/images/cloud_download_item_ico@2x.png"));
                loadIconResourceCompleted = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_item_ico.png")
                                                              : QString::fromUtf8(":/images/cloud_item_ico@2x.png"));
                ui->lActionTypeCompleted->setPixmap(loadIconResourceCompleted);

                delete animation;
                animation = new QMovie(ratio < 2 ? QString::fromUtf8(":/images/downloading.gif")
                                                 : QString::fromUtf8(":/images/downloading@2x.gif"));
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
    delete animation;
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
            QString pattern(QString::fromUtf8("(%1/s)"));
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

    // Update transferred bytes
    ui->lTotal->setText(QString::fromUtf8("%1%2").arg(!totalTransferredBytes ? QString::fromUtf8(""): QString::fromUtf8("%1<span style=\"color:#777777; text-decoration:none;\">&nbsp;&nbsp;of&nbsp;&nbsp;</span>").arg(Utilities::getSizeString(totalTransferredBytes)))
                        .arg(Utilities::getSizeString(totalSize)));
}

void TransferItem::updateFinishedTime()
{
    if (!dsFinishedTime)
    {
        return;
    }

    Preferences *preferences = Preferences::instance();
    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = ( now.toMSecsSinceEpoch() / 100 - (preferences->getMsDiffTimeWithSDK() + dsFinishedTime) ) / 10;
    if (secs < 2)
    {
        ui->lRemainingTimeCompleted->setText(tr("just now"));
    }
    else if (secs < 60)
    {
        ui->lRemainingTimeCompleted->setText(tr("%1 seconds ago").arg(secs));
    }
    else if (secs < 3600)
    {
        int minutes = secs/60;
        if (minutes == 1)
        {
            ui->lRemainingTimeCompleted->setText(tr("1 minute ago"));
        }
        else
        {
            ui->lRemainingTimeCompleted->setText(tr("%1 minutes ago").arg(minutes));
        }
    }
    else if (secs < 86400)
    {
        int hours = secs/3600;
        if (hours == 1)
        {
            ui->lRemainingTimeCompleted->setText(tr("1 hour ago"));
        }
        else
        {
            ui->lRemainingTimeCompleted->setText(tr("%1 hours ago").arg(hours));
        }
    }
    else if (secs < 2592000)
    {
        int days = secs/86400;
        if (days == 1)
        {
            ui->lRemainingTimeCompleted->setText(tr("1 day ago"));
        }
        else
        {
            ui->lRemainingTimeCompleted->setText(tr("%1 days ago").arg(days));
        }
    }
    else if (secs < 31536000)
    {
        int months = secs/2592000;
        if (months == 1)
        {
            ui->lRemainingTimeCompleted->setText(tr("1 month ago"));
        }
        else
        {
            ui->lRemainingTimeCompleted->setText(tr("%1 months ago").arg(months));
        }
    }
    else
    {
        int years = secs/31536000;
        if (years == 1)
        {
            ui->lRemainingTimeCompleted->setText(tr("1 year ago"));
        }
        else
        {
            ui->lRemainingTimeCompleted->setText(tr("%1 years ago").arg(years));
        }
    }
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

    emit refreshTransfer(this->getTransferTag());
}

bool TransferItem::eventFilter(QObject *, QEvent *ev)
{
    return ev->type() == QEvent::Paint || ev->type() == QEvent::ToolTip;
}

void TransferItem::frameChanged(int)
{
    emit refreshTransfer(this->getTransferTag());
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
    return QSize(800, 48);
}
QSize TransferItem::sizeHint() const
{
    return QSize(800, 48);
}
