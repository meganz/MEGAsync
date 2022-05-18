#include "ActiveTransfersWidget.h"
#include "ui_ActiveTransfersWidget.h"
#include "control/Utilities.h"
#include "HighDpiResize.h"
#include "Preferences.h"
#include "MegaApplication.h"
#include <QMessageBox>

using namespace mega;

ActiveTransfersWidget::ActiveTransfersWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActiveTransfersWidget),
    previousTotalUploads{0}, previousTotalDownloads{0}
{
    ui->setupUi(this);

    // Choose the right icon for initial load (hdpi/normal displays)
    qreal ratio = 1.0;
#if QT_VERSION >= 0x050000
    ratio = qApp->testAttribute(Qt::AA_UseHighDpiPixmaps) ? devicePixelRatio() : 1.0;
#endif
    ui->lDownAnimation->setPixmap(QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_item_ico.png")
                                                   : QString::fromUtf8(":/images/cloud_item_ico@2x.png")));
    ui->lUpAnimation->setPixmap(QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_item_ico.png")
                                                   : QString::fromUtf8(":/images/cloud_item_ico@2x.png")));

    activeDownload.clear();
    animationDown = animationUp = NULL;

    mThreadPool = ThreadPoolSingleton::getInstance();

    ui->bDownPaused->setVisible(false);
    ui->bUpPaused->setVisible(false);
    ui->sDownloads->setCurrentWidget(ui->wNoDownloads);
    ui->sUploads->setCurrentWidget(ui->wNoUploads);
    ui->sTransfersContainer->setCurrentWidget(ui->pNoTransfers);
    ui->bGraphsSeparator->setStyleSheet(QString::fromAscii("background-color: transparent; "
                                                           "border: none; "));
    mWhichGraphsStyleSheet = 1;
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

void ActiveTransfersWidget::onTransferFinish(mega::MegaApi* api, mega::MegaTransfer* transfer, mega::MegaError*)
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

void ActiveTransfersWidget::onTransferTemporaryError(mega::MegaApi*, mega::MegaTransfer*, mega::MegaError*)
{

}

