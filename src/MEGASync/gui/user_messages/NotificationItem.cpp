#include "NotificationItem.h"

#include "ui_NotificationItem.h"
#include "UserNotification.h"
#include "Utilities.h"

#include "megaapi.h"

namespace
{
const QLatin1String DescriptionHtmlStart("<html><head/><body><p style=\"line-height:22px;\">");
const QLatin1String DescriptionHtmlEnd("</p></body></html>");
constexpr int SpacingWithoutLargeImage = 6;
constexpr int SpacingWithoutSmallImage = 0;
constexpr int SmallImageSize = 48;
constexpr int LargeImageWidth = 370;
constexpr int LargeImageHeight = 115;
constexpr int HeightWithoutImage = 219;
}

NotificationItem::NotificationItem(QWidget *parent)
    : UserMessageWidget(parent)
    , mUi(new Ui::NotificationItem)
{
    mUi->setupUi(this);
}

NotificationItem::~NotificationItem()
{
    delete mUi;
}

void NotificationItem::setData(UserMessage* data)
{
    UserNotification* notification = dynamic_cast<UserNotification*>(data);
    if(notification)
    {
        setNotificationData(notification);
    }
}

UserMessage* NotificationItem::getData() const
{
    return mNotificationData;
}

QSize NotificationItem::minimumSizeHint() const
{
    return sizeHint();
}

QSize NotificationItem::sizeHint() const
{
    QSize size = this->size();
    if(!mNotificationData->showImage())
    {
        size.setHeight(HeightWithoutImage);
    }
    return size;
}

void NotificationItem::onCTAClicked()
{
    auto actionUrl = mNotificationData->getActionUrl();
    if(actionUrl.isEmpty())
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Empty action URL in notification item.");
        return;
    }
    Utilities::openUrl(actionUrl);
}

void NotificationItem::setNotificationData(UserNotification* newNotificationData)
{
    mNotificationData = newNotificationData;

    mUi->lTitle->setText(mNotificationData->getTitle());

    QString labelText(DescriptionHtmlStart);
    labelText += mNotificationData->getDescription();
    labelText += DescriptionHtmlEnd;
    mUi->lDescription->setText(labelText);

    setImages();

    mUi->bCTA->setText(mNotificationData->getActionText());
    connect(mUi->bCTA, &QPushButton::clicked,
            this, &NotificationItem::onCTAClicked, Qt::UniqueConnection);

    mUi->lTime->setText(QString::fromLatin1("Offer expires in 5 days"));
}

void NotificationItem::setImages()
{
    bool showImage = mNotificationData->showImage();
    mUi->lImageLarge->setVisible(showImage);
    if(showImage)
    {
        mUi->lImageLarge->setPixmap(mNotificationData->getImagePixmap());
        connect(mNotificationData, &UserNotification::imageChanged, this, [this]()
                {
                    mUi->lImageLarge->setPixmap(mNotificationData->getImagePixmap());
                });
    }
    else
    {
        mUi->vlContent->setSpacing(SpacingWithoutLargeImage);
    }

    bool showIcon = mNotificationData->showIcon();
    mUi->lImageSmall->setVisible(showIcon);
    if(showIcon)
    {
        mUi->lImageSmall->setPixmap(mNotificationData->getIconPixmap());
        connect(mNotificationData, &UserNotification::imageChanged, this, [this]()
        {
            mUi->lImageSmall->setPixmap(mNotificationData->getIconPixmap());
        });
    }
    else
    {
        mUi->hlDescription->setSpacing(SpacingWithoutSmallImage);
    }
}
