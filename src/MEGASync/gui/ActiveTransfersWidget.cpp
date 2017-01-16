#include "ActiveTransfersWidget.h"
#include "ui_ActiveTransfersWidget.h"
#include "control/Utilities.h"
#include "Preferences.h"
#include <QMessageBox>

using namespace mega;

ActiveTransfersWidget::ActiveTransfersWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActiveTransfersWidget)
{
    ui->setupUi(this);
    activeDownload.clear();
    animationDown = animationUp = NULL;

    ui->bDownPaused->setVisible(false);
    ui->bUpPaused->setVisible(false);
    ui->sDownloads->setCurrentWidget(ui->wNoDownloads);
    ui->sUploads->setCurrentWidget(ui->wNoUploads);
    ui->sTransfersContainer->setCurrentWidget(ui->pNoTransfers);
    ui->bGraphsSeparator->setStyleSheet(QString::fromAscii("background-color: transparent; "
                                                           "border: none; "));
}

void ActiveTransfersWidget::init(MegaApi *megaApi, MegaTransfer *activeUpload, MegaTransfer *activeDownload)
{
    this->megaApi = megaApi;
    ui->wDownGraph->init(megaApi,MegaTransfer::TYPE_DOWNLOAD);
    ui->wUpGraph->init(megaApi,MegaTransfer::TYPE_UPLOAD);

    connect(ui->wDownGraph, SIGNAL(newValue(long long)), this, SLOT(updateDownSpeed(long long)));
    connect(ui->wUpGraph, SIGNAL(newValue(long long)), this, SLOT(updateUpSpeed(long long)));
    ui->wDownGraph->start();
    ui->wUpGraph->start();

    updateNumberOfTransfers(megaApi);
    if (activeUpload)
    {
        updateTransferInfo(activeUpload);
    }

    if (activeDownload)
    {
        updateTransferInfo(activeDownload);
    }
}

ActiveTransfersWidget::~ActiveTransfersWidget()
{
    delete ui;
    delete animationDown;
    delete animationUp;
}

void ActiveTransfersWidget::onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer)
{
    updateNumberOfTransfers(api);
    updateTransferInfo(transfer);
}

void ActiveTransfersWidget::onTransferFinish(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError *e)
{
    if (transfer->isStreamingTransfer() || transfer->isFolderTransfer())
    {
        return;
    }

    updateNumberOfTransfers(api);

    int type = transfer->getType();
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        activeDownload.clear();
    }
    else
    {
        activeUpload.clear();
    }

    MegaTransfer *nextTransfer = megaApi->getFirstTransfer(type);
    if (nextTransfer)
    {
        onTransferUpdate(api, nextTransfer);
        delete nextTransfer;
    }
}

void ActiveTransfersWidget::onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer)
{
    if (transfer->isStreamingTransfer() || transfer->isFolderTransfer())
    {
        return;
    }

    updateNumberOfTransfers(api);
    updateTransferInfo(transfer);
}

void ActiveTransfersWidget::onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError *e)
{

}

