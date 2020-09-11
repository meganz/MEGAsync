#include "megaapi.h"
#include "OsNotifications.h"
#include "MegaApplication.h"
#include "Platform.h"
#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>

const auto iconPrefix{QStringLiteral("://images/")};
const auto iconFolderName{QStringLiteral("icons")};
const auto newContactIconName{QStringLiteral("new_contact@3x.png")};
const auto storageQuotaFullIconName{QStringLiteral("Storage_Quota_full@3x.png")};
const auto storageQuotaWarningIconName{QStringLiteral("Storage_Quota_almost_full@3x.png")};
const auto failedToDownloadIconName{QStringLiteral("Failed_to_download@3x.png")};
const auto folderIconName{QStringLiteral("Folder@3x.png")};
const auto fileDownloadSucceedIconName{QStringLiteral("File_download_succeed@3x.png")};

void copyIconsToAppFolder(QString folderPath)
{
    QStringList iconNames;
    iconNames << newContactIconName << storageQuotaFullIconName << storageQuotaWarningIconName
              << failedToDownloadIconName << folderIconName << fileDownloadSucceedIconName;

    for(const auto& iconName : iconNames)
    {
        QFile iconFile(iconPrefix + iconName);
        iconFile.copy(folderPath + QDir::separator() + iconName);
    }
}

QString getIconsPath()
{
    return MegaApplication::applicationDataPath() + QDir::separator() + iconFolderName + QDir::separator();
}

