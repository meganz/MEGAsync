#include "AlertItem.h"

#include "CommonMessages.h"
#include "FullName.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "ThemeManager.h"
#include "ui_AlertItem.h"
#include "UserAlert.h"

#include <QDateTime>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

using namespace mega;

AlertItem::AlertItem(QWidget *parent)
    : UserMessageWidget(parent)
    , mUi(new Ui::AlertItem)
    , mMegaApi(MegaSyncApp->getMegaApi())
{
    mUi->setupUi(this);

    mUi->sIconWidget->hide();
    mUi->wNotificationIcon->hide();
    mUi->lNew->hide();

    mUi->bNotificationIcon->installEventFilter(this);
    connect(mUi->wAvatarContact, &AvatarWidget::avatarUpdated, this, &AlertItem::dataChanged);
}

AlertItem::~AlertItem()
{
    delete mUi;
}

void AlertItem::setData(UserMessage* data)
{
    UserAlert* alert = dynamic_cast<UserAlert*>(data);
    if (alert && mAlertData != data)
    {
        if (mAlertData)
        {
            disconnect(mAlertData, &UserMessage::dataReset, this, &AlertItem::updateAlertData);
        }

        mAlertData = alert;

        connect(mAlertData, &UserMessage::dataReset, this, &AlertItem::updateAlertData);

        connect(mAlertData,
                &UserAlert::emailChanged,
                this,
                &AlertItem::contactEmailChanged,
                Qt::QueuedConnection);
        connect(mAlertData, &UserAlert::emailChanged, this, &AlertItem::updateAlertData);

        if (mAlertData->getUserHandle() != INVALID_HANDLE)
        {
            if (!mAlertData->getEmail().isEmpty())
            {
                requestFullName();
                setAvatarEmail();
            }
        }
        else // If it comes without user handler, it is because is an own alert, then take your
             // email.
        {
            if (mMegaApi)
            {
                mUi->wAvatarContact->setUserEmail(mMegaApi->getMyEmail());
            }
        }
    }

    updateAlertData();
}

UserMessage* AlertItem::getData() const
{
    return mAlertData.data();
}

void AlertItem::contactEmailChanged()
{
    requestFullName();
    setAvatarEmail();
    updateAlertData();
}

void AlertItem::requestFullName()
{
    if (!mAlertData || (mAlertData && mAlertData->getEmail().isEmpty()))
    {
        return;
    }

    mFullNameAttributes = UserAttributes::FullName::requestFullName(mAlertData->getEmail().toUtf8().constData());

    if(mFullNameAttributes)
    {
        connect(mFullNameAttributes.get(),
                &UserAttributes::FullName::fullNameReady,
                this,
                &AlertItem::onAttributesReady);
    }
}

void AlertItem::setAvatarEmail()
{
    mUi->wAvatarContact->setUserEmail(mAlertData->getEmail().toUtf8().constData());
}

void AlertItem::onAttributesReady()
{
    updateAlertData();
    mAlertData->updated();
}

void AlertItem::updateAlertData()
{
    if(!mAlertData)
    {
        return;
    }

    updateAlertType();
    setAlertHeading(mAlertData);
    setAlertContent(mAlertData);
    setAlertTimeStamp(mAlertData->getTimestamp(0));
    mAlertData->isSeen() ? mUi->lNew->hide() : mUi->lNew->show();
}

