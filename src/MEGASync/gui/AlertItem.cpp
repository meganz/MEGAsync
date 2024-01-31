#include "AlertItem.h"
#include "ui_AlertItem.h"
#include "CommonMessages.h"
#include "MegaApplication.h"
#include "UserAttributesRequests/FullName.h"
#include <MegaNodeNames.h>
#include "EmailRequester.h"

#include <QDateTime>
#include <QFutureWatcher>
#include <QFuture>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

AlertItem::AlertItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AlertItem),
    megaApi(MegaSyncApp->getMegaApi())
{
    ui->setupUi(this);

    ui->sIconWidget->hide();
    ui->wNotificationIcon->hide();
    ui->lNew->hide();

    connect(&mAlertNodeWatcher, &QFutureWatcher<void>::finished, this, [=](){

        mAlertNode.reset(static_cast<MegaNode*>(mAlertNodeWatcher.result()));

        setAlertType(mAlertUser->getType());
        setAlertHeading(mAlertUser);
        setAlertContent(mAlertUser);
        setAlertTimeStamp(mAlertUser->getTimestamp(0));
        mAlertUser->getSeen() ? ui->lNew->hide() : ui->lNew->show();

        emit refreshAlertItem(mAlertUser->getId());
        });
}

AlertItem::~AlertItem()
{
    delete ui;
}

void AlertItem::setAlertData(MegaUserAlertExt* alert)
{
    mAlertUser = alert;

    if (alert->getUserHandle() != INVALID_HANDLE)
    {
        if (alert->getEmail())
        {
            mEmail = QString::fromUtf8(alert->getEmail());
            requestFullName();
        }
        else
        {
            requestEmail(alert);
        }
    }
    else //If it comes without user handler, it is because is an own alert, then take your email.
    {
        if(megaApi)
        {
            ui->wAvatarContact->setUserEmail(megaApi->getMyEmail());
        }
    }

    connect(ui->wAvatarContact, &AvatarWidget::avatarUpdated, this, [this](){
        emit refreshAlertItem(mAlertUser->getId());
    });

    onAttributesReady();
}

void AlertItem::contactEmailChanged(QString email)
{
    if (mEmail != email)
    {
        requestFullName();
    }
}

void AlertItem::requestEmail(MegaUserAlertExt* alert)
{
    EmailRequester* request = new EmailRequester(alert->getUserHandle());

    connect(request, &EmailRequester::emailReceived, this, &AlertItem::contactEmailChanged, Qt::QueuedConnection);

    request->requestEmail();
}

void AlertItem::requestFullName()
{
    if (!mEmail.isEmpty())
    {
        mFullNameAttributes = UserAttributes::FullName::requestFullName(mEmail.toStdString().c_str());

        if(mFullNameAttributes)
        {
            connect(mFullNameAttributes.get(), &UserAttributes::FullName::fullNameReady, this, &AlertItem::onAttributesReady);
        }

        ui->wAvatarContact->setUserEmail(mEmail.toStdString().c_str());
    }

    onAttributesReady();
}

void AlertItem::onAttributesReady()
{
    MegaHandle handle = mAlertUser->getNodeHandle();

    if (handle != INVALID_HANDLE)
    {
        mAlertNodeWatcher.setFuture(QtConcurrent::run([=]()
        {
            return megaApi ? megaApi->getNodeByHandle(handle) : nullptr;
        }));
    }
    else
    {
        setAlertType(mAlertUser->getType());
        setAlertHeading(mAlertUser);
        setAlertContent(mAlertUser);
        setAlertTimeStamp(mAlertUser->getTimestamp(0));
        mAlertUser->getSeen() ? ui->lNew->hide() : ui->lNew->show();
        emit refreshAlertItem(mAlertUser->getId());
    }
}

void AlertItem::setAlertType(int type)
{
    ui->wNotificationIcon->hide();

    QString notificationTitle;
    QString notificationColor;
    switch (type)
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
                if (type == MegaUserAlert::TYPE_DELETEDSHARE)
                {
                    ui->bSharedFolder->setIcon(QIcon(QString::fromUtf8(":/images/icons/folder/small-folder-disabled.png")).pixmap(24.0, 24.0));
                }
                else
                {
                    ui->bSharedFolder->setIcon(QIcon(QString::fromUtf8(":/images/icons/folder/small-folder.png")).pixmap(24.0, 24.0));
                }

                ui->bNotificationIcon->setMinimumSize(QSize(10, 8));
                ui->bNotificationIcon->setMaximumSize(QSize(10, 8));
                ui->bNotificationIcon->setIconSize(QSize(10, 8));
                ui->bNotificationIcon->setIcon(QIcon(QString::fromLatin1("://images/share_arrow.png")));
                ui->wNotificationIcon->show();
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
                ui->bNotificationIcon->setMinimumSize(QSize(16, 16));
                ui->bNotificationIcon->setMaximumSize(QSize(16, 16));
                ui->bNotificationIcon->setIconSize(QSize(16, 16));
                ui->bNotificationIcon->setIcon(QIcon(QString::fromLatin1("://images/mega_notifications.png")));
                ui->wNotificationIcon->show();
            }
                break;
    }

    ui->lTitle->setStyleSheet(QString::fromLatin1("#lTitle { font-family: Lato; font-weight: 900; font-size: 10px; color: %1; } ").arg(notificationColor));
    ui->lTitle->setText(notificationTitle);
}

