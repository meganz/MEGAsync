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

void NotificationItem::setNotificationData(NotifTest* notification)
{
    mNotificationData = notification;

    ui->lTitle->setText(QString::fromStdString(mNotificationData->title));

    QString labelText = QString::fromStdString("<html><head/><body><p style=\"line-height:22px;\">");
    labelText += QString::fromStdString(mNotificationData->description);
    labelText += QString::fromStdString("</p></body></html>");
    ui->lDescription->setText(labelText);

    ui->lImageLarge->setVisible(!mNotificationData->imageName.empty());
    if(!mNotificationData->imageName.empty())
    {
        QString fullImagePath = QString::fromStdString(mNotificationData->imagePath)
                                + QString::fromStdString(mNotificationData->imageName);
        QPixmap image(fullImagePath);
        ui->lImageLarge->setPixmap(image);
    }
    else
    {
        ui->vlContent->setSpacing(6);
    }

    ui->lImageSmall->setVisible(!mNotificationData->iconName.empty());
    if(!mNotificationData->iconName.empty())
    {
        QString fullIconPath = QString::fromStdString(mNotificationData->imagePath)
                               + QString::fromStdString(mNotificationData->iconName);
        QPixmap image(fullIconPath);
        ui->lImageSmall->setPixmap(image);
    }
    else
    {
        ui->hlDescription->setSpacing(0);
    }

    auto it = mNotificationData->callToAction1.find("text");
    if (it!= mNotificationData->callToAction1.end())
    {
        ui->bCTA->setText(QString::fromStdString(it->second));
    }

    ui->lTime->setText(QString::fromLatin1("Offer expires in 5 days"));
}