void AlertItem::updateAlertType()
{
    mUi->wNotificationIcon->hide();

    if(!mAlertData)
    {
        return;
    }

    QString notificationTitle;
    QString notificationColor;
    switch (mAlertData->getType())
    {
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
        case MegaUserAlert::TYPE_CONTACTCHANGE_DELETEDYOU:
        case MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
        case MegaUserAlert::TYPE_CONTACTCHANGE_ACCOUNTDELETED:
        case MegaUserAlert::TYPE_CONTACTCHANGE_BLOCKEDYOU:
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED:
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED:
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_DENIED:
        {
            notificationTitle = tr("Contacts").toUpper();
            notificationColor = QString::fromUtf8("#1CB5A0");
            break;
        }
        case MegaUserAlert::TYPE_NEWSHARE:
        case MegaUserAlert::TYPE_DELETEDSHARE:
        case MegaUserAlert::TYPE_NEWSHAREDNODES:
        case MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
        case MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
        {
            if (mAlertData->getType() == MegaUserAlert::TYPE_DELETEDSHARE)
            {
                mUi->bSharedFolder->setIcon(QIcon(QString::fromUtf8(":/images/icons/folder/small-folder-disabled.png")).pixmap(24.0, 24.0));
            }
            else
            {
                mUi->bSharedFolder->setIcon(QIcon(QString::fromUtf8(":/images/icons/folder/small-folder.png")).pixmap(24.0, 24.0));
            }
            mUi->bNotificationIcon->setMinimumSize(QSize(10, 8));
            mUi->bNotificationIcon->setMaximumSize(QSize(10, 8));
            mUi->bNotificationIcon->setIconSize(QSize(10, 8));
            mUi->bNotificationIcon->setIcon(QIcon(QString::fromLatin1("://images/share_arrow.png")));
            mUi->wNotificationIcon->show();
            notificationTitle = tr("Incoming Shares").toUpper();
            notificationColor = QString::fromUtf8("#F2C249");
            break;
        }
        case MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
        case MegaUserAlert::TYPE_PAYMENT_FAILED:
        case MegaUserAlert::TYPE_PAYMENTREMINDER:
        {
            notificationTitle = tr("Payment").toUpper();
            notificationColor = QString::fromUtf8("#FFA502");
            break;
        }
        case MegaUserAlert::TYPE_TAKEDOWN:
        case MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
        {
            notificationTitle = tr("Takedown notice").toUpper();
            notificationColor = QString::fromUtf8("#D64446");
            break;
        }
        default:
        {
            notificationTitle = QString::fromUtf8("");
            notificationColor = QString::fromUtf8("#FFFFFF");
            mUi->bNotificationIcon->setMinimumSize(QSize(16, 16));
            mUi->bNotificationIcon->setMaximumSize(QSize(16, 16));
            mUi->bNotificationIcon->setIconSize(QSize(16, 16));
            mUi->bNotificationIcon->setIcon(QIcon(QString::fromLatin1("://images/mega_notifications.png")));
            mUi->wNotificationIcon->show();
            break;
        }
    }

    mUi->lTitle->setStyleSheet(QString::fromLatin1("#lTitle { font-family: Lato; font-weight: 900; font-size: 10px; color: %1; } ")
                                   .arg(notificationColor));
    mUi->lTitle->setText(notificationTitle);
}