OsNotifications::OsNotifications(const QString &appName, QSystemTrayIcon *trayIcon)
    :mAppIcon{QString::fromUtf8("://images/app_128.png")},
     mNewContactIconPath{getIconsPath() + newContactIconName},
     mStorageQuotaFullIconPath{getIconsPath() + storageQuotaFullIconName},
     mStorageQuotaWarningIconPath{getIconsPath() + storageQuotaWarningIconName},
     mFolderIconPath{getIconsPath() + folderIconName},
     mFileDownloadSucceedIconPath{getIconsPath() + fileDownloadSucceedIconName}
{
#ifdef __APPLE__
    mNotificator = new Notificator(appName, NULL, this);
#else
    mNotificator = new Notificator(appName, trayIcon, this);
#endif

    QDir appDir{MegaApplication::applicationDataPath()};
    appDir.mkdir(iconFolderName);
    copyIconsToAppFolder(getIconsPath());
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

QString createPaymentReminderText(int64_t expirationTimeStamp)
{
    QDateTime expiredDate;
    expiredDate.setMSecsSinceEpoch(expirationTimeStamp * 1000);
    QDateTime currentDate(QDateTime::currentDateTime());

    const auto daysExpired{currentDate.daysTo(expiredDate)};
    if (daysExpired == 1)
    {
        return QCoreApplication::translate("OsNotifications", "Your PRO membership plan will expire in 1 day");
    }
    else if (daysExpired > 0)
    {
        return QCoreApplication::translate("OsNotifications", "Your PRO membership plan will expire in [A] days")
                .replace(QString::fromUtf8("[A]"), QString::number(daysExpired));
    }
    else if (daysExpired == 0)
    {
        return QCoreApplication::translate("OsNotifications", "PRO membership plan expiring soon");
    }
    else if (daysExpired == -1)
    {
        return QCoreApplication::translate("OsNotifications", "Your PRO membership plan expired 1 day ago");
    }
    else
    {
        return QCoreApplication::translate("OsNotifications", "Your PRO membership plan expired [A] days ago")
                .replace(QString::fromUtf8("[A]"), QString::number(-daysExpired));
    }
}

void OsNotifications::addUserAlertList(mega::MegaUserAlertList *alertList) const
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
                notification->setImage(mAppIcon);
                notification->setImagePath(mNewContactIconPath);
                notification->setActions(QStringList() << tr("Accept") << tr("Reject"));
                QObject::connect(notification, &MegaNotification::activated, this, &OsNotifications::replayIncomingPendingRequest);
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(tr("New Contact Request"));
                notification->setText(tr("[A] cancelled the contact request")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail())));
                notification->setImage(mAppIcon);
                notification->setImagePath(mNewContactIconPath);
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(tr("New Contact Request"));
                notification->setText(tr("Reminder") + QStringLiteral(": ") +
                                      tr("You have a contact request"));
                notification->setImage(mAppIcon);
                notification->setImagePath(mNewContactIconPath);
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
                notification->setImage(mAppIcon);
                notification->setImagePath(mNewContactIconPath);
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
                notification->setImage(mAppIcon);
                notification->setImagePath(mNewContactIconPath);
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(tr("Contact Updated"));
                notification->setText(tr("You ignored a contact request"));
                notification->setImage(mAppIcon);
                notification->setImagePath(mNewContactIconPath);
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(tr("Contact Updated"));
                notification->setText(tr("You denied a contact request"));
                notification->setImage(mAppIcon);
                notification->setImagePath(mNewContactIconPath);
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_NEWSHARE:
            {
                const auto message{tr("New Shared folder from [X]")
                            .replace(QString::fromUtf8("[X]"), QString::fromUtf8(alert->getEmail()))};
                notifySharedUpdate(alert, message);
                break;
            }
            case mega::MegaUserAlert::TYPE_DELETEDSHARE:
            {
                const auto message{tr("[A] has left the shared folder")
                                      .replace(QString::fromUtf8("[A]"), QString::fromUtf8(alert->getEmail()))};
                notifySharedUpdate(alert, message);
                break;
            }
            case mega::MegaUserAlert::TYPE_NEWSHAREDNODES:
            {
                notifySharedUpdate(alert, getItemsAddedText(alert));
                break;
            }
            case mega::MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
            {
                notifySharedUpdate(alert, getItemsRemovedText(alert));
                break;
            }
            case mega::MegaUserAlert::TYPE_PAYMENTREMINDER:
            {
                auto notification{new MegaNotification()};
                notification->setTitle(tr("Payment Info"));
                constexpr auto paymentReminderIndex{1};
                notification->setText(createPaymentReminderText(alert->getTimestamp(paymentReminderIndex)));
                notification->setActions(QStringList() << tr("Upgrade"));
                notification->setImage(mAppIcon);
                connect(notification, &MegaNotification::activated, this, &OsNotifications::redirectToUpgrade);
                mNotificator->notify(notification);
                break;
            }
            case mega::MegaUserAlert::TYPE_TAKEDOWN:
            case mega::MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
            {
                notifyTakeDownReinstated(alert);
                break;
            }
            default:
                break;
            }
        }
    }
}

void OsNotifications::replayIncomingPendingRequest(MegaNotification::Action action) const
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

std::unique_ptr<mega::MegaNode> getMegaNode(mega::MegaUserAlert* alert)
{
    const auto megaApi{static_cast<MegaApplication*>(qApp)->getMegaApi()};
    return std::unique_ptr<mega::MegaNode>(megaApi->getNodeByHandle(alert->getNodeHandle()));
}

void OsNotifications::notifySharedUpdate(mega::MegaUserAlert *alert, const QString& message) const
{
    auto notification{new MegaNotification()};
    const auto node{getMegaNode(alert)};
    auto sharedFolderName{QString::fromUtf8(node ? node->getName() : alert->getName())};
    if(sharedFolderName.isEmpty())
    {
        sharedFolderName = QCoreApplication::translate("OsNotifications", "Shared Folder Activity");
    }
    notification->setTitle(sharedFolderName);
    notification->setText(message);
    if(node)
    {
        notification->setData(QString::fromUtf8(node->getBase64Handle()));
        notification->setActions(QStringList() << tr("Show"));
        QObject::connect(notification, &MegaNotification::activated, this, &OsNotifications::viewShareOnWebClient);
    }
    notification->setImage(mAppIcon);
    notification->setImagePath(mFolderIconPath);
    mNotificator->notify(notification);
}

