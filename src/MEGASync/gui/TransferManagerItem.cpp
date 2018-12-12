#include "TransferManagerItem.h"
#include "ui_TransferManagerItem.h"
#include <QMouseEvent>
#include "megaapi.h"
#include "control/Utilities.h"
#include "Preferences.h"

using namespace mega;

TransferManagerItem::TransferManagerItem(QWidget *parent) :
    TransferItem(parent),
    ui(new Ui::TransferManagerItem)
{
    ui->setupUi(this);

    animation = NULL;

    // Choose the right icon for initial load (hdpi/normal displays)
    qreal ratio = 1.0;
#if QT_VERSION >= 0x050000
    ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
#endif
    loadIconResource = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_item_ico.png")
                                         : QString::fromUtf8(":/images/cloud_item_ico@2x.png"));
    ui->lActionType->setPixmap(loadIconResource);

    ui->lCancelTransfer->installEventFilter(this);
    ui->lCancelTransferCompleted->installEventFilter(this);
    update();
}

void TransferManagerItem::setFileName(QString fileName)
{
    TransferItem::setFileName(fileName);

    QFont f = ui->lTransferName->font();
    QFontMetrics fm = QFontMetrics(f);
    ui->lTransferNameCompleted->setText(fm.elidedText(fileName, Qt::ElideRight,ui->lTransferNameCompleted->width()));
    ui->lTransferNameCompleted->setToolTip(fileName);
    ui->lTransferName->setText(fm.elidedText(fileName, Qt::ElideRight,ui->lTransferName->width()));
    ui->lTransferName->setToolTip(fileName);

    QIcon icon;
    icon.addFile(Utilities::getExtensionPixmapSmall(fileName), QSize(), QIcon::Normal, QIcon::Off);
    ui->lFileType->setIcon(icon);
    ui->lFileType->setIconSize(QSize(24, 24));
    ui->lFileTypeCompleted->setIcon(icon);
    ui->lFileTypeCompleted->setIconSize(QSize(24, 24));
}

void TransferManagerItem::setStateLabel(QString labelState)
{
    ui->lSpeed->setText(QString::fromUtf8("(%1)").arg(labelState));
    ui->lRemainingTime->setText(QString::fromUtf8(""));
}

QString TransferManagerItem::getTransferName()
{
    if (ui->sTransferState->currentWidget() == ui->stateCompleted)
    {
        return ui->lTransferNameCompleted->text();
    }
    else
    {
        return ui->lTransferName->text();
    }
}

void TransferManagerItem::setType(int type, bool isSyncTransfer)
{
    TransferItem::setType(type, isSyncTransfer);
    QIcon icon;

    qreal ratio = 1.0;
#if QT_VERSION >= 0x050000
    ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
#endif

    if (isSyncTransfer)
    {
        this->loadIconResource = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/sync_item_ico.png")
                                                   : QString::fromUtf8(":/images/sync_item_ico@2x.png"));
        delete animation;
        animation = new QMovie(ratio < 2 ? QString::fromUtf8(":/images/synching.gif")
                                         : QString::fromUtf8(":/images/synching@2x.gif"));
        connect(animation, SIGNAL(frameChanged(int)), this, SLOT(frameChanged(int)));
    }
    else
    {
        this->loadIconResource = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_item_ico.png")
                                                   : QString::fromUtf8(":/images/cloud_item_ico@2x.png"));
    }
    ui->lActionTypeCompleted->setPixmap(loadIconResource);
    ui->lActionType->setPixmap(loadIconResource);

    switch (type)
    {
        case MegaTransfer::TYPE_UPLOAD:

            if (!isSyncTransfer)
            {
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


TransferManagerItem::~TransferManagerItem()
{
    delete ui;
    delete animation;
}

void TransferManagerItem::setTransferState(int value)
{
    TransferItem::setTransferState(value);
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

void TransferManagerItem::finishTransfer()
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

void TransferManagerItem::updateTransfer()
{
    ui->sTransferState->setCurrentWidget(ui->stateActive);
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
            if (transferError == MegaError::API_EOVERQUOTA)
            {
                ui->lSpeed->setText(QString::fromUtf8("(%1)").arg(tr("Out of storage space")));
            }
            else
            {
                ui->lSpeed->setText(QString::fromUtf8("(%1)").arg(tr("retrying")));
            }

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

void TransferManagerItem::updateFinishedTime()
{
    if (!dsFinishedTime)
    {
        return;
    }

    Preferences *preferences = Preferences::instance();
    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = ( now.toMSecsSinceEpoch() / 100 - (preferences->getMsDiffTimeWithSDK() + dsFinishedTime) ) / 10;
    ui->lRemainingTimeCompleted->setText(Utilities::getFinishedTimeString(secs));
}

bool TransferManagerItem::cancelButtonClicked(QPoint pos)
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

bool TransferManagerItem::mouseHoverRetryingLabel(QPoint pos)
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

void TransferManagerItem::mouseHoverTransfer(bool isHover)
{
    if (isHover)
    {
        if (isSyncTransfer && !(transferState == MegaTransfer::STATE_COMPLETED || transferSpeed == MegaTransfer::STATE_FAILED))
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

void TransferManagerItem::loadDefaultTransferIcon()
{
    if (animation && animation->state() != QMovie::NotRunning)
    {
        animation->stop();
        ui->lActionType->setMovie(NULL);
        ui->lActionType->setPixmap(loadIconResource);
    }
}

bool TransferManagerItem::eventFilter(QObject *, QEvent *ev)
{
    return ev->type() == QEvent::Paint || ev->type() == QEvent::ToolTip;
}

void TransferManagerItem::frameChanged(int)
{
    emit refreshTransfer(this->getTransferTag());
}

void TransferManagerItem::updateAnimation()
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

QSize TransferManagerItem::minimumSizeHint() const
{
    return QSize(800, 48);
}
QSize TransferManagerItem::sizeHint() const
{
    return QSize(800, 48);
}
