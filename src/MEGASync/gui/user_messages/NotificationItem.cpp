#include "NotificationItem.h"

#include "ui_NotificationItem.h"
#include "UserNotification.h"
#include "StatsEventHandler.h"
#include "MegaApplication.h"

#include "megaapi.h"

#include <QDateTime>

namespace
{
const QLatin1String DESCRIPTION_HTML_START("<html><head/><body><p style=\"line-height:22px;\">");
const QLatin1String DESCRIPTION_HTML_END("</p></body></html>");
const QLatin1String NON_EXPIRED_TIME_COLOR("color: #777777;");
const QLatin1String EXPIRED_SOON_COLOR("color: #D64446;");
constexpr int SPACING_WITHOUT_SMALL_IMAGE = 0;
constexpr int SMALL_IMAGE_SIZE = 48;
constexpr int LARGE_IMAGE_WIDTH = 370;
constexpr int LARGE_IMAGE_HEIGHT = 115;
constexpr int HEIGHT_WITHOUT_IMAGE = 219;
constexpr int HEIGHT_WITH_IMAGE = 346;
constexpr int BUTTON_HEIGHT_WITH_SPACING = 40;
constexpr int NUM_SECS_TO_WAIT_BEFORE_REMOVE = 1;
}

NotificationItem::NotificationItem(QWidget* parent):
    UserMessageWidget(parent),
    mUi(new Ui::NotificationItem),
    mNotificationData(nullptr),
    mExpirationTimer(this),
    mDisplayEventSent(false)
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
    if (notification)
    {
        setNotificationData(notification);
        connect(notification, &UserMessage::dataChanged, this, [this, notification]() {
            updateNotificationData(notification);
        });
        mDisplayEventSent = false;
    }
}

UserMessage* NotificationItem::getData() const
{
    return mNotificationData.data();
}

QSize NotificationItem::minimumSizeHint() const
{
    return sizeHint();
}

QSize NotificationItem::sizeHint() const
{
    QSize size = this->size();
    if (mNotificationData->showImage())
    {
        size.setHeight(HEIGHT_WITH_IMAGE);
    }
    else
    {
        size.setHeight(HEIGHT_WITHOUT_IMAGE);
    }

    // If there is no action text, the button is not shown
    if (mNotificationData->getActionText().isEmpty())
    {
        size.setHeight(size.height() - BUTTON_HEIGHT_WITH_SPACING);
    }

    return size;
}

void NotificationItem::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        updateNotificationData();
    }

    QWidget::changeEvent(event);
}

void NotificationItem::showEvent(QShowEvent* event)
{
    if (!mDisplayEventSent && mNotificationData)
    {
        // Avoid to send this event every time
        MegaSyncApp->getStatsEventHandler()->sendTrackedEventArg(
            AppStatsEvents::EventType::NOTIFICATION_DISPLAYED,
            {QString::number(mNotificationData->id())});

        mDisplayEventSent = true;
    }

    QWidget::showEvent(event);
}

void NotificationItem::mousePressEvent(QMouseEvent* event)
{
    if (!mNotificationData)
    {
        return;
    }

    onCTAClicked();

    QWidget::mousePressEvent(event);
}

void NotificationItem::onCTAClicked()
{
    auto actionUrl = mNotificationData->getActionUrl();
    if (actionUrl.isEmpty())
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           "Empty action URL in notification item.");
        return;
    }
    Utilities::openUrl(actionUrl);

    MegaSyncApp->getStatsEventHandler()->sendTrackedEventArg(
        AppStatsEvents::EventType::NOTIFICATION_CTA_CLICKED,
        {QString::number(mNotificationData->id())});
}

void NotificationItem::onTimerExpirated(int64_t remainingTimeSecs)
{
    // It is required to display this text and disable clicks
    // after the expiration time.
    if(updateExpiredTimeAndClicks(remainingTimeSecs))
    {
        return;
    }

    // The timer is accurate and the requirement is to display remaining
    // time rounded down to the nearest hour or day.
    TimeInterval timeInterval(remainingTimeSecs - 1);
    QString timeText;
    if (timeInterval.days > 0)
    {
        timeText = tr("Offer expires in %n day", "", timeInterval.days);
    }
    else if (timeInterval.hours > 0)
    {
        timeText = tr("Offer expires in %n hour", "", timeInterval.hours);
    }
    else if (timeInterval.minutes > 0)
    {
        if (timeInterval.seconds == 0)
        {
            timeText = tr("Offer expires in %1 m").arg(timeInterval.minutes);
        }
        else
        {
            timeText = tr("Offer expires in %1 m %2 s")
                           .arg(timeInterval.minutes)
                           .arg(timeInterval.seconds);
        }
        mUi->lTime->setStyleSheet(EXPIRED_SOON_COLOR);
    }
    else if (timeInterval.seconds > 0)
    {
        timeText = tr("Offer expires in %1 s").arg(timeInterval.seconds);
        mUi->lTime->setStyleSheet(EXPIRED_SOON_COLOR);
    }
    mUi->lTime->setText(timeText);
}

