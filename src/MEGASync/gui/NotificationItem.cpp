#include "NotificationItem.h"
#include "ui_NotificationItem.h"

#include "NotificationModel.h"

namespace
{
const QLatin1String DescriptionHtmlStart("<html><head/><body><p style=\"line-height:22px;\">");
const QLatin1String DescriptionHtmlEnd("</p></body></html>");
constexpr int SpacingWithoutLargeImage = 6;
constexpr int SpacingWithoutSmallImage = 0;
constexpr int SmallImageSize = 48;
constexpr int LargeImageWidth = 370;
constexpr int LargeImageHeight = 115;
}

NotificationItem::NotificationItem(MegaNotificationExt* notification, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NotificationItem)
    , mNotificationData(notification)
{
    ui->setupUi(this);
    init();
}

NotificationItem::~NotificationItem()
{
    delete ui;
}

void NotificationItem::init()
{
    ui->lTitle->setText(mNotificationData->getTitle());

    QString labelText(DescriptionHtmlStart);
    labelText += mNotificationData->getDescription();
    labelText += DescriptionHtmlEnd;
    ui->lDescription->setText(labelText);

    setImages();

    ui->bCTA->setText(QString::fromUtf8(mNotificationData->getActionText()));
    ui->lTime->setText(QString::fromLatin1("Offer expires in 5 days"));
}

QSize NotificationItem::minimumSizeHint() const
{
    return this->size();
}

QSize NotificationItem::sizeHint() const
{
    return this->size();
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