void ActiveTransfersWidget::updateTransferInfo(MegaTransfer *transfer)
{
    if (transfer->isStreamingTransfer() || transfer->isFolderTransfer())
    {
        return;
    }

    int type = transfer->getType();
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        if (transfer->getPriority() > activeDownload.priority
                && !(activeDownload.transferState == MegaTransfer::STATE_PAUSED))
        {
            return;
        }

        // New Download transfer, update name, size and tag
        if (activeDownload.tag != transfer->getTag())
        {           
            activeDownload.tag = transfer->getTag();
            setType(&activeDownload, type, transfer->isSyncTransfer());
            activeDownload.fileName = QString::fromUtf8(transfer->getFileName());
            QFont f = ui->lDownFilename->font();
            QFontMetrics fm = QFontMetrics(f);
            ui->lDownFilename->setText(fm.elidedText(activeDownload.fileName, Qt::ElideRight,ui->lDownFilename->width()));
            ui->lDownFilename->setToolTip(activeDownload.fileName);
            QIcon icon;
            icon.addFile(Utilities::getExtensionPixmapSmall(activeDownload.fileName), QSize(), QIcon::Normal, QIcon::Off);
            ui->bDownFileType->setIcon(icon);
            setTotalSize(&activeDownload, transfer->getTotalBytes());
        }

        activeDownload.transferState = transfer->getState();
        activeDownload.priority = transfer->getPriority();
        activeDownload.meanTransferSpeed = transfer->getMeanSpeed();
        setSpeed(&activeDownload, transfer->getSpeed());
        setTransferredBytes(&activeDownload, transfer->getTransferredBytes());
        udpateTransferState(&activeDownload);
    }
    else
    {
        if (transfer->getPriority() > activeUpload.priority)
        {
            return;
        }

        // New Upload transfer, update name, size and tag
        if (activeUpload.tag != transfer->getTag())
        {
            activeUpload.tag = transfer->getTag();
            setType(&activeUpload, type, transfer->isSyncTransfer());
            activeUpload.fileName = QString::fromUtf8(transfer->getFileName());
            QFont f = ui->lUpFilename->font();
            QFontMetrics fm = QFontMetrics(f);
            ui->lUpFilename->setText(fm.elidedText(activeUpload.fileName, Qt::ElideRight,ui->lUpFilename->width()));
            ui->lUpFilename->setToolTip(activeUpload.fileName);
            QIcon icon;
            icon.addFile(Utilities::getExtensionPixmapSmall(activeUpload.fileName), QSize(), QIcon::Normal, QIcon::Off);
            ui->bUpFileType->setIcon(icon);
            setTotalSize(&activeUpload, transfer->getTotalBytes());
        }

        activeUpload.transferState = transfer->getState();
        activeUpload.priority = transfer->getPriority();
        activeUpload.meanTransferSpeed = transfer->getMeanSpeed();
        setSpeed(&activeUpload, transfer->getSpeed());
        setTransferredBytes(&activeUpload, transfer->getTransferredBytes());
        udpateTransferState(&activeUpload);
    }
}

void ActiveTransfersWidget::pausedDownTransfers(bool paused)
{
    if (paused)
    {
        QString remainingTime = QString::fromUtf8("- <span style=\"color:#777777; text-decoration:none;\">m</span> - <span style=\"color:#777777; text-decoration:none;\">s</span>");
        if (totalDownloads)
        {
            ui->bDownPaused->setVisible(true);
            ui->lDownRemainingTime->setText(remainingTime);
            ui->lCurrentDownSpeed->setText(tr("PAUSED"));
        }
    }
}

bool ActiveTransfersWidget::areTransfersActive()
{
    return totalDownloads || totalUploads;
}

void ActiveTransfersWidget::pausedUpTransfers(bool paused)
{
    if (paused)
    {
        QString remainingTime = QString::fromUtf8("- <span style=\"color:#777777; text-decoration:none;\">m</span> - <span style=\"color:#777777; text-decoration:none;\">s</span>");
        if (totalUploads)
        {
            ui->bUpPaused->setVisible(true);
            ui->lUpRemainingTime->setText(remainingTime);
            ui->lCurrentUpSpeed->setText(tr("PAUSED"));
        }
    }
}

void ActiveTransfersWidget::updateDownSpeed(long long speed)
{
    if (Preferences::instance()->getDownloadsPaused())
    {
        pausedDownTransfers(true);
        return;
    }

    ui->bDownPaused->setVisible(false);
    if (activeDownload.transferState == MegaTransfer::STATE_PAUSED)
    {
        ui->lCurrentDownSpeed->setText(tr("PAUSED"));
    }
    else
    {
        if (!speed)
        {
            ui->lCurrentDownSpeed->setText(QString::fromUtf8(""));
        }
        else
        {
            ui->lCurrentDownSpeed->setText(QString::fromUtf8("%1 %2")
                                           .arg(QString::fromUtf8("<span style=\"color:#666666; font-size: 28px; text-decoration:none;\">%1</span>")
                                           .arg(Utilities::getSizeString(speed).split(QString::fromUtf8(" ")).at(0)))
                                           .arg(Utilities::getSizeString(speed).split(QString::fromUtf8(" ")).at(1) + QString::fromUtf8("/s")));
        }
    }
}

