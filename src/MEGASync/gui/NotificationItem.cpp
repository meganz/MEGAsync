#include "NotificationItem.h"
#include "ui_NotificationItem.h"

#include "NotificationModel.h"

NotificationItem::NotificationItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NotificationItem)
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

    bool showImage = mNotificationData->showImage();
    ui->lImageLarge->setVisible(showImage);
    if(showImage)
    {
        QPixmap image(mNotificationData->getImageNamePath());
        ui->lImageLarge->setPixmap(image);
    }
    else
    {
        ui->vlContent->setSpacing(6);
    }

    bool showIcon = mNotificationData->showIcon();
    ui->lImageSmall->setVisible(showIcon);
    if(showIcon)
    {
        QPixmap image(mNotificationData->getIconNamePath());
        ui->lImageSmall->setPixmap(image);
    }
    else
    {
        ui->hlDescription->setSpacing(0);
    }

    ui->bCTA->setText(QString::fromStdString(mNotificationData->getActionText()));

    ui->lTime->setText(QString::fromLatin1("Offer expires in 5 days"));
}
