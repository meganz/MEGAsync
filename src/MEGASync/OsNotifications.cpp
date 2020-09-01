#include "megaapi.h"
#include "OsNotifications.h"
#include "MegaApplication.h"
#include <QCoreApplication>

OsNotifications::OsNotifications(Notificator *notificator)
    :mNotificator{notificator}
{
}

void OsNotifications::addUserAlertList(mega::MegaUserAlertList *alertList)
{
    for(int iAlert = 0; iAlert < alertList->size(); iAlert++)
    {
        const auto alert{alertList->get(iAlert)};
        // alerts are sent again after seen state updated, so lets only notify the unseen alerts
        if(!alert->getSeen())
        {
            auto notification{new MegaNotification()};
            switch (alert->getType())
            {
            case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
                notification->setTitle(QCoreApplication::translate("OsNotifications", "New Contact Request"));
                notification->setText(QCoreApplication::translate("OsNotifications","[A] sent you a contact request")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                notification->setData(QString::fromUtf8(alert->getEmail()));
                notification->setActions(QStringList() << QCoreApplication::translate("OsNotifications", "Accept") <<
                                         QCoreApplication::translate("OsNotifications", "Reject"));
                QObject::connect(notification, &MegaNotification::activated, this, &OsNotifications::incomingPendingRequest);
                break;
            case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
                notification->setTitle(QCoreApplication::translate("OsNotifications", "New Contact Request"));
                notification->setText(QCoreApplication::translate("OsNotifications","[A] cancelled their contact request")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                break;
            case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
                notification->setTitle(QCoreApplication::translate("OsNotifications", "New Contact Request"));
                notification->setText(QCoreApplication::translate("OsNotifications", "Reminder") + QStringLiteral(": ") +
                                      QCoreApplication::translate("OsNotifications", "You have a contact request"));
                break;
            case mega::MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
                notification->setTitle(QCoreApplication::translate("OsNotifications", "Contact Established"));
                notification->setText(QCoreApplication::translate("OsNotifications","[A] established you as a contact")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                break;
            case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
                notification->setTitle(QCoreApplication::translate("OsNotifications", "Contact Established"));
                notification->setText(QCoreApplication::translate("OsNotifications", "You ignored a contact request"));
                break;
            }

            mNotificator->notify(notification);
        }
    }
}

void OsNotifications::incomingPendingRequest(int activationButton)
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
            if(activationButton == MegaNotification::ActivationActionButtonClicked)
            {
               megaApp->getMegaApi()->replyContactRequest(request, mega::MegaContactRequest::REPLY_ACTION_ACCEPT);
            }
        }
    }
}
