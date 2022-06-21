#include "megaapi.h"
#include "CommonMessages.h"
#include "DesktopNotifications.h"
#include "MegaApplication.h"
#include "mega/user.h"
#include "Platform.h"
#include "UserAttributesRequests.h"

#include <QCoreApplication>
#include <QtConcurrent/QtConcurrent>

const QString iconPrefix{QStringLiteral("://images/")};
const QString iconFolderName{QStringLiteral("icons")};
const QString newContactIconName{QStringLiteral("new_contact@3x.png")};
const QString storageQuotaFullIconName{QStringLiteral("Storage_Quota_full@3x.png")};
const QString storageQuotaWarningIconName{QStringLiteral("Storage_Quota_almost_full@3x.png")};
const QString failedToDownloadIconName{QStringLiteral("Failed_to_download@3x.png")};
const QString folderIconName{QStringLiteral("Folder@3x.png")};
const QString fileDownloadSucceedIconName{QStringLiteral("File_download_succeed@3x.png")};
constexpr int maxNumberOfUnseenNotifications{3};

bool checkIfActionIsValid(MegaNotification::Action action)
{
    return action == MegaNotification::Action::firstButton
            || action == MegaNotification::Action::legacy
        #ifndef _WIN32
            || action == MegaNotification::Action::content
        #endif
            ;
}

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

DesktopNotifications::DesktopNotifications(const QString &appName, QSystemTrayIcon *trayIcon)
    :mAppIcon(QString::fromUtf8("://images/app_128.png")),
     mNewContactIconPath(getIconsPath() + newContactIconName),
     mStorageQuotaFullIconPath(getIconsPath() + storageQuotaFullIconName),
     mStorageQuotaWarningIconPath(getIconsPath() + storageQuotaWarningIconName),
     mFolderIconPath(getIconsPath() + folderIconName),
     mFileDownloadSucceedIconPath(getIconsPath() + fileDownloadSucceedIconName),
     mPreferences(Preferences::instance()),
     mIsFirstTime(true)
{
#ifdef __APPLE__
    mNotificator = new Notificator(appName, NULL, this);
#else
    mNotificator = new Notificator(appName, trayIcon, this);
#endif

    QDir appDir{MegaApplication::applicationDataPath()};
    appDir.mkdir(iconFolderName);
    copyIconsToAppFolder(getIconsPath());

    QObject::connect(&mRemovedSharedNotificator, &RemovedSharesNotificator::sendClusteredAlert, this, &DesktopNotifications::receiveClusteredAlert);
}

QString DesktopNotifications::getItemsAddedText(mega::MegaUserAlert *info)
{
    const int updatedItems = static_cast<int>(info->getNumber(1) + info->getNumber(0));
    auto FullNameRequest = mUserAttributes.value(QString::fromUtf8(info->getEmail()));
    QString message(tr("[A] added %n item", "", updatedItems));
    if(FullNameRequest)
    {
        return message
                .replace(QString::fromUtf8("[A]"), FullNameRequest->getFullName());
    }
    else
    {
        return message
                .replace(QString::fromUtf8("[A]"), QString::fromUtf8(info->getEmail()));
    }
}

QString DesktopNotifications::createDeletedShareMessage(mega::MegaUserAlert* info)
{
    QString message;
    QString name;

    const bool someoneLeftTheFolder{info->getNumber(0) == 0};
    auto FullNameRequest = mUserAttributes.value(QString::fromUtf8(info->getEmail()));
    if(FullNameRequest)
    {
        name = FullNameRequest->getFullName();
    }
    else
    {
        name = QString::fromUtf8(info->getEmail());
    }

    if (someoneLeftTheFolder)
    {
        message = tr("[A] has left the shared folder")
                .replace(QString::fromUtf8("[A]"), name);
    }
    else //Access for the user was removed by share owner
    {
        message = name.isEmpty() ? tr("Access to shared folder was removed") :
                                       tr("Access to shared folder was removed by [A]")
                                       .replace(QString::fromUtf8("[A]"), name);
    }

    return message;
}