void NotificationItem::setNotificationData(UserNotification* newNotificationData)
{
    if (!newNotificationData)
    {
        return;
    }

    connect(mUi->bCTA,
            &QPushButton::clicked,
            this,
            &NotificationItem::onCTAClicked,
            Qt::UniqueConnection);

    // NotificationExpirationTimer is a QTimer optimized for this use case.
    // We want only update the remaining time every second, minute, hour or day
    // depending in the remaining time, not always to update every second.
    connect(&mExpirationTimer,
            &NotificationExpirationTimer::expired,
            this,
            &NotificationItem::onTimerExpirated,
            Qt::UniqueConnection);

    updateNotificationData(newNotificationData);
}

void NotificationItem::updateNotificationData(UserNotification* newNotificationData)
{
    if (!newNotificationData)
    {
        return;
    }

    bool imageHasChanged = true;
    bool iconHasChanged = true;
    if (mNotificationData != nullptr)
    {
        imageHasChanged =
            mNotificationData->getImageNamePath() != newNotificationData->getImageNamePath();
        iconHasChanged =
            mNotificationData->getIconNamePath() != newNotificationData->getIconNamePath();
    }

    mNotificationData = newNotificationData;

    updateNotificationData(imageHasChanged, iconHasChanged);

    // Since the notifications can be reused,
    // we need to reset the expiration time color
    mUi->lTime->setStyleSheet(NON_EXPIRED_TIME_COLOR);
    updateExpirationText();
}

void NotificationItem::updateNotificationData(bool downloadImage, bool downloadIcon)
{
    mUi->lTitle->setText(mNotificationData->getTitle());

    QString labelText(DESCRIPTION_HTML_START);
    labelText += mNotificationData->getDescription();
    labelText += DESCRIPTION_HTML_END;
    mUi->lDescription->setText(labelText);

    // If there is no action text, the button is not shown
    bool showButton = !mNotificationData->getActionText().isEmpty();
    mUi->bCTA->setVisible(showButton);
    if (showButton)
    {
        mUi->bCTA->setEnabled(true);
        this->setCursor(Qt::PointingHandCursor);
        mUi->bCTA->setText(mNotificationData->getActionText());
    }

    this->setAttribute(Qt::WA_TransparentForMouseEvents, false);

    // Avoid to download images again if they have not changed
    if (downloadImage)
    {
        setImage();
    }

    if (downloadIcon)
    {
        setIcon();
    }

    mExpirationTimer.startExpirationTime(mNotificationData->getEnd());
    updateExpirationText();
}

void NotificationItem::setImage()
{
    bool showImage = mNotificationData->showImage();
    mUi->lImageLarge->setVisible(showImage);
    if (showImage)
    {
        mUi->lImageLarge->setPixmap(mNotificationData->getImagePixmap());
        connect(mNotificationData, &UserNotification::imageChanged, this, [this]() {
            mUi->lImageLarge->setPixmap(mNotificationData->getImagePixmap());
        });
    }
}

void NotificationItem::setIcon()
{
    bool showIcon = mNotificationData->showIcon();
    mUi->lImageSmall->setVisible(showIcon);
    if (showIcon)
    {
        mUi->lImageSmall->setPixmap(mNotificationData->getIconPixmap());
        connect(mNotificationData, &UserNotification::imageChanged, this, [this]() {
            mUi->lImageSmall->setPixmap(mNotificationData->getIconPixmap());
        });
    }
    else
    {
        mUi->hlDescription->setSpacing(SPACING_WITHOUT_SMALL_IMAGE);
    }
}

void NotificationItem::updateExpirationText()
{
    onTimerExpirated(mExpirationTimer.getRemainingTime());
}

bool NotificationItem::updateExpiredTimeAndClicks(int64_t remainingTimeSecs)
{
    bool updated = false;
    if (remainingTimeSecs <= 1)
    {
        if (remainingTimeSecs == 1)
        {
            mUi->lTime->setText(tr("Offer expired"));
            mUi->lTime->setStyleSheet(EXPIRED_SOON_COLOR);
            mUi->bCTA->setEnabled(false);
            this->setCursor(Qt::ArrowCursor);
            this->setAttribute(Qt::WA_TransparentForMouseEvents);
        }
        else if (remainingTimeSecs == -NUM_SECS_TO_WAIT_BEFORE_REMOVE)
        {
            mExpirationTimer.stopExpirationTime();
            mNotificationData->markAsExpired();
        }
        updated = true;
    }
    return updated;
}