QString createTakeDownMessage(mega::MegaUserAlert* alert)
{
    const auto megaApi{static_cast<MegaApplication*>(qApp)->getMegaApi()};
    const auto node{megaApi->getNodeByHandle(alert->getNodeHandle())};
    if (node)
    {
        if (node->getType() == mega::MegaNode::TYPE_FILE)
        {
            return QCoreApplication::translate("OsNotifications", "Your publicly shared file ([A]) has been taken down")
                    .replace(QString::fromUtf8("[A]"), QString::fromUtf8(node->getName()));
        }
        else if (node->getType() == mega::MegaNode::TYPE_FOLDER)
        {
            return QCoreApplication::translate("OsNotifications", "Your publicly shared folder ([A]) has been taken down")
                    .replace(QString::fromUtf8("[A]"), QString::fromUtf8(node->getName()));
        }
        else
        {
            return QCoreApplication::translate("OsNotifications", "Your publicly shared has been taken down");
        }

        delete node;
    }
    else
    {
        return QCoreApplication::translate("OsNotifications", "Your publicly shared has been taken down");
    }
}

void OsNotifications::notifyTakeDownReinstated(mega::MegaUserAlert *alert) const
{
    auto notification{new MegaNotification()};
    const auto node{getMegaNode(alert)};
    notification->setTitle(tr("Takedown Notice"));
    notification->setText(createTakeDownMessage(alert));
    if(node)
    {
        notification->setData(QString::fromUtf8(node->getBase64Handle()));
        notification->setActions(QStringList() << tr("Show"));
        QObject::connect(notification, &MegaNotification::activated, this, &OsNotifications::viewShareOnWebClient);
    }
    notification->setImage(mAppIcon);
    mNotificator->notify(notification);
}

void OsNotifications::viewContactOnWebClient() const
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

void OsNotifications::sendOverStorageNotification(int state) const
{
    switch (state)
    {
    case Preferences::STATE_ALMOST_OVER_STORAGE:
    {
        auto notification{new MegaNotification()};
        notification->setTitle(tr("Your account is almost full."));
        notification->setText(tr("Upgrade now to a PRO account."));
        notification->setActions(QStringList() << tr("Get PRO"));
        notification->setImage(mAppIcon);
        notification->setImagePath(mStorageQuotaWarningIconPath);
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
        notification->setImage(mAppIcon);
        notification->setImagePath(mStorageQuotaFullIconPath);
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
        notification->setImage(mAppIcon);
        connect(notification, &MegaNotification::activated, this, &OsNotifications::redirectToUpgrade);
        mNotificator->notify(notification);
        break;
    }
    default:
        break;
    }
}

void OsNotifications::sendOverTransferNotification(const QString &title) const
{
    const auto notification{new MegaNotification()};
    notification->setTitle(title);
    notification->setText(tr("Upgrade now to a PRO account."));
    notification->setActions(QStringList() << tr("Get PRO"));
    notification->setImage(mAppIcon);
    connect(notification, &MegaNotification::activated, this, &OsNotifications::redirectToUpgrade);
    mNotificator->notify(notification);
}

void OsNotifications::sendFinishedTransferNotification(const QString &title, const QString &message, const QString &extraData) const
{
    auto notification{new MegaNotification()};
    notification->setTitle(title);
    notification->setText(message);
    notification->setActions(QStringList() << tr("Show in folder"));
    notification->setData(extraData);
    notification->setImage(mAppIcon);
    notification->setImagePath(mFileDownloadSucceedIconPath);
    connect(notification, &MegaNotification::activated, this, &OsNotifications::showInFolder);
    mNotificator->notify(notification);
}