void AlertItem::setAlertHeading(UserAlert* alert)
{
    mUi->sIconWidget->hide();
    mNotificationHeading.clear();

    switch (alert->getType())
    {
        // Contact notifications
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
        {
            mNotificationHeading = tr("New Contact Request");
            mUi->sIconWidget->setCurrentWidget(mUi->pContact);
            mUi->sIconWidget->show();
            break;
        }
        case MegaUserAlert::TYPE_CONTACTCHANGE_DELETEDYOU:
        case MegaUserAlert::TYPE_CONTACTCHANGE_ACCOUNTDELETED:
        {
            mNotificationHeading = tr("Contact Deleted");
            mUi->sIconWidget->setCurrentWidget(mUi->pContact);
            mUi->sIconWidget->show();
            break;
        }
        case MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
        {
            mNotificationHeading = tr("Contact Established");
            mUi->sIconWidget->setCurrentWidget(mUi->pContact);
            mUi->sIconWidget->show();
            break;
        }
        case MegaUserAlert::TYPE_CONTACTCHANGE_BLOCKEDYOU:
        {
            mNotificationHeading = tr("Contact Blocked");
            mUi->sIconWidget->setCurrentWidget(mUi->pContact);
            mUi->sIconWidget->show();
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED:
        {
            mNotificationHeading = tr("Contact Updated");
            mUi->sIconWidget->setCurrentWidget(mUi->pContact);
            mUi->sIconWidget->show();
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED:
        {
            mNotificationHeading = tr("Contact Accepted");
            mUi->sIconWidget->setCurrentWidget(mUi->pContact);
            mUi->sIconWidget->show();
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_DENIED:
        {
            mNotificationHeading = tr("Contact Denied");
            mUi->sIconWidget->setCurrentWidget(mUi->pContact);
            mUi->sIconWidget->show();
            break;
        }
        // Share notifications
        case MegaUserAlert::TYPE_NEWSHARE:
        case MegaUserAlert::TYPE_DELETEDSHARE:
        case MegaUserAlert::TYPE_NEWSHAREDNODES:
        case MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
        {
            mUi->sIconWidget->setCurrentWidget(mUi->pSharedFolder);
            mUi->sIconWidget->show();
            mNotificationHeading = MegaNodeNames::getNodeName(mAlertData->getAlertNode().get());

            if (mNotificationHeading.isEmpty())
            {
                mNotificationHeading = tr("Shared Folder Activity");
            }
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
        {
            mUi->sIconWidget->setCurrentWidget(mUi->pSharedFolder);
            mUi->sIconWidget->show();
            mNotificationHeading = MegaNodeNames::getNodeName(mAlertData->getAlertNode().get());

            if (mNotificationHeading.isEmpty())
            {
                mNotificationHeading = tr("Shared folder updated");
            }
            break;
        }
        // Payment notifications
        case MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
        case MegaUserAlert::TYPE_PAYMENT_FAILED:
        case MegaUserAlert::TYPE_PAYMENTREMINDER:
            mNotificationHeading = tr("Payment Info");
            break;
        // Takedown notifications
        case MegaUserAlert::TYPE_TAKEDOWN:
        case MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
            mNotificationHeading = tr("Takedown Notice");
            break;

        default:
            mNotificationHeading = tr("Notification");
            break;
    }


    mUi->lHeading->ensurePolished();
    mUi->lHeading->setText(mUi->lHeading->fontMetrics()
                                .elidedText(mNotificationHeading,
                                           Qt::ElideMiddle,mUi->lHeading->minimumWidth()));

    if(!mAlertData->getEmail().isEmpty())
    {
        mNotificationHeading.append(QString::fromLatin1(" (")
                                        + mAlertData->getEmail()
                                        + QString::fromLatin1(")"));
        setToolTip(mNotificationHeading);
    }
}

void AlertItem::setAlertContent(UserAlert *alert)
{
    QString notificationContent;
    switch (alert->getType())
    {
        // Contact notifications
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
        {
            notificationContent = tr("[A] sent you a contact request")
                    .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            break;
        }
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
        {
            notificationContent = tr("[A] cancelled their contact request")
                    .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            break;
        }
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
        {
            notificationContent = tr("Reminder: You have a contact request");
            break;
        }
        case MegaUserAlert::TYPE_CONTACTCHANGE_DELETEDYOU:
        {
            notificationContent = tr("[A] deleted you as a contact")
                    .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            break;
        }
        case MegaUserAlert::TYPE_CONTACTCHANGE_ACCOUNTDELETED:
        {
            notificationContent = tr("[A] has been deleted/deactivated")
                    .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            break;
        }
        case MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
        {
            notificationContent = tr("[A] accepted your contact request")
                    .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            break;
        }
        case MegaUserAlert::TYPE_CONTACTCHANGE_BLOCKEDYOU:
        {
            notificationContent = tr("[A] blocked you as contact")
                    .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
        {
            notificationContent = tr("You ignored a contact request");
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
        {
            notificationContent = tr("You accepted a contact request");
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED:
        {
            notificationContent = tr("You denied a contact request");
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED:
        {
            notificationContent = tr("[A] accepted your contact request")
                    .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_DENIED:
        {
            notificationContent = tr("[A] denied your contact request")
                    .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            break;
        }
        // Share notifications
        case MegaUserAlert::TYPE_NEWSHARE:
        {
            notificationContent = tr("New shared folder from [A]")
                    .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            break;
        }
        case MegaUserAlert::TYPE_DELETEDSHARE:
        {
            if (alert->getNumber(0) == 0) //Someone left the folder
            {
                notificationContent = tr("[A] has left the shared folder")
                        .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            }
            else //Access for the user was removed by share owner
            {
                if(mAlertData)
                {
                    notificationContent = !mAlertData->getEmail().isEmpty()
                                            ? tr("Access to shared folder was removed by [A]")
                                                .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()))
                                            : tr("Access to shared folder was removed");
                }
            }
            break;
        }
        case MegaUserAlert::TYPE_NEWSHAREDNODES:
        {
            int64_t updatedItems = alert->getNumber(1) + alert->getNumber(0);
            notificationContent = tr("[A] added %n item", "", static_cast<int>(updatedItems))
                    .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            break;
        }
        case MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
        {
            int64_t updatedItems = alert->getNumber(0);
            notificationContent = tr("[A] removed %n item", "", static_cast<int>(updatedItems))
                    .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
        {
            int64_t updatedItems = alert->getNumber(0);
            notificationContent = tr("[A] updated %n item", "", static_cast<int>(updatedItems))
                    .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
            break;
        }
        // Payment notifications
        case MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
        {
            notificationContent = tr("Your payment for the [A] plan was received")
                    .replace(QString::fromUtf8("[A]"), alert->getString(0) ? QString::fromUtf8(alert->getString(0)) : QString::fromUtf8(""));
            break;
        }
        case MegaUserAlert::TYPE_PAYMENT_FAILED:
        {
            notificationContent = tr("Your payment for the [A] plan was unsuccessful")
                    .replace(QString::fromUtf8("[A]"), alert->getString(0) ? QString::fromUtf8(alert->getString(0)) : QString::fromUtf8(""));
            break;
        }
        case MegaUserAlert::TYPE_PAYMENTREMINDER:
        {
            notificationContent = CommonMessages::createPaymentReminder(alert->getTimestamp(1));
            break;
        }
        // Takedown notifications
        case MegaUserAlert::TYPE_TAKEDOWN:
        {
            auto alertNode(mAlertData->getAlertNode());
            if (alertNode)
            {
                if (alertNode->getType() == MegaNode::TYPE_FILE)
                {
                    notificationContent =
                        tr("Your publicly shared file ([A]) has been taken down")
                            .replace(QString::fromUtf8("[A]"),
                                     formatRichString(MegaNodeNames::getNodeName(alertNode.get())));
                }
                else if (alertNode->getType() == MegaNode::TYPE_FOLDER)
                {
                    notificationContent =
                        tr("Your publicly shared folder ([A]) has been taken down")
                            .replace(QString::fromUtf8("[A]"),
                                     formatRichString(MegaNodeNames::getNodeName(alertNode.get())));
                }
                else
                {
                    notificationContent = tr("Your publicly shared has been taken down");
                }
            }
            else
            {
                notificationContent = tr("Your publicly shared has been taken down");
            }
            break;
        }
        case MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
        {
            auto alertNode(mAlertData->getAlertNode());
            if (alertNode)
            {
                if (alertNode->getType() == MegaNode::TYPE_FILE)
                {
                    notificationContent =
                        tr("Your publicly shared file ([A]) has been reinstated")
                            .replace(QString::fromUtf8("[A]"),
                                     formatRichString(MegaNodeNames::getNodeName(alertNode.get())));
                }
                else if (alertNode->getType() == MegaNode::TYPE_FOLDER)
                {
                    notificationContent =
                        tr("Your publicly shared folder ([A]) has been reinstated")
                            .replace(QString::fromUtf8("[A]"),
                                     formatRichString(MegaNodeNames::getNodeName(alertNode.get())));
                }
                else
                {
                    notificationContent = tr("Your taken down has been reinstated");
                }
            }
            else
            {
                notificationContent = tr("Your taken down has been reinstated");
            }
            break;
        }
        default:
        {
            notificationContent = QString::fromUtf8(alert->getTitle());
            break;
        }
    }

    mUi->lDesc->setText(notificationContent);
}

void AlertItem::setAlertTimeStamp(int64_t ts)
{
    if (ts != -1)
    {
        const QDateTime dateTime{QDateTime::fromMSecsSinceEpoch(ts * 1000)};
        mUi->lTimeStamp->setText(MegaSyncApp->getFormattedDateByCurrentLanguage(dateTime));
    }
    else
    {
        mUi->lTimeStamp->setText(QString::fromUtf8(""));
    }
}

QString AlertItem::getHeadingString()
{
    return mNotificationHeading;
}

bool AlertItem::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == mUi->bNotificationIcon && event->type() == QEvent::Resize)
    {
        // Set with the same height than in updateAlertType.
        switch (mAlertData->getType())
        {
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
            // Fallthrough
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
            // Fallthrough
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
            // Fallthrough
            case MegaUserAlert::TYPE_CONTACTCHANGE_DELETEDYOU:
            // Fallthrough
            case MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
            // Fallthrough
            case MegaUserAlert::TYPE_CONTACTCHANGE_ACCOUNTDELETED:
            // Fallthrough
            case MegaUserAlert::TYPE_CONTACTCHANGE_BLOCKEDYOU:
            // Fallthrough
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
            // Fallthrough
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
            // Fallthrough
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED:
            // Fallthrough
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED:
            // Fallthrough
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_DENIED:
            // Fallthrough
            case MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
            // Fallthrough
            case MegaUserAlert::TYPE_PAYMENT_FAILED:
            // Fallthrough
            case MegaUserAlert::TYPE_PAYMENTREMINDER:
            // Fallthrough
            case MegaUserAlert::TYPE_TAKEDOWN:
            // Fallthrough
            case MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
            {
                break;
            }
            case MegaUserAlert::TYPE_NEWSHARE:
            // Fallthrough
            case MegaUserAlert::TYPE_DELETEDSHARE:
            // Fallthrough
            case MegaUserAlert::TYPE_NEWSHAREDNODES:
            // Fallthrough
            case MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
            // Fallthrough
            case MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
            {
                mUi->bNotificationIcon->setFixedHeight(8);
                break;
            }
            default:
            {
                mUi->bNotificationIcon->setFixedHeight(16);
                break;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

QSize AlertItem::minimumSizeHint() const
{
    return this->size();
}

QSize AlertItem::sizeHint() const
{
    return this->size();
}

void AlertItem::mousePressEvent(QMouseEvent* event)
{
    if(!mAlertData)
    {
        return;
    }

    switch(mAlertData->getType())
    {
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
        {
            processIncomingPendingContactClick();
            break;
        }
        case MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED:
        {
            processIncomingContactChangeOrAcceptedClick();
            break;
        }
        case MegaUserAlert::TYPE_NEWSHARE:
        case MegaUserAlert::TYPE_DELETEDSHARE:
        case MegaUserAlert::TYPE_NEWSHAREDNODES:
        case MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
        case MegaUserAlert::TYPE_TAKEDOWN:
        case MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
        {
            processShareOrTakedownClick();
            break;
        }
        case MegaUserAlert::TYPE_PAYMENT_SUCCEEDED:
        case MegaUserAlert::TYPE_PAYMENT_FAILED:
        case MegaUserAlert::TYPE_PAYMENTREMINDER:
        {
            processPaymentClick();
            break;
        }
        default:
        {
            break;
        }
    }

    QWidget::mousePressEvent(event);
}

void AlertItem::processIncomingPendingContactClick()
{
    bool found = false;
    std::unique_ptr<MegaContactRequestList> icr(mMegaApi->getIncomingContactRequests());
    if (icr)
    {
        for (int i = 0; i < icr->size(); i++)
        {
            const MegaContactRequest* request = icr->get(i);
            if (!request)
            {
                continue;
            }

            const char* email = request->getSourceEmail();
            if (mAlertData && mAlertData->getEmail().toStdString().c_str() == email)
            {
                found = true;
                Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/ipc")));
                break;
            }
        }
    }

    if (!found)
    {
        Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/contacts")));
    }
}

void AlertItem::processIncomingContactChangeOrAcceptedClick()
{
    std::unique_ptr<MegaUser> user { mMegaApi->getContact(mAlertData->getEmail().toStdString().c_str()) };
    if (user && user->getVisibility() == MegaUser::VISIBILITY_VISIBLE)
    {
        Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/%1")
                                    .arg(QString::fromUtf8(mMegaApi->userHandleToBase64(user->getHandle())))));
    }
    else
    {
        Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/contacts")));
    }
}

void AlertItem::processShareOrTakedownClick()
{
    std::unique_ptr<MegaNode> node { mMegaApi->getNodeByHandle(mAlertData->getNodeHandle()) };
    if (node)
    {
        Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/%1")
                                    .arg(QString::fromUtf8(node->getBase64Handle()))));
    }
}

void AlertItem::processPaymentClick()
{
    Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/account/plan")));
}

bool AlertItem::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        updateAlertType();
        if (mAlertData)
        {
            setAlertHeading(mAlertData);
            setAlertContent(mAlertData);
            setAlertTimeStamp(mAlertData->getTimestamp(0));
        }
    }
    if (event->type() == ThemeManager::ThemeChanged)
    {
        if (mAlertData)
        {
            setAlertContent(mAlertData);
        }
    }
    return UserMessageWidget::event(event);
}

QString AlertItem::formatRichString(const QString& str)
{
    return QString::fromUtf8("<span style='color:#333333; font-family: Lato; font-size: 14px; font-weight: bold; text-decoration:none;'>%1</span>")
            .arg(str);
}

QString AlertItem::getUserFullName()
{
    if(mFullNameAttributes)
    {
        return mFullNameAttributes->getRichFullName();
    }

    return mAlertData ? mAlertData->getEmail() : QString::fromUtf8("");
}