int DesktopNotifications::countUnseenAlerts(mega::MegaUserAlertList *alertList)
{
    auto count = 0;
    for(int iAlert = 0; iAlert < alertList->size(); iAlert++)
    {
        if(!alertList->get(iAlert)->getSeen())
        {
            count++;
        }
    }
    return count;
}

void DesktopNotifications::addUserAlertList(mega::MegaUserAlertList *alertList)
{
    if(mPreferences->isAnyNotificationEnabled())
    {
        const auto unseenAlertsCount = countUnseenAlerts(alertList);
        const bool tooManyAlertsUnseen{unseenAlertsCount > maxNumberOfUnseenNotifications};
        if(tooManyAlertsUnseen || (mIsFirstTime && unseenAlertsCount))
        {
            mIsFirstTime = false;
            notifyUnreadNotifications();
            return;
        }
        mIsFirstTime = false;
    }

    for(int iAlert = 0; iAlert < alertList->size(); iAlert++)
    {
        const auto alert = alertList->get(iAlert);

        // alerts are sent again after seen state updated, so lets only notify the unseen alerts
        if(!alert->getSeen())
        {
            auto userEmail = QString::fromUtf8(alert->getEmail());

            if(!userEmail.isEmpty())
            {
                auto fullNameUserAttributes = UserAttributes::FullNameAttributeRequest::requestFullName(alert->getEmail());
                if(fullNameUserAttributes && !mUserAttributes.contains(userEmail))
                {
                    mUserAttributes.insert(userEmail, fullNameUserAttributes);
                    connect(fullNameUserAttributes.get(), &UserAttributes::FullNameAttributeRequest::attributeReady,
                            this, &DesktopNotifications::OnUserAttributesReady, Qt::UniqueConnection);
                }

                if(fullNameUserAttributes && !fullNameUserAttributes->isAttributeReady())
                {
                    mPendingUserAlerts.insert(userEmail, alert->copy());
                }
                else
                {
                    processAlert(alert);
                }
            }
            else
            {
                processAlert(alert);
            }
        }
    }
}