void ActiveTransfersWidget::updateUpSpeed(long long speed)
{
    if (Preferences::instance()->getUploadsPaused())
    {
        pausedUpTransfers(true);
        return;
    }

    ui->bUpPaused->setVisible(false);
    if (activeUpload.transferState == MegaTransfer::STATE_PAUSED)
    {
        ui->lCurrentUpSpeed->setText(tr("PAUSED"));
    }
    else
    {
        if (!speed)
        {
            ui->lCurrentUpSpeed->setText(QString::fromUtf8(""));
        }
        else
        {
            ui->lCurrentUpSpeed->setText(QString::fromUtf8("%1 %2")
                                         .arg(QString::fromUtf8("<span style=\"color:#666666; font-size: 28px; text-decoration:none;\">%1</span>")
                                         .arg(Utilities::getSizeString(speed).split(QString::fromUtf8(" ")).at(0)))
                                         .arg(Utilities::getSizeString(speed).split(QString::fromUtf8(" ")).at(1) + QString::fromUtf8("/s")));
        }
    }
}

void ActiveTransfersWidget::on_bDownCancel_clicked()
{
    MegaTransfer *transfer = NULL;
    transfer = megaApi->getTransferByTag(activeDownload.tag);
    if (!transfer)
    {
        return;
    }

    QMessageBox warning;
    warning.setWindowTitle(QString::fromUtf8("MEGAsync"));
    warning.setText(tr("Are you sure you want to cancel this transfer?"));
    warning.setIcon(QMessageBox::Warning);
    warning.setIconPixmap(QPixmap(Utilities::getDevicePixelRatio() < 2 ? QString::fromUtf8(":/images/mbox-warning.png")
                                                                       : QString::fromUtf8(":/images/mbox-warning@2x.png")));
    warning.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    warning.setDefaultButton(QMessageBox::No);
    int result = warning.exec();
    if (result == QMessageBox::Yes)
    {
        megaApi->cancelTransfer(transfer);
    }
}

void ActiveTransfersWidget::on_bUpCancel_clicked()
{
    MegaTransfer *transfer = NULL;
    transfer = megaApi->getTransferByTag(activeUpload.tag);
    if (!transfer)
    {
        return;
    }

    QMessageBox warning;
    warning.setWindowTitle(QString::fromUtf8("MEGAsync"));
    warning.setText(tr("Are you sure you want to cancel this transfer?"));
    warning.setIcon(QMessageBox::Warning);
    warning.setIconPixmap(QPixmap(Utilities::getDevicePixelRatio() < 2 ? QString::fromUtf8(":/images/mbox-warning.png")
                                                                       : QString::fromUtf8(":/images/mbox-warning@2x.png")));
    warning.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    warning.setDefaultButton(QMessageBox::No);
    int result = warning.exec();
    if (result == QMessageBox::Yes)
    {
        megaApi->cancelTransfer(transfer);
    }
}