void ActiveTransfersWidget::updateTransferInfo(MegaTransfer *transfer)
{
    if (transfer->isStreamingTransfer() || transfer->isFolderTransfer())
    {
        return;
    }

    int type = transfer->getType();
    unsigned long long priority = transfer->getPriority();
    if (!priority)
    {
        priority = 0xFFFFFFFFFFFFFFFFULL;
    }
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        if (priority > activeDownload.priority
                && !(activeDownload.transferState == MegaTransfer::STATE_PAUSED))
        {
            if (activeDownload.tag == transfer->getTag())
            {
                activeDownload.clear();
            }
            return;
        }

        // New Download transfer, update name, size and tag
        if (activeDownload.tag != transfer->getTag())
        {
            activeDownload.tag = transfer->getTag();
            setType(&activeDownload, type, transfer->isSyncTransfer());
            activeDownload.fileName = QString::fromUtf8(transfer->getFileName());
            ui->lDownFilename->ensurePolished();
            ui->lDownFilename->setText(ui->lDownFilename->fontMetrics().elidedText(activeDownload.fileName, Qt::ElideMiddle, ui->lDownFilename->width()));
            ui->lDownFilename->setToolTip(activeDownload.fileName);
            ui->bDownFileType->setIcon(Utilities::getExtensionPixmapSmall(activeDownload.fileName));
            setTotalSize(&activeDownload, transfer->getTotalBytes());
        }

        // Get http speed
        long long httpSpeed {static_cast<MegaApplication*>(qApp)->getMegaApi()->getCurrentDownloadSpeed()};

        activeDownload.transferState = transfer->getState();
        activeDownload.priority = priority;
        activeDownload.meanTransferSpeed = transfer->getMeanSpeed();
        setSpeed(&activeDownload, std::min(transfer->getSpeed(), httpSpeed));
        setTransferredBytes(&activeDownload, transfer->getTransferredBytes());
        // Update remaining time
        activeDownload.updateRemainingTimeSeconds();
        updateTransferState(&activeDownload);
    }
    else
    {
        if (priority > activeUpload.priority
                && !(activeUpload.transferState == MegaTransfer::STATE_PAUSED))
        {
            if (activeUpload.tag == transfer->getTag())
            {
                activeUpload.clear();
            }
            return;
        }

        // New Upload transfer, update name, size and tag
        if (activeUpload.tag != transfer->getTag())
        {
            activeUpload.tag = transfer->getTag();
            setType(&activeUpload, type, transfer->isSyncTransfer());
            activeUpload.fileName = QString::fromUtf8(transfer->getFileName());
            ui->lUpFilename->ensurePolished();
            ui->lUpFilename->setText(ui->lUpFilename->fontMetrics().elidedText(activeUpload.fileName, Qt::ElideMiddle, ui->lUpFilename->width()));
            ui->lUpFilename->setToolTip(activeUpload.fileName);
            ui->bUpFileType->setIcon(Utilities::getExtensionPixmapSmall(activeUpload.fileName));
            setTotalSize(&activeUpload, transfer->getTotalBytes());
        }

        // Get http speed
        long long httpSpeed {static_cast<MegaApplication*>(qApp)->getMegaApi()->getCurrentUploadSpeed()};

        activeUpload.transferState = transfer->getState();
        activeUpload.priority = priority;
        activeUpload.meanTransferSpeed = transfer->getMeanSpeed();
        setSpeed(&activeUpload, std::min(transfer->getSpeed(), httpSpeed));
        setTransferredBytes(&activeUpload, transfer->getTransferredBytes());
        // Update remaining time
        activeUpload.updateRemainingTimeSeconds();
        updateTransferState(&activeUpload);
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
    QPointer<ActiveTransfersWidget> activeTransfersWidget = this;

    if (totalDownloads && activeDownload.priority == 0xFFFFFFFFFFFFFFFFULL)
    {
        mThreadPool->push([this, activeTransfersWidget]()
        {//thread pool function
            if (!activeTransfersWidget)
            {
                return;
            }

            MegaTransfer *nextTransfer = ((MegaApplication *)qApp)->getMegaApi()->getFirstTransfer(MegaTransfer::TYPE_DOWNLOAD);

            if (nextTransfer)
            {
                Utilities::queueFunctionInAppThread([this, activeTransfersWidget, nextTransfer]()
                {//queued function

                    if (activeTransfersWidget)
                    {
                        onTransferUpdate(megaApi, nextTransfer);
                    }
                    delete nextTransfer;

                });//end of queued function
            }

        });// end of thread pool function;
    }

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

            megaApi->log(MegaApi::LOG_LEVEL_INFO, (QString::fromUtf8("DOWNLOAD SPEED %1 %2")
                .arg(Utilities::getSizeString(speed).split(QString::fromUtf8(" ")).at(0))
                .arg(Utilities::getSizeString(speed).split(QString::fromUtf8(" ")).at(1) + QString::fromUtf8("/s"))).toUtf8().constData());

        }
    }
}

void ActiveTransfersWidget::updateUpSpeed(long long speed)
{
    QPointer<ActiveTransfersWidget> activeTransfersWidget = this;

    if (totalUploads && activeUpload.priority == 0xFFFFFFFFFFFFFFFFULL)
    {
        mThreadPool->push([this, activeTransfersWidget]()
        {//thread pool function
            if (!activeTransfersWidget)
            {
                return;
            }

            MegaTransfer *nextTransfer = ((MegaApplication *)qApp)->getMegaApi()->getFirstTransfer(MegaTransfer::TYPE_UPLOAD);
            if (nextTransfer)
            {
                Utilities::queueFunctionInAppThread([this, activeTransfersWidget, nextTransfer]()
                {//queued function

                    if (activeTransfersWidget)
                    {
                        onTransferUpdate(megaApi, nextTransfer);
                    }

                    delete nextTransfer;

                });//end of queued function
            }

        });// end of thread pool function;
    }

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

            megaApi->log(MegaApi::LOG_LEVEL_INFO, (QString::fromUtf8("UPLOAD SPEED %1 %2")
                .arg(Utilities::getSizeString(speed).split(QString::fromUtf8(" ")).at(0))
                .arg(Utilities::getSizeString(speed).split(QString::fromUtf8(" ")).at(1) + QString::fromUtf8("/s"))).toUtf8().constData());
        }
    }
}

