#include "NotificationItem.h"
#include "ui_NotificationItem.h"

#include "NotificationModel.h"

namespace
{
constexpr int SpacingWithoutLargeImage = 6;
constexpr int SpacingWithoutSmallImage = 0;
constexpr int SmallImageSize = 48;
constexpr int LargeImageWidth = 370;
constexpr int LargeImageHeight = 115;
}

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
        mDownloader->downloadImage(mNotificationData->getImageNamePath(), LargeImageWidth, LargeImageHeight);
    }
    else
    {
        ui->vlContent->setSpacing(SpacingWithoutLargeImage);
    }

    bool showIcon = mNotificationData->showIcon();
    ui->lImageSmall->setVisible(showIcon);
    if(showIcon)
    {
        mDownloader->downloadImage(mNotificationData->getIconNamePath(), SmallImageSize, SmallImageSize);
    }
    else
    {
        ui->hlDescription->setSpacing(SpacingWithoutSmallImage);
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
        size = QSize(LargeImageWidth, LargeImageHeight);
    }
    else if (imageUrl == mNotificationData->getIconNamePath())
    {
        imageLabel = ui->lImageSmall;
        size = QSize(SmallImageSize, SmallImageSize);
    }

    if(imageLabel)
    {
        QPixmap pixmap = QPixmap::fromImage(image).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imageLabel->setPixmap(pixmap);
    }
}