void ActiveTransfersWidget::setType(TransferData *td, int type, bool isSyncTransfer)
{
    td->type = type;

    qreal ratio = 1.0;
#if QT_VERSION >= 0x050000
    ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
#endif

    switch (type)
    {
        case MegaTransfer::TYPE_UPLOAD:
            delete animationUp;
            if (!isSyncTransfer)
            {
                loadIconResourceUp = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_upload_item_ico.png")
                                                           : QString::fromUtf8(":/images/cloud_upload_item_ico@2x.png"));
                animationUp = new QMovie(ratio < 2 ? QString::fromUtf8(":/images/uploading.gif")
                                                 : QString::fromUtf8(":/images/uploading@2x.gif"));
            }
            else
            {
                loadIconResourceUp = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/sync_item_ico.png")
                                                           : QString::fromUtf8(":/images/sync_item_ico@2x.png"));
                animationUp = new QMovie(ratio < 2 ? QString::fromUtf8(":/images/synching.gif")
                                                 : QString::fromUtf8(":/images/synching@2x.gif"));
            }
            break;

        case MegaTransfer::TYPE_DOWNLOAD:
            delete animationDown;
            if (!isSyncTransfer)
            {
                loadIconResourceDown = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_download_item_ico.png")
                                                           : QString::fromUtf8(":/images/cloud_download_item_ico@2x.png"));
                animationDown = new QMovie(ratio < 2 ? QString::fromUtf8(":/images/downloading.gif")
                                                 : QString::fromUtf8(":/images/downloading@2x.gif"));

            }
            else
            {
                loadIconResourceDown = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/sync_item_ico.png")
                                                           : QString::fromUtf8(":/images/sync_item_ico@2x.png"));
                animationDown = new QMovie(ratio < 2 ? QString::fromUtf8(":/images/synching.gif")
                                                 : QString::fromUtf8(":/images/synching@2x.gif"));

            }
            break;

        default:
            break;
    }
}

void ActiveTransfersWidget::setTotalSize(TransferData *td, long long size)
{
    td->totalSize = size;
    if (td->totalSize < 0)
    {
        td->totalSize = 0;
    }
    if (td->totalTransferredBytes > td->totalSize)
    {
        td->totalTransferredBytes = td->totalSize;
    }
}

void ActiveTransfersWidget::setSpeed(TransferData *td, long long transferSpeed)
{   
    if (transferSpeed < 0)
    {
        td->transferSpeed = 0;
    }
    else
    {
        td->transferSpeed = transferSpeed;
    }
}

void ActiveTransfersWidget::setTransferredBytes(TransferData *td, long long totalTransferredBytes)
{
    td->totalTransferredBytes = totalTransferredBytes;
    if (td->totalTransferredBytes < 0)
    {
        td->totalTransferredBytes = 0;
    }
    if (td->totalTransferredBytes > td->totalSize)
    {
        td->totalTransferredBytes = td->totalSize;
    }
}

void ActiveTransfersWidget::udpateTransferState(TransferData *td)
{
    QString remainingTime;

    updateAnimation(td);
    switch (td->transferState)
    {
        case MegaTransfer::STATE_ACTIVE:
        {
            // Update remaining time
            long long remainingBytes = td->totalSize - td->totalTransferredBytes;
            int totalRemainingSeconds = td->meanTransferSpeed ? remainingBytes / td->meanTransferSpeed : 0;
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

            break;
        }
        case MegaTransfer::STATE_PAUSED:
        {
            remainingTime = QString::fromUtf8("- <span style=\"color:#777777; text-decoration:none;\">m</span> - <span style=\"color:#777777; text-decoration:none;\">s</span>");
            break;
        }
        default:
            remainingTime = QString::fromUtf8("");
            break;
    }

    // Update progress bar
    unsigned int permil = (td->totalSize > 0) ? ((1000 * td->totalTransferredBytes) / td->totalSize) : 0;
    if (td->type == MegaTransfer::TYPE_DOWNLOAD)
    {
        ui->sDownloads->setCurrentWidget(ui->wActiveDownloads);
        ui->lDownRemainingTime->setText(remainingTime);
        ui->pbDownloads->setValue(permil);
        ui->lDownCompletedSize->setText(QString::fromUtf8("%1%2")
                                        .arg(!td->totalTransferredBytes ? QString::fromUtf8("") : QString::fromUtf8("<span style=\"color:#333333; text-decoration:none;\">%1</span>")
                                        .arg(Utilities::getSizeString(td->totalTransferredBytes)))
                                        .arg((!td->totalTransferredBytes ? QString::fromUtf8("") : QString::fromUtf8(" / ")) + Utilities::getSizeString(td->totalSize)));
    }
    else
    {
        ui->sUploads->setCurrentWidget(ui->wActiveUploads);
        ui->lUpRemainingTime->setText(remainingTime);
        ui->pbUploads->setValue(permil);
        ui->lUpCompletedSize->setText(QString::fromUtf8("%1%2")
                                      .arg(!td->totalTransferredBytes ? QString::fromUtf8(""): QString::fromUtf8("<span style=\"color:#333333; text-decoration:none;\">%1</span>")
                                      .arg(Utilities::getSizeString(td->totalTransferredBytes)))
                                      .arg(!td->totalTransferredBytes ? QString::fromUtf8("") : QString::fromUtf8(" / ") + Utilities::getSizeString(td->totalSize)));
    }
}