bool checkIfActionIsValid(MegaNotification::Action action)
{
    return action == MegaNotification::Action::firstButton
            || action == MegaNotification::Action::legacy
        #ifndef _WIN32
            || action == MegaNotification::Action::content
        #endif
            ;
}

void OsNotifications::redirectToUpgrade(MegaNotification::Action activationButton) const
{
    if (checkIfActionIsValid(activationButton))
    {
        QString url = QString::fromUtf8("mega://#pro");
        Utilities::getPROurlWithParameters(url);
        QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
    }
}

void OsNotifications::sendBusinessWarningNotification(int businessStatus) const
{
    const auto megaApi{static_cast<MegaApplication*>(qApp)->getMegaApi()};

    switch (businessStatus)
    {
    case mega::MegaApi::BUSINESS_STATUS_GRACE_PERIOD:
    {
        if (megaApi->isMasterBusinessAccount())
        {
            const auto notification{new MegaNotification()};
            notification->setTitle(tr("Payment Failed"));
            notification->setText(tr("Please resolve your payment issue to avoid suspension of your account."));
            notification->setActions(QStringList() << tr("Pay Now"));
            notification->setImage(mAppIcon);
            connect(notification, &MegaNotification::activated, this, &OsNotifications::redirectToPayBusiness);
            mNotificator->notify(notification);
        }
        break;
    }
    case mega::MegaApi::BUSINESS_STATUS_EXPIRED:
    {
        const auto notification{new MegaNotification()};

        if (megaApi->isMasterBusinessAccount())
        {
            notification->setTitle(tr("Your Business account is expired"));
            notification->setText(tr("Your account is suspended as read only until you proceed with the needed payments."));
            notification->setActions(QStringList() << tr("Pay Now"));
            notification->setImage(mAppIcon);
            connect(notification, &MegaNotification::activated, this, &OsNotifications::redirectToPayBusiness);
        }
        else
        {
            notification->setTitle(tr("Account Suspended"));
            notification->setText(tr("Contact your business account administrator to resolve the issue and activate your account."));
        }
        mNotificator->notify(notification);
        break;
    }
    default:
        break;
    }
}

void OsNotifications::sendInfoNotification(const QString &title, const QString &message) const
{
    mNotificator->notify(Notificator::Information, title, message, mAppIcon);
}

void OsNotifications::sendWarningNotification(const QString &title, const QString &message) const
{
    mNotificator->notify(Notificator::Warning, title, message, mAppIcon);
}

void OsNotifications::sendErrorNotification(const QString &title, const QString &message) const
{
    mNotificator->notify(Notificator::Warning, title, message, mAppIcon);
}

void OsNotifications::redirectToPayBusiness(MegaNotification::Action activationButton) const
{
    if (checkIfActionIsValid(activationButton))
    {
        QString url = QString::fromUtf8("mega://#repay");
        Utilities::getPROurlWithParameters(url);
        QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
    }
}

void OsNotifications::showInFolder(MegaNotification::Action action) const
{
    const auto notification{static_cast<MegaNotification*>(QObject::sender())};

    if (checkIfActionIsValid(action) && notification->getData().size() > 1)
    {
        QString localPath = QDir::toNativeSeparators(notification->getData().mid(1));
        if (notification->getData().at(0) == QChar::fromAscii('1'))
        {
            Platform::showInFolder(localPath);
        }
        else
        {
            QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(localPath));
        }
    }
}

void OsNotifications::viewShareOnWebClient(MegaNotification::Action action) const
{
    if (checkIfActionIsValid(action))
    {
        const auto notification{static_cast<MegaNotification*>(QObject::sender())};
        const auto nodeHandlerBase64{notification->getData()};
        if (!nodeHandlerBase64.isEmpty())
        {
            const auto url{QUrl(QString::fromUtf8("mega://#fm/%1").arg(nodeHandlerBase64))};
            QtConcurrent::run(QDesktopServices::openUrl, url);
        }
    }
}