void DesktopNotifications::processAlert(mega::MegaUserAlert* alert)
{
    QString fullName;
    QString email = QString::fromUtf8(alert->getEmail());
    if (!email.isEmpty())
    {
        auto fullNameRequest = mUserAttributes.value(email);
        if (fullNameRequest)
        {
            fullName = fullNameRequest->getFullName();
        }
    }

    switch (alert->getType())
    {
    case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
    {
        if(mPreferences->isNotificationEnabled(Preferences::NotificationsTypes::NEW_CONTACT_REQUESTS))
        {
            QStringList actions(tr("Accept"));
#ifndef __APPLE__
            actions << tr("Reject");
#endif

            auto notification = CreateContacNotification(tr("New Contact Request"),
                                                         tr("[A] sent you a contact request").replace(QString::fromUtf8("[A]"), fullName),
                                                         email,
                                                         actions);

            QObject::connect(notification, &MegaNotification::activated, this, &DesktopNotifications::replayIncomingPendingRequest);

        }
        break;
    }
    case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
    {
        //Kept to remind decision about this notification
        //This notification is sent when the user cancels a incoming pending notification
        //The current implementation on the SDK filters this kind of notifications, and
        //all "own-caused-user" notifications are blocked.
        //However, as in MEGA Desktop App this notification has been developed,
        //only the last step (notification sending) has been removed just in case it needs to be used again.

        break;
    }
    case mega::MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
    {
        if(mPreferences->isNotificationEnabled(Preferences::NotificationsTypes::PENDING_CONTACT_REQUEST_REMINDER))
        {
            auto notification = CreateContacNotification(tr("New Contact Request"),
                                                         tr("Reminder") + QStringLiteral(": ") + tr("You have a contact request"),
                                                         email,
                                                         QStringList() << tr("View"));

            QObject::connect(notification, &MegaNotification::activated, this, &DesktopNotifications::viewContactOnWebClient);
        }
        break;
    }
    case mega::MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
    {
        if(mPreferences->isNotificationEnabled(Preferences::NotificationsTypes::CONTACT_ESTABLISHED))
        {
            QStringList actions(tr("Accept"));
#ifndef __APPLE__
            actions << tr("Chat");
#endif

            auto notification = CreateContacNotification(tr("New Contact Established"),
                                                         tr("New contact with [A] has been established").replace(QString::fromUtf8("[A]"), fullName),
                                                         email,
                                                         actions);

            QObject::connect(notification, &MegaNotification::activated, this, &DesktopNotifications::viewContactOnWebClient);
        }
        break;
    }
    case mega::MegaUserAlert::TYPE_NEWSHARE:
    {
        if(mPreferences->isNotificationEnabled(Preferences::NotificationsTypes::NEW_FOLDERS_SHARED_WITH_ME))
        {
            const QString message{tr("New shared folder from [A]")
                        .replace(QString::fromUtf8("[A]"), fullName)};
            notifySharedUpdate(alert, message, NEW_SHARE);
        }
        break;
    }
    case mega::MegaUserAlert::TYPE_DELETEDSHARE:
    {
        if(mPreferences->isNotificationEnabled(Preferences::NotificationsTypes::FOLDERS_SHARED_WITH_ME_DELETED))
        {
            notifySharedUpdate(alert, createDeletedShareMessage(alert), DELETE_SHARE);
        }
        break;
    }
    case mega::MegaUserAlert::TYPE_NEWSHAREDNODES:
    {
        if(mPreferences->isNotificationEnabled(Preferences::NotificationsTypes::NODES_SHARED_WITH_ME_CREATED_OR_REMOVED))
        {
            notifySharedUpdate(alert, getItemsAddedText(alert), NEW_SHARED_NODES);
        }
        break;
    }
    case mega::MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
    {
        if(mPreferences->isNotificationEnabled(Preferences::NotificationsTypes::NODES_SHARED_WITH_ME_CREATED_OR_REMOVED))
        {
            mRemovedSharedNotificator.addUserAlert(alert, fullName);
        }
        break;
    }
    case mega::MegaUserAlert::TYPE_PAYMENTREMINDER:
    {
        auto notification = new MegaNotification();
        notification->setTitle(tr("Payment Info"));
        constexpr int paymentReminderIndex{1};
        notification->setText(CommonMessages::createPaymentReminder(alert->getTimestamp(paymentReminderIndex)));
        notification->setActions(QStringList() << tr("Upgrade"));
        notification->setImage(mAppIcon);
        connect(notification, &MegaNotification::activated, this, &DesktopNotifications::redirectToUpgrade);
        mNotificator->notify(notification);
        break;
    }
    case mega::MegaUserAlert::TYPE_TAKEDOWN:
    {
        notifyTakeDown(alert, false);
        break;
    }
    case mega::MegaUserAlert::TYPE_TAKEDOWN_REINSTATED:
    {
        notifyTakeDown(alert, true);
        break;
    }
    default:
        break;
    }
}

MegaNotification* DesktopNotifications::CreateContacNotification(const QString& title,
                                                                 const QString& message,
                                                                 const QString& email,
                                                                 const QStringList& actions)
{
    //No need to delete it after using, the class itself deletes it when activated or closed

    auto notification = new MegaNotification();
    notification->setTitle(title);
    notification->setText(message);
    notification->setData(email);
    notification->setImage(mAppIcon);
    notification->setImagePath(mNewContactIconPath);
    notification->setActions(actions);

    mNotificator->notify(notification);

    return notification;
}

