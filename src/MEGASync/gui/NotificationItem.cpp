#include "NotificationItem.h"
#include "ui_NotificationItem.h"

#include "NotificationModel.h"

NotificationItem::NotificationItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NotificationItem)
    , mNotificationData(nullptr)
    , mDownloader(std::make_unique<ImageDownloader>(this))
{
    ui->setupUi(this);

    connect(mDownloader.get(), &ImageDownloader::downloadFinished,
            this, &NotificationItem::onDownloadFinished);
}

NotificationItem::~NotificationItem()
{
    delete ui;
}

void NotificationItem::setNotificationData(MegaNotificationExt* notification)
{
    mNotificationData = notification;

    ui->lTitle->setText(mNotificationData->getTitle());

    QString labelText = QString::fromLatin1("<html><head/><body><p style=\"line-height:22px;\">");
    labelText += mNotificationData->getDescription();
    labelText += QString::fromLatin1("</p></body></html>");
    ui->lDescription->setText(labelText);

    bool showImage = mNotificationData->showImage();
    ui->lImageLarge->setVisible(showImage);
    if(showImage)
    {
        mDownloader->downloadImage(mNotificationData->getImageNamePath(), 370, 115);
    }
    else
    {
        ui->vlContent->setSpacing(6);
    }

    bool showIcon = mNotificationData->showIcon();
    ui->lImageSmall->setVisible(showIcon);
    if(showIcon)
    {
        mDownloader->downloadImage(mNotificationData->getIconNamePath(), 48, 48);
    }
    else
    {
        ui->hlDescription->setSpacing(0);
    }

    ui->bCTA->setText(QString::fromStdString(mNotificationData->getActionText()));

    ui->lTime->setText(QString::fromLatin1("Offer expires in 5 days"));
}

void NotificationItem::onDownloadFinished(const QImage& image, const QString& imageUrl)
{
    if (image.isNull())
    {
        return;
    }

    QLabel* imageLabel = nullptr;
    QSize size(0, 0);
    if (imageUrl == mNotificationData->getImageNamePath())
    {
        imageLabel = ui->lImageLarge;
        size = QSize(370, 115);
    }
    else if (imageUrl == mNotificationData->getIconNamePath())
    {
        imageLabel = ui->lImageSmall;
        size = QSize(48, 48);
    }

    if(imageLabel)
    {
        QPixmap pixmap = QPixmap::fromImage(image).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setPixmap(pixmap);
    }
}
