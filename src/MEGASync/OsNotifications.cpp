#include "megaapi.h"
#include "OsNotifications.h"
#include "MegaApplication.h"
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>

OsNotifications::OsNotifications(Notificator *notificator)
    :mNotificator{notificator}
{
}

QString getSharedFolderName(mega::MegaUserAlert* alert)
{
    const auto megaApi{static_cast<MegaApplication*>(qApp)->getMegaApi()};
    const auto node{megaApi->getNodeByHandle(alert->getNodeHandle())};
    auto sharedFolderName{QString::fromUtf8(node ? node->getName() : alert->getName())};
    if(!sharedFolderName.isEmpty())
    {
        return sharedFolderName;
    }
    return QCoreApplication::translate("OsNotifications", "Shared Folder Activity");
}

QString getItemsAddedText(mega::MegaUserAlert* alert)
{
    const auto updatedItems{alert->getNumber(1) + alert->getNumber(0)};
    if (updatedItems == 1)
    {
        return QCoreApplication::translate("OsNotifications", "[A] added 1 item")
                .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail()));
    }
    else
    {
         return QCoreApplication::translate("OsNotifications", "[A] added [B] items")
                 .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail()))
                 .replace(QString::fromUtf8("[B]"), QString::number(updatedItems));
    }
}

QString getItemsRemovedText(mega::MegaUserAlert* alert)
{
    const auto updatedItems{alert->getNumber(0)};
    if (updatedItems == 1)
    {
        return QCoreApplication::translate("OsNotifications", "[A] removed 1 item")
                .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail()));
    }
    else
    {
         return QCoreApplication::translate("OsNotifications", "[A] removed [B] items")
                 .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail()))
                 .replace(QString::fromUtf8("[B]"), QString::number(updatedItems));
    }
}