void AlertItem::setAlertHeading(MegaUserAlertExt* alert)
{
    ui->sIconWidget->hide();
    mNotificationHeading.clear();

    switch (alert->getType())
    {
        // Contact notifications
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
        case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
        {
            mNotificationHeading = tr("New Contact Request");
            ui->sIconWidget->setCurrentWidget(ui->pContact);
            ui->sIconWidget->show();
            break;
        }
        case MegaUserAlert::TYPE_CONTACTCHANGE_DELETEDYOU:
        case MegaUserAlert::TYPE_CONTACTCHANGE_ACCOUNTDELETED:
        {
            mNotificationHeading = tr("Contact Deleted");
            ui->sIconWidget->setCurrentWidget(ui->pContact);
            ui->sIconWidget->show();
            break;
        }
        case MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
        {
            mNotificationHeading = tr("Contact Established");
            ui->sIconWidget->setCurrentWidget(ui->pContact);
            ui->sIconWidget->show();
            break;
        }
        case MegaUserAlert::TYPE_CONTACTCHANGE_BLOCKEDYOU:
        {
            mNotificationHeading = tr("Contact Blocked");
            ui->sIconWidget->setCurrentWidget(ui->pContact);
            ui->sIconWidget->show();
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED:
        {
            mNotificationHeading = tr("Contact Updated");
            ui->sIconWidget->setCurrentWidget(ui->pContact);
            ui->sIconWidget->show();
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED:
        {
            mNotificationHeading = tr("Contact Accepted");
            ui->sIconWidget->setCurrentWidget(ui->pContact);
            ui->sIconWidget->show();
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_DENIED:
        {
            mNotificationHeading = tr("Contact Denied");
            ui->sIconWidget->setCurrentWidget(ui->pContact);
            ui->sIconWidget->show();
            break;
        }
        // Share notifications
        case MegaUserAlert::TYPE_NEWSHARE:
        case MegaUserAlert::TYPE_DELETEDSHARE:
        case MegaUserAlert::TYPE_NEWSHAREDNODES:
        case MegaUserAlert::TYPE_REMOVEDSHAREDNODES:
        {
            ui->sIconWidget->setCurrentWidget(ui->pSharedFolder);
            ui->sIconWidget->show();
            mNotificationHeading = MegaNodeNames::getNodeName(mAlertNode.get());

            if (mNotificationHeading.isEmpty())
            {
                mNotificationHeading = tr("Shared Folder Activity");
            }
            break;
        }
        case MegaUserAlert::TYPE_UPDATEDSHAREDNODES:
        {
            ui->sIconWidget->setCurrentWidget(ui->pSharedFolder);
            ui->sIconWidget->show();
            mNotificationHeading = MegaNodeNames::getNodeName(mAlertNode.get());

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


    ui->lHeading->ensurePolished();
    ui->lHeading->setText(ui->lHeading->fontMetrics().elidedText(mNotificationHeading, Qt::ElideMiddle,ui->lHeading->minimumWidth()));

    if(!mEmail.isEmpty())
    {
        mNotificationHeading.append(QString::fromLatin1(" (") + mEmail + QString::fromLatin1(")"));
        setToolTip(mNotificationHeading);
    }
}

void AlertItem::setAlertContent(MegaUserAlertExt *alert)
{
    QString notificationContent;
    switch (alert->getType())
    {
            // Contact notifications
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REQUEST:
                notificationContent = tr("[A] sent you a contact request")
                        .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
                break;
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_CANCELLED:
                notificationContent = tr("[A] cancelled their contact request")
                        .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
                break;
            case MegaUserAlert::TYPE_INCOMINGPENDINGCONTACT_REMINDER:
                notificationContent = tr("Reminder: You have a contact request");
                break;
            case MegaUserAlert::TYPE_CONTACTCHANGE_DELETEDYOU:
                notificationContent = tr("[A] deleted you as a contact")
                        .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
                break;
            case MegaUserAlert::TYPE_CONTACTCHANGE_ACCOUNTDELETED:
                notificationContent = tr("[A] has been deleted/deactivated")
                        .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
                break;
            case MegaUserAlert::TYPE_CONTACTCHANGE_CONTACTESTABLISHED:
                notificationContent = tr("[A] accepted your contact request")
                        .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
                break;
            case MegaUserAlert::TYPE_CONTACTCHANGE_BLOCKEDYOU:
                notificationContent = tr("[A] blocked you as contact")
                        .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
                break;
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_IGNORED:
                notificationContent = tr("You ignored a contact request");
                break;
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_ACCEPTED:
                notificationContent = tr("You accepted a contact request");
                break;
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTINCOMING_DENIED:
                notificationContent = tr("You denied a contact request");
                break;
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_ACCEPTED:
                notificationContent = tr("[A] accepted your contact request")
                        .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
                break;
            case MegaUserAlert::TYPE_UPDATEDPENDINGCONTACTOUTGOING_DENIED:
                notificationContent = tr("[A] denied your contact request")
                        .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
                break;
            // Share notifications
            case MegaUserAlert::TYPE_NEWSHARE:
                notificationContent = tr("New shared folder from [A]")
                        .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
                break;
            case MegaUserAlert::TYPE_DELETEDSHARE:
            {
                if (alert->getNumber(0) == 0) //Someone left the folder
                {
                    notificationContent = tr("[A] has left the shared folder")
                            .replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()));
                }
                else //Access for the user was removed by share owner
                {
                    notificationContent = !mEmail.isEmpty() ? tr("Access to shared folder was removed by [A]").replace(QString::fromUtf8("[A]"), formatRichString(getUserFullName()))
                                                            : tr("Access to shared folder was removed");
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
                notificationContent = tr("Your payment for the [A] plan was received")
                        .replace(QString::fromUtf8("[A]"), alert->getString(0) ? QString::fromUtf8(alert->getString(0)) : QString::fromUtf8(""));
                break;
            case MegaUserAlert::TYPE_PAYMENT_FAILED:
                notificationContent = tr("Your payment for the [A] plan was unsuccessful")
                        .replace(QString::fromUtf8("[A]"), alert->getString(0) ? QString::fromUtf8(alert->getString(0)) : QString::fromUtf8(""));
                break;
            case MegaUserAlert::TYPE_PAYMENTREMINDER:
            {
                notificationContent = CommonMessages::createPaymentReminder(alert->getTimestamp(1));
                break;
            }
            // Takedown notifications
            case MegaUserAlert::TYPE_TAKEDOWN:
            {
                if (mAlertNode)
                {
                    if (mAlertNode->getType() == MegaNode::TYPE_FILE)
                    {
                        notificationContent = tr("Your publicly shared file ([A]) has been taken down")
                                .replace(QString::fromUtf8("[A]"), formatRichString(MegaNodeNames::getNodeName(mAlertNode.get())));
                    }
                    else if (mAlertNode->getType() == MegaNode::TYPE_FOLDER)
                    {
                        notificationContent = tr("Your publicly shared folder ([A]) has been taken down")
                                .replace(QString::fromUtf8("[A]"), formatRichString(MegaNodeNames::getNodeName(mAlertNode.get())));
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
                if (mAlertNode)
                {
                    if (mAlertNode->getType() == MegaNode::TYPE_FILE)
                    {
                        notificationContent = tr("Your publicly shared file ([A]) has been reinstated")
                                .replace(QString::fromUtf8("[A]"), formatRichString(MegaNodeNames::getNodeName(mAlertNode.get())));
                    }
                    else if (mAlertNode->getType() == MegaNode::TYPE_FOLDER)
                    {
                        notificationContent = tr("Your publicly shared folder ([A]) has been reinstated")
                                .replace(QString::fromUtf8("[A]"), formatRichString(MegaNodeNames::getNodeName(mAlertNode.get())));
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
                notificationContent = QString::fromUtf8(alert->getTitle());
                break;
    }

    ui->lDesc->setText(notificationContent);
}

void AlertItem::setAlertTimeStamp(int64_t ts)
{
    if (ts != -1)
    {
        const QDateTime dateTime{QDateTime::fromMSecsSinceEpoch(ts * 1000)};
        ui->lTimeStamp->setText(MegaSyncApp->getFormattedDateByCurrentLanguage(dateTime));
    }
    else
    {
        ui->lTimeStamp->setText(QString::fromUtf8(""));
    }
}

QString AlertItem::getHeadingString()
{
    return mNotificationHeading;
}

QSize AlertItem::minimumSizeHint() const
{
    return QSize(400, 122);
}

QSize AlertItem::sizeHint() const
{
    return QSize(400, 122);
}

MegaHandle AlertItem::getContactHandle() const
{
    if (mAlertUser != nullptr)
    {
        return mAlertUser->getUserHandle();
    }

    return INVALID_HANDLE;
}

void AlertItem::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        setAlertType(mAlertUser->getType());
        setAlertHeading(mAlertUser);
        setAlertContent(mAlertUser);
        setAlertTimeStamp(mAlertUser->getTimestamp(0));
    }
    QWidget::changeEvent(event);
}

QString AlertItem::formatRichString(QString str)
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

    return mEmail;
}