void ActiveTransfersWidget::updateNumberOfTransfers(mega::MegaApi *api)
{
    totalUploads = api->getNumPendingUploads();
    totalDownloads = api->getNumPendingDownloads();

    ui->lRemainingDownloads->setText(QString::fromUtf8("%1").arg(totalDownloads));
    ui->lRemainingUploads->setText(QString::fromUtf8("%1").arg(totalUploads));

    if (totalDownloads)
    {
        ui->sDownloads->setCurrentWidget(ui->wActiveDownloads);
    }
    else
    {
        ui->sDownloads->setCurrentWidget(ui->wNoDownloads);
    }

    if (totalUploads)
    {
        ui->sUploads->setCurrentWidget(ui->wActiveUploads);
    }
    else
    {
        ui->sUploads->setCurrentWidget(ui->wNoUploads);
    }

    if (!totalDownloads && !totalUploads)
    {
        ui->sTransfersContainer->setCurrentWidget(ui->pNoTransfers);
        ui->bGraphsSeparator->setStyleSheet(QString::fromAscii("background-color: transparent; "
                                                               "border: none; "));
    }
    else
    {
        ui->sTransfersContainer->setCurrentWidget(ui->pTransfers);
        ui->bGraphsSeparator->setStyleSheet(QString::fromAscii("background-color: rgba(0, 0, 0, 10%); "
                                                               "border: none; "));
    }
}

void ActiveTransfersWidget::updateAnimation(TransferData *td)
{
    if (!animationUp && !animationDown)
    {
        return;
    }

    switch (td->transferState)
    {
        case MegaTransfer::STATE_ACTIVE:
            if (td->type == MegaTransfer::TYPE_UPLOAD)
            {
                if (animationUp->state() != QMovie::Running)
                {
                    ui->lUpAnimation->setMovie(animationUp);
                    animationUp->start();
                }
            }
            else if (td->type == MegaTransfer::TYPE_DOWNLOAD)
            {
                if (animationDown->state() != QMovie::Running)
                {
                    ui->lDownAnimation->setMovie(animationDown);
                    animationDown->start();
                }
            }
            break;

        default:
            if(td->type == MegaTransfer::TYPE_UPLOAD)
            {
                if (animationUp->state() != QMovie::NotRunning)
                {
                    animationUp->stop();
                    ui->lUpAnimation->setMovie(NULL);
                    ui->lUpAnimation->setPixmap(loadIconResourceUp);
                }
            }
            else if (td->type == MegaTransfer::TYPE_DOWNLOAD)
            {
                if (animationDown->state() != QMovie::NotRunning)
                {
                    animationDown->stop();
                    ui->lDownAnimation->setMovie(NULL);
                    ui->lDownAnimation->setPixmap(loadIconResourceDown);
                }
            }
            break;
    }
}

TransferData::TransferData()
{
    clear();
}

void TransferData::clear()
{
    fileName = QString();
    transferState = 0;
    tag = 0;
    transferSpeed = 0;
    meanTransferSpeed = 0;
    totalSize = 0;
    totalTransferredBytes = 0;
    priority = 0xffffffffffffffff;
}