void DesktopNotifications::replayIncomingPendingRequest(MegaNotification::Action action) const
{
    const auto notification = static_cast<MegaNotification*>(QObject::sender());
    const auto megaApp = static_cast<MegaApplication*>(qApp);
    const auto requestList = megaApp->getMegaApi()->getIncomingContactRequests();
    const auto sourceEmail = notification->getData();

    for(int iRequest=0; iRequest < requestList->size(); iRequest++)
    {
        const auto request = requestList->get(iRequest);
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
    const auto megaApi = static_cast<MegaApplication*>(qApp)->getMegaApi();
    return std::unique_ptr<mega::MegaNode>(megaApi->getNodeByHandle(alert->getNodeHandle()));
}

void DesktopNotifications::notifySharedUpdate(mega::MegaUserAlert *alert, const QString& message, int type) const
{
    auto notification = new MegaNotification();
    const auto node = getMegaNode(alert);
    QString sharedFolderName{QString::fromUtf8(node ? node->getName() : alert->getName())};
    if(sharedFolderName.isEmpty())
    {
        switch (type) {
            case NEW_SHARE:
                sharedFolderName = tr("Shared Folder Received");
                break;
            case DELETE_SHARE:
                sharedFolderName = tr("Shared Folder Removed");
                break;
            case NEW_SHARED_NODES:
            case REMOVED_SHARED_NODES:
                sharedFolderName = tr("Shared Folder Updated");
                break;
            default:
                sharedFolderName = tr("Shared Folder Activity");
                break;
        }
    }
    notification->setTitle(sharedFolderName);
    notification->setText(message);
    if(node)
    {
        notification->setData(QString::fromUtf8(node->getBase64Handle()));
        const auto megaApi = static_cast<MegaApplication*>(qApp)->getMegaApi();
        const auto fullAccess = megaApi->getAccess(node.get()) >= mega::MegaShare::ACCESS_FULL;
        if(type == NEW_SHARE && fullAccess)
        {
#ifdef __APPLE__
            //Apple do not support multi option with notifications. Just provide default one.
            notification->setActions(QStringList() << tr("Show"));
#else
            notification->setActions(QStringList() << tr("Show") << tr("Sync"));
#endif

            QObject::connect(notification, &MegaNotification::activated, this, &DesktopNotifications::replayNewShareReceived);
        }
        else
        {
            notification->setActions(QStringList() << tr("Show"));
            QObject::connect(notification, &MegaNotification::activated, this, &DesktopNotifications::viewShareOnWebClient);
        }

    }
    notification->setImage(mAppIcon);
    notification->setImagePath(mFolderIconPath);
    mNotificator->notify(notification);
}

void DesktopNotifications::notifyUnreadNotifications() const
{
    auto notification = new MegaNotification();
    notification->setText(tr("You have unread notifications"));
    notification->setImage(mAppIcon);
    notification->setActions(QStringList() << tr("View"));
    QObject::connect(notification, &MegaNotification::activated, this, &DesktopNotifications::viewOnInfoDialogNotifications);
    mNotificator->notify(notification);
}

QString DesktopNotifications::createTakeDownMessage(mega::MegaUserAlert* alert, bool isReinstated) const
{
    const auto megaApi = static_cast<MegaApplication*>(qApp)->getMegaApi();
    std::unique_ptr<mega::MegaNode> node(megaApi->getNodeByHandle(alert->getNodeHandle()));
    if (node)
    {
        if (node->getType() == mega::MegaNode::TYPE_FILE)
        {
            QString message = isReinstated ? tr("Your publicly shared file ([A]) has been reinstated")
                                             .replace(QString::fromUtf8("[A]"), QString::fromUtf8(node->getName()))
                                           : tr("Your publicly shared file ([A]) has been taken down")
                                             .replace(QString::fromUtf8("[A]"), QString::fromUtf8(node->getName()));
            return message;
        }
        else if (node->getType() == mega::MegaNode::TYPE_FOLDER)
        {

            QString message = isReinstated ? tr("Your publicly shared folder ([A]) has been reinstated")
                                             .replace(QString::fromUtf8("[A]"), QString::fromUtf8(node->getName()))
                                           : tr("Your publicly shared folder ([A]) has been taken down")
                                             .replace(QString::fromUtf8("[A]"), QString::fromUtf8(node->getName()));
            return message;
        }
        else
        {
            QString message = isReinstated ? tr("Your taken down has been reinstated")
                                           : tr("Your publicly shared has been taken down");
            return message;
        }
    }
    else
    {
        QString message = isReinstated ? tr("Your taken down has been reinstated")
                                       : tr("Your publicly shared has been taken down");
        return message;
    }
}

void DesktopNotifications::notifyTakeDown(mega::MegaUserAlert *alert, bool isReinstated) const
{
    auto notification = new MegaNotification();
    const auto node = getMegaNode(alert);
    notification->setTitle(tr("Takedown Notice"));
    notification->setText(createTakeDownMessage(alert, isReinstated));
    if(node)
    {
        notification->setData(QString::fromUtf8(node->getBase64Handle()));
        notification->setActions(QStringList() << tr("Show"));
        QObject::connect(notification, &MegaNotification::activated, this, &DesktopNotifications::viewShareOnWebClient);
    }
    notification->setImage(mAppIcon);
    mNotificator->notify(notification);
}

void DesktopNotifications::viewContactOnWebClient(MegaNotification::Action activationButton) const
{
    const auto notification = static_cast<MegaNotification*>(QObject::sender());
    const auto megaApi = static_cast<MegaApplication*>(qApp)->getMegaApi();
    const auto userMail = megaApi->getContact(notification->getData().toUtf8());
    auto url = QUrl(QString::fromUtf8("mega://#fm/contacts"));
    const auto userVisible = userMail && userMail->getVisibility() == mega::MegaUser::VISIBILITY_VISIBLE;
    if (userVisible)
    {
        const QString userHandle{QString::fromUtf8(megaApi->userHandleToBase64(userMail->getHandle()))};
        const bool actionIsViewContact{checkIfActionIsValid(activationButton)};
        const bool actionIsOpenChat{activationButton == MegaNotification::Action::secondButton};
        if(actionIsViewContact)
        {
            url = QUrl(QString::fromUtf8("mega://#fm/%1").arg(userHandle));
        }
        else if(actionIsOpenChat)
        {
            url = QUrl(QString::fromUtf8("mega://#fm/chat/p/%1").arg(userHandle));
        }
    }
    QtConcurrent::run(QDesktopServices::openUrl, url);
    delete userMail;
}

void DesktopNotifications::sendOverStorageNotification(int state) const
{
    switch (state)
    {
    case Preferences::STATE_ALMOST_OVER_STORAGE:
    {
        auto notification = new MegaNotification();
        notification->setTitle(tr("Your account is almost full."));
        notification->setText(tr("Upgrade now to a Pro account."));
        notification->setActions(QStringList() << tr("Get Pro"));
        notification->setImage(mAppIcon);
        notification->setImagePath(mStorageQuotaWarningIconPath);
        connect(notification, &MegaNotification::activated, this, &DesktopNotifications::redirectToUpgrade);
        mNotificator->notify(notification);
        break;
    }
    case Preferences::STATE_OVER_STORAGE:
    {
        auto notification = new MegaNotification();
        notification->setTitle(tr("Your account is full."));
        notification->setText(tr("Upgrade now to a Pro account."));
        notification->setActions(QStringList() << tr("Get Pro"));
        notification->setImage(mAppIcon);
        notification->setImagePath(mStorageQuotaFullIconPath);
        connect(notification, &MegaNotification::activated, this, &DesktopNotifications::redirectToUpgrade);
        mNotificator->notify(notification);
        break;
    }
    case Preferences::STATE_PAYWALL:
    {
        auto notification = new MegaNotification();
        notification->setTitle(tr("Your data is at risk"));
        const auto megaApi = static_cast<MegaApplication*>(qApp)->getMegaApi();
        int64_t remainDaysOut(0);
        Utilities::getDaysToTimestamp(megaApi->getOverquotaDeadlineTs(), remainDaysOut);
        notification->setText(tr("You have %n day left to save your data", "", static_cast<int>(remainDaysOut)));
        notification->setActions(QStringList() << tr("Get Pro"));
        notification->setImage(mAppIcon);
        connect(notification, &MegaNotification::activated, this, &DesktopNotifications::redirectToUpgrade);
        mNotificator->notify(notification);
        break;
    }
    default:
        break;
    }
}

void DesktopNotifications::sendOverTransferNotification(const QString &title) const
{
    const auto notification = new MegaNotification();
    notification->setTitle(title);
    notification->setText(tr("Upgrade now to a Pro account."));
    notification->setActions(QStringList() << tr("Get Pro"));
    notification->setImage(mAppIcon);
    connect(notification, &MegaNotification::activated, this, &DesktopNotifications::redirectToUpgrade);
    mNotificator->notify(notification);
}

void DesktopNotifications::sendFinishedTransferNotification(const QString &title, const QString &message, const QString &extraData) const
{
    if(mPreferences->isNotificationEnabled(Preferences::NotificationsTypes::INFO_MESSAGES))
    {
        auto notification = new MegaNotification();
        notification->setTitle(title);
        notification->setText(message);
        notification->setActions(QStringList() << tr("Show in folder"));
        notification->setData(extraData);
        notification->setImage(mAppIcon);
        notification->setImagePath(mFileDownloadSucceedIconPath);
        connect(notification, &MegaNotification::activated, this, &DesktopNotifications::showInFolder);
        mNotificator->notify(notification);
    }
}

void DesktopNotifications::redirectToUpgrade(MegaNotification::Action activationButton) const
{
    if (checkIfActionIsValid(activationButton))
    {
        QString url = QString::fromUtf8("mega://#pro");
        Utilities::getPROurlWithParameters(url);
        QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
    }
}

void DesktopNotifications::sendBusinessWarningNotification(int businessStatus) const
{
    const auto megaApi = static_cast<MegaApplication*>(qApp)->getMegaApi();

    switch (businessStatus)
    {
    case mega::MegaApi::BUSINESS_STATUS_GRACE_PERIOD:
    {
        if (megaApi->isMasterBusinessAccount())
        {
            const auto notification = new MegaNotification();
            notification->setTitle(tr("Payment Failed"));
            notification->setText(tr("Please resolve your payment issue to avoid suspension of your account."));
            notification->setActions(QStringList() << tr("Pay Now"));
            notification->setImage(mAppIcon);
            connect(notification, &MegaNotification::activated, this, &DesktopNotifications::redirectToPayBusiness);
            mNotificator->notify(notification);
        }
        break;
    }
    case mega::MegaApi::BUSINESS_STATUS_EXPIRED:
    {
        const auto notification = new MegaNotification();

        if (megaApi->isMasterBusinessAccount())
        {
            notification->setTitle(tr("Your Business account is expired"));
            notification->setText(tr("Your account is suspended as read only until you proceed with the needed payments."));
            notification->setActions(QStringList() << tr("Pay Now"));
            notification->setImage(mAppIcon);
            connect(notification, &MegaNotification::activated, this, &DesktopNotifications::redirectToPayBusiness);
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

void DesktopNotifications::sendInfoNotification(const QString &title, const QString &message) const
{
    if(mPreferences->isNotificationEnabled(Preferences::NotificationsTypes::INFO_MESSAGES))
    {
        mNotificator->notify(Notificator::Information, title, message, mAppIcon);
    }
}

//Warning notifications are not in used. If in a future they are used, keep in mind whether they should
//be included in notifications settings or sent always
void DesktopNotifications::sendWarningNotification(const QString &title, const QString &message) const
{
     mNotificator->notify(Notificator::Warning, title, message, mAppIcon);
}

//Error notifications are not in used. If in a future they are used, keep in mind whether they should
//be included in notifications settings or sent always
void DesktopNotifications::sendErrorNotification(const QString &title, const QString &message) const
{
    mNotificator->notify(Notificator::Warning, title, message, mAppIcon);
}

void DesktopNotifications::redirectToPayBusiness(MegaNotification::Action activationButton) const
{
    if (checkIfActionIsValid(activationButton))
    {
        QString url = QString::fromUtf8("mega://#repay");
        Utilities::getPROurlWithParameters(url);
        QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
    }
}

void DesktopNotifications::showInFolder(MegaNotification::Action action) const
{
    const auto notification = static_cast<MegaNotification*>(QObject::sender());

    if (checkIfActionIsValid(action) && !notification->getData().isEmpty())
    {
        QString localPath = QDir::toNativeSeparators(notification->getData());
        Platform::showInFolder(localPath);
    }
}

void DesktopNotifications::viewShareOnWebClient(MegaNotification::Action action) const
{
    if (checkIfActionIsValid(action))
    {
        const auto notification = static_cast<MegaNotification*>(QObject::sender());
        const auto nodeHandlerBase64 = notification->getData();
        if (!nodeHandlerBase64.isEmpty())
        {
            const auto url = QUrl(QString::fromUtf8("mega://#fm/%1").arg(nodeHandlerBase64));
            QtConcurrent::run(QDesktopServices::openUrl, url);
        }
    }
}

void DesktopNotifications::receiveClusteredAlert(mega::MegaUserAlert *alert, const QString &message) const
{
    switch (alert->getType())
    {
    case mega::MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
    {
        notifySharedUpdate(alert, message, REMOVED_SHARED_NODES);
    }
    }
}

void DesktopNotifications::replayNewShareReceived(MegaNotification::Action action) const
{
    const bool actionIsViewOnWebClient{checkIfActionIsValid(action)};
    const bool actionIsSyncShare{action == MegaNotification::Action::secondButton};
    if(actionIsViewOnWebClient)
    {
        viewShareOnWebClient(action);
    }
    else if(actionIsSyncShare)
    {
        const auto notification = static_cast<MegaNotification*>(QObject::sender());
        const auto nodeHandlerBase64 = notification->getData();
        if (!nodeHandlerBase64.isEmpty())
        {
            const auto megaFolderHandle = mega::MegaApi::base64ToUserHandle(nodeHandlerBase64.toStdString().c_str());
            const auto megaApp = static_cast<MegaApplication*>(qApp);
            megaApp->openSettingsAddSync(megaFolderHandle);
        }
    }
}

void DesktopNotifications::viewOnInfoDialogNotifications(MegaNotification::Action action) const
{
    if(checkIfActionIsValid(action))
    {
        const auto megaApp = static_cast<MegaApplication*>(qApp);
        megaApp->showInfoDialogNotifications();
    }
}

void DesktopNotifications::OnUserAttributesReady()
{
    auto UserAttribute = dynamic_cast<UserAttributes::FullNameAttributeRequest*>(sender());
    if(UserAttribute)
    {
        auto pendingAlerts = mPendingUserAlerts.values(UserAttribute->getEmail());
        foreach(auto alert, pendingAlerts)
        {
            processAlert(alert);
            delete alert;
        }
        mPendingUserAlerts.remove(UserAttribute->getEmail());
        mUserAttributes.remove(UserAttribute->getEmail());

        //After processing the alerts, disconnect the full name attribute request as it still lives
        //in attributes manager
        disconnect(UserAttribute, &UserAttributes::FullNameAttributeRequest::attributeReady,
                this, &DesktopNotifications::OnUserAttributesReady);
    }
}
