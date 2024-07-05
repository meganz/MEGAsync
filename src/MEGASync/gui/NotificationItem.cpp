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
{
    ui->setupUi(this);
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

    setImages();

    ui->bCTA->setText(QString::fromStdString(mNotificationData->getActionText()));
    ui->lTime->setText(QString::fromLatin1("Offer expires in 5 days"));
}

void NotificationItem::setImages()
{
    bool showImage = mNotificationData->showImage();
    ui->lImageLarge->setVisible(showImage);
    if(showImage)
    {
        ui->lImageLarge->setPixmap(mNotificationData->getImagePixmap());
        connect(mNotificationData, &MegaNotificationExt::imageChanged, this, [this]()
                {
                    ui->lImageLarge->setPixmap(mNotificationData->getImagePixmap());
                });
    }
    else
    {
        ui->vlContent->setSpacing(SpacingWithoutLargeImage);
    }

    bool showIcon = mNotificationData->showIcon();
    ui->lImageSmall->setVisible(showIcon);
    if(showIcon)
    {
        ui->lImageSmall->setPixmap(mNotificationData->getIconPixmap());
        connect(mNotificationData, &MegaNotificationExt::imageChanged, this, [this]()
                {
                    ui->lImageSmall->setPixmap(mNotificationData->getIconPixmap());
                });
    }
    else
    {
        ui->hlDescription->setSpacing(SpacingWithoutSmallImage);
    }
}