void OsNotifications::addUserAlertList(mega::MegaUserAlertList *alertList)
{
    for(int iAlert = 0; iAlert < alertList->size(); iAlert++)
    {
        const auto alert{alertList->get(iAlert)};
        // alerts are sent again after seen state updated, so lets only notify the unseen alerts
        if(!alert->getSeen())
        {
            switch (alert->getType())
            {
            case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(tr("New Contact Request"));
                notification->setText(tr("[A] sent you a contact request")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                notification->setData(QString::fromUtf8(alert->getEmail()));
                notification->setActions(QStringList() << tr("Accept")
                                         << tr("Reject"));
                QObject::connect(notification, &MegaNotification::activated, this, &OsNotifications::incomingPendingRequest);
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(tr("New Contact Request"));
                notification->setText(tr("[A] cancelled the contact request")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(tr("New Contact Request"));
                notification->setText(tr("Reminder") + QStringLiteral(": ") +
                                      tr("You have a contact request"));
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(tr("Contact Established"));
                notification->setText(tr("New contact with [A] has been established")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                notification->setData(QString::fromUtf8(alert->getEmail()));
                notification->setActions(QStringList() << tr("View"));
                QObject::connect(notification, &MegaNotification::activated, this, &OsNotifications::viewContactOnWebClient);
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(tr("Contact Updated"));
                notification->setText(QCoreApplication::translate("OsNotifications","You accepted a contact request")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(tr("Contact Updated"));
                notification->setText(tr("You ignored a contact request"));
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(tr("Contact Updated"));
                notification->setText(tr("You denied a contact request"));
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_NEWSHARE:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(getSharedFolderName(alert));
                notification->setText(tr("New Shared folder from [X]")
                                      .replace(QString::fromUtf8("[X]"), QString::fromUtf8(alert->getEmail())));
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_DELETEDSHARE:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(getSharedFolderName(alert));
                notification->setText(tr("[A] has left the shared folder")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_NEWSHAREDNODES:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(getSharedFolderName(alert));
                notification->setText(getItemsAddedText(alert));
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(getSharedFolderName(alert));
                notification->setText(getItemsRemovedText(alert));
                mNotificator->notify(notification);
                break;
            }
            default:
                break;
            }
        }
    }
}

void OsNotifications::incomingPendingRequest(MegaNotification::Action action)
{
    const auto notification{static_cast<MegaNotification*>(QObject::sender())};
    const auto megaApp{static_cast<MegaApplication*>(qApp)};
    const auto requestList{megaApp->getMegaApi()->getIncomingContactRequests()};
    const auto sourceEmail{notification->getData()};

    for(int iRequest=0; iRequest < requestList->size(); iRequest++)
    {
        const auto request{requestList->get(iRequest)};
        if(QString::fromUtf8(request->getSourceEmail()) == sourceEmail)
        {
            if(action == MegaNotification::Action::firstButton)
            {
               megaApp->getMegaApi()->replyContactRequest(request, mega::MegaContactRequest::REPLY_ACTION_ACCEPT);
            }
            else if(action == MegaNotification::Action::secondButton)
            {
               megaApp->getMegaApi()->replyContactRequest(request, mega::MegaContactRequest::REPLY_ACTION_DENY);
            }
        }
    }
}

void OsNotifications::viewContactOnWebClient()
{
    const auto notification{static_cast<MegaNotification*>(QObject::sender())};
    const auto megaApi{static_cast<MegaApplication*>(qApp)->getMegaApi()};
    const auto user{megaApi->getContact(notification->getData().toUtf8())};
    const auto userVisible{user && user->getVisibility() == mega::MegaUser::VISIBILITY_VISIBLE};
    const auto userHandle{QString::fromUtf8(megaApi->userHandleToBase64(user->getHandle()))};
    auto url{QUrl(QString::fromUtf8("mega://#fm/contacts"))};

    if (userVisible)
    {
        url = QUrl(QString::fromUtf8("mega://#fm/%1").arg(userHandle));
    }
    QtConcurrent::run(QDesktopServices::openUrl, url);
    delete user;
}

void OsNotifications::sendOverStorageNotification(int state)
{
    switch (state)
    {
    case Preferences::STATE_ALMOST_OVER_STORAGE:
    {
        auto notification{new MegaNotification()};
        notification->setTitle(tr("Your account is almost full."));
        notification->setText(tr("Upgrade now to a PRO account."));
        notification->setActions(QStringList() << tr("Get PRO"));
        connect(notification, &MegaNotification::activated, this, &OsNotifications::redirectToUpgrade);
        mNotificator->notify(notification);
        break;
    }
    case Preferences::STATE_OVER_STORAGE:
    {
        auto notification{new MegaNotification()};
        notification->setTitle(tr("Your account is full."));
        notification->setText(tr("Upgrade now to a PRO account."));
        notification->setActions(QStringList() << tr("Get PRO"));
        connect(notification, &MegaNotification::activated, this, &OsNotifications::redirectToUpgrade);
        mNotificator->notify(notification);
        break;
    }
    case Preferences::STATE_PAYWALL:
    {
        auto notification{new MegaNotification()};
        notification->setTitle(tr("Your data is at risk"));
        const auto megaApi{static_cast<MegaApplication*>(qApp)->getMegaApi()};
        const auto daysToTimeStamps{QString::number(Utilities::getDaysToTimestamp(megaApi->getOverquotaDeadlineTs() * 1000))};
        notification->setText(tr("You have [A] days left to save your data").replace(QString::fromUtf8("[A]"), daysToTimeStamps));
        notification->setActions(QStringList() << tr("Get PRO"));
        connect(notification, &MegaNotification::activated, this, &OsNotifications::redirectToUpgrade);
        mNotificator->notify(notification);
        break;
    }
    default:
        break;
    }
}

void OsNotifications::sendOverTransferNotification(const QString &title)
{
    const auto notification{new MegaNotification()};
    notification->setTitle(title);
    notification->setText(tr("Upgrade now to a PRO account."));
    notification->setActions(QStringList() << tr("Get PRO"));
    connect(notification, &MegaNotification::activated, this, &OsNotifications::redirectToUpgrade);
    mNotificator->notify(notification);
}

void OsNotifications::redirectToUpgrade(MegaNotification::Action activationButton)
{
    if (activationButton == MegaNotification::Action::firstButton
            || activationButton == MegaNotification::Action::legacy
        #ifndef _WIN32
            || activationButton == MegaNotification::Action::content
        #endif
            )
    {
        QString url = QString::fromUtf8("mega://#pro");
        Utilities::getPROurlWithParameters(url);
        QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
    }
}
