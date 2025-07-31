#include "NotificationItem.h"

#include "megaapi.h"
#include "MegaApplication.h"
#include "StatsEventHandler.h"
#include "ui_NotificationItem.h"
#include "UserNotification.h"

#include <QDateTime>

namespace
{
constexpr int SPACING_WITHOUT_SMALL_IMAGE = 0;
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
    TokenParserWidgetManager::instance()->applyCurrentTheme(this);
    this->style()->unpolish(this);
    this->style()->polish(this);
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
        connect(notification,
                &UserMessage::dataReset,
                this,
                [this, notification]()
                {
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

bool NotificationItem::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        updateNotificationData();
    }

    return UserMessageWidget::event(event);
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

    UserMessageWidget::showEvent(event);
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
            timeText = tr("Offer expires in %n minute", "", timeInterval.minutes);
        }
        else
        {
            timeText = tr("Offer expires in %1 m %2 s")
                           .arg(timeInterval.minutes)
                           .arg(timeInterval.seconds);
        }
    }
    else if (timeInterval.seconds > 0)
    {
        timeText = tr("Offer expires in %n second", "", timeInterval.seconds);
    }
    mUi->lTime->setText(timeText);
}

void NotificationItem::onUserNotificationDestroyed(QObject*)
{
    mExpirationTimer.stopExpirationTime();
    disconnect(mTimerConnection);
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
    mTimerConnection = connect(&mExpirationTimer,
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
    connect(mNotificationData,
            &QObject::destroyed,
            this,
            &NotificationItem::onUserNotificationDestroyed);

    updateNotificationData(imageHasChanged, iconHasChanged);

    // Since the notifications can be reused,
    // we need to reset the expiration time color
    updateExpirationText();
}

void NotificationItem::updateNotificationData(bool downloadImage, bool downloadIcon)
{
    mUi->lTitle->setText(mNotificationData->getTitle());

    mUi->lDescription->setText(mNotificationData->getDescription());

    // If there is no action text, the button is not shown
    bool showButton = !mNotificationData->getActionText().isEmpty();
    mUi->bCTA->setVisible(showButton);
    if (showButton)
    {
        mUi->bCTA->setEnabled(true);
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
        mUi->lImageLarge->setImageUrl(mNotificationData->getImageNamePath());
    }
}

void NotificationItem::setIcon()
{
    bool showIcon = mNotificationData->showIcon();
    mUi->lImageSmall->setVisible(showIcon);
    if (showIcon)
    {
        mUi->lImageSmall->setImageUrl(mNotificationData->getIconNamePath());
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
            mUi->bCTA->setEnabled(false);
            this->setCursor(Qt::ArrowCursor);
            this->setAttribute(Qt::WA_TransparentForMouseEvents);
        }
        else if (remainingTimeSecs == -NUM_SECS_TO_WAIT_BEFORE_REMOVE)
        {
            mExpirationTimer.stopExpirationTime();
            if (mNotificationData)
            {
                mNotificationData->markAsExpired();
            }
        }
        updated = true;
    }
    return updated;
}