void ActiveTransfersWidget::on_bDownCancel_clicked()
{
    MegaTransfer *transfer = nullptr;
    transfer = megaApi->getTransferByTag(activeDownload.tag);
    if (!transfer)
    {
        return;
    }

    QMessageBox warning;
    HighDpiResize hDpiResizer(&warning);
    warning.setWindowTitle(QString::fromUtf8("MEGAsync"));
    warning.setText(tr("Are you sure you want to cancel this transfer?"));
    warning.setIcon(QMessageBox::Warning);
    warning.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    warning.setDefaultButton(QMessageBox::No);
    int result = warning.exec();
    if (result == QMessageBox::Yes)
    {
        megaApi->cancelTransfer(transfer);
    }
    delete transfer;
}

void ActiveTransfersWidget::on_bUpCancel_clicked()
{
    MegaTransfer *transfer = nullptr;
    transfer = megaApi->getTransferByTag(activeUpload.tag);
    if (!transfer)
    {
        return;
    }

    QMessageBox warning;
    HighDpiResize hDpiResizer(&warning);
    warning.setWindowTitle(QString::fromUtf8("MEGAsync"));
    warning.setText(tr("Are you sure you want to cancel this transfer?"));
    warning.setIcon(QMessageBox::Warning);
    warning.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    warning.setDefaultButton(QMessageBox::No);
    int result = warning.exec();
    if (result == QMessageBox::Yes)
    {
        megaApi->cancelTransfer(transfer);
    }
    delete transfer;
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
                animationUp = new QMovie(ratio < 2 ? QString::fromUtf8(":/animations/uploading.gif")
                                                 : QString::fromUtf8(":/animations/uploading@2x.gif"));
                ui->bUpCancel->show();
            }
            else
            {
                loadIconResourceUp = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/sync_item_ico.png")
                                                           : QString::fromUtf8(":/images/sync_item_ico@2x.png"));
                animationUp = new QMovie(ratio < 2 ? QString::fromUtf8(":/animations/synching.gif")
                                                 : QString::fromUtf8(":/animations/synching@2x.gif"));
                ui->bUpCancel->hide();
            }
            break;

        case MegaTransfer::TYPE_DOWNLOAD:
            delete animationDown;
            if (!isSyncTransfer)
            {
                loadIconResourceDown = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/cloud_download_item_ico.png")
                                                           : QString::fromUtf8(":/images/cloud_download_item_ico@2x.png"));
                animationDown = new QMovie(ratio < 2 ? QString::fromUtf8(":/animations/downloading.gif")
                                                 : QString::fromUtf8(":/animations/downloading@2x.gif"));
                ui->bDownCancel->show();

            }
            else
            {
                loadIconResourceDown = QPixmap(ratio < 2 ? QString::fromUtf8(":/images/sync_item_ico.png")
                                                           : QString::fromUtf8(":/images/sync_item_ico@2x.png"));
                animationDown = new QMovie(ratio < 2 ? QString::fromUtf8(":/animations/synching.gif")
                                                 : QString::fromUtf8(":/animations/synching@2x.gif"));
                ui->bDownCancel->hide();

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
    td->transferSpeed = std::max(0LL, transferSpeed);
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

void ActiveTransfersWidget::updateTransferState(TransferData *td)
{
    updateAnimation(td);
    QString remainingTimeString;

    // "- m - s"
    static const QString undeterminedRemainingTimeString{QString::fromUtf8(
                    "- <span style=\"color:#777777; text-decoration:none;\">m</span>"
                    " - <span style=\"color:#777777; text-decoration:none;\">s</span>"
                    )};
    switch (td->transferState)
    {
    case MegaTransfer::STATE_ACTIVE:
    {
        // The remaining time is considered infinite if remaining time value is the max possible.
        const bool infiniteRemainingTime{td->remainingTimeSeconds.count()
                    && td->remainingTimeSeconds == td->remainingTimeSeconds.max()};
        // Test if the transfer will complete in less than 1 minute
        const bool lowerThanMinute{td->remainingTimeSeconds.count()
                    && td->remainingTimeSeconds < std::chrono::minutes{1}};

        // Adapt time display according to remainig time
        if (infiniteRemainingTime)
        {
            // If inifinite display undetermined string
            remainingTimeString = undeterminedRemainingTimeString;
        }
        else if(lowerThanMinute)
        {
            // If less than a minute, "1m"
            static const auto lowerThanMinuteTimeString = QString::fromUtf8("%1 <span style=\"color:#777777; text-decoration:none;\">m</span>").arg(QString::fromUtf8("&lt; 1"));
            remainingTimeString = lowerThanMinuteTimeString;
        }
        else if (td->remainingTimeSeconds.count())
        {
            remainingTimeString = Utilities::getTimeString(td->remainingTimeSeconds.count());
        }
        else
        {
            remainingTimeString = QString::fromUtf8("");
        }
        break;
    }
    case MegaTransfer::STATE_PAUSED:
    {
        remainingTimeString = undeterminedRemainingTimeString;
        break;
    }
    default:
        remainingTimeString = QString::fromUtf8("");
        break;
    }

    // Update progress bar
    unsigned int permil = (td->totalSize > 0) ? static_cast<unsigned int>((1000 * td->totalTransferredBytes) / td->totalSize) : 0;
    if (td->type == MegaTransfer::TYPE_DOWNLOAD)
    {
        ui->sDownloads->setCurrentWidget(ui->wActiveDownloads);
        ui->lDownRemainingTime->setText(remainingTimeString);
        ui->pbDownloads->setValue(permil);
        ui->lDownCompletedSize->setText(QString::fromUtf8("%1%2")
                                        .arg(!td->totalTransferredBytes ? QString::fromUtf8("") : QString::fromUtf8("<span style=\"color:#333333; text-decoration:none;\">%1</span>")
                                        .arg(Utilities::getSizeString(td->totalTransferredBytes)))
                                        .arg((!td->totalTransferredBytes ? QString::fromUtf8("") : QString::fromUtf8(" / ")) + Utilities::getSizeString(td->totalSize)));
    }
    else
    {
        ui->sUploads->setCurrentWidget(ui->wActiveUploads);
        ui->lUpRemainingTime->setText(remainingTimeString);
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
    ui->lRemainingUploads->setText(QString::fromUtf8("%1").arg(totalUploads));

    totalDownloads = api->getNumPendingDownloads();
    ui->lRemainingDownloads->setText(QString::fromUtf8("%1").arg(totalDownloads));

    ui->lDescRemainingUp->setText(tr("Remaining upload", "", totalUploads));
    ui->lDescRemainingDown->setText(tr("Remaining download", "", totalDownloads));

    previousTotalDownloads = totalDownloads;
    previousTotalUploads = totalUploads;

    QWidget* downloadWidget = (totalDownloads) ? ui->wActiveDownloads : ui->wNoDownloads;
    ui->sDownloads->setCurrentWidget(downloadWidget);

    QWidget* uploadWidget = (totalUploads) ? ui->wActiveUploads : ui->wNoUploads;
    ui->sUploads->setCurrentWidget(uploadWidget);

    const bool areThereTransfers = (!totalDownloads && !totalUploads);
    if (areThereTransfers)
    {
        ui->sTransfersContainer->setCurrentWidget(ui->pNoTransfers);
        updateTransfersContainerStyleSheet(1, "transparent");
    }
    else
    {
        ui->sTransfersContainer->setCurrentWidget(ui->pTransfers);
        updateTransfersContainerStyleSheet(2, "rgba(0, 0, 0, 10%)");
    }
}

void ActiveTransfersWidget::updateAnimation(TransferData *td)
{
    if ((!animationUp && td->type == MegaTransfer::TYPE_UPLOAD)
            || (!animationDown && td->type == MegaTransfer::TYPE_DOWNLOAD))
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
            if (td->type == MegaTransfer::TYPE_UPLOAD)
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

void ActiveTransfersWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

void ActiveTransfersWidget::updateTransfersContainerStyleSheet(int whichStyleSheet, const char* backgroundColor)
{
    if (mWhichGraphsStyleSheet != whichStyleSheet)
    {
        const QString styleSheet = QString::fromAscii("background-color: ") +
                QString::fromAscii(backgroundColor) + QString::fromAscii("; border: none; ");
        ui->bGraphsSeparator->setStyleSheet(styleSheet);
        mWhichGraphsStyleSheet = whichStyleSheet;
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
    totalSize = 0;
    totalTransferredBytes = 0;
    priority = 0xFFFFFFFFFFFFFFFFULL;
    remainingTimeSeconds = std::chrono::seconds{0};
    mTransferRemainingTime.reset();
}

void TransferData::updateRemainingTimeSeconds()
{
    const auto remainingBytes = totalSize - totalTransferredBytes;
    remainingTimeSeconds = mTransferRemainingTime.calculateRemainingTimeSeconds(transferSpeed, remainingBytes);
}
