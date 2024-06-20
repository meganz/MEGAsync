#include "NotificationAlertController.h"

#include "MegaApplication.h"

NotificationAlertController::NotificationAlertController(QObject* parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mGlobalListener(std::make_unique<mega::QTMegaGlobalListener>(MegaSyncApp->getMegaApi(), this))
    , mNotificationAlertModel(nullptr)
    , mNotificationAlertDelegate(nullptr)
{
    mMegaApi->addGlobalListener(mGlobalListener.get());
}

void NotificationAlertController::populateUserAlerts(mega::MegaUserAlertList* alertList)
{
    if (!alertList)
    {
        return;
    }

    emit userAlertsUpdated(alertList);

    if (mNotificationAlertModel)
    {
        mNotificationAlertModel->insertAlerts(alertList);
    }
    else
    {
        AlertModel* alertsModel = new AlertModel(alertList, this);
        AlertDelegate* alertDelegate = new AlertDelegate(alertsModel, this);

        NotificationModel* notificationsModel = new NotificationModel(this);
        NotificationDelegate* notificationDelegate = new NotificationDelegate(notificationsModel, this);

        mNotificationAlertModel = std::make_unique<NotificationAlertModel>(notificationsModel, alertsModel, this);
        mNotificationAlertDelegate = std::make_unique<NotificationAlertDelegate>(notificationDelegate, alertDelegate, this);

        mAlertsProxyModel = std::make_unique<NotificationAlertProxyModel>(this);
        mAlertsProxyModel->setSourceModel(mNotificationAlertModel.get());
        mAlertsProxyModel->setSortRole(Qt::UserRole); //Role used to sort the model by date.

        emit notificationAlertCreated(mAlertsProxyModel.get(), mNotificationAlertDelegate.get());
    }

    emit unseenAlertsChanged(mNotificationAlertModel->getUnseenNotifications());
}

void NotificationAlertController::onUserAlertsUpdate(mega::MegaApi* api, mega::MegaUserAlertList* list)
{
    Q_UNUSED(api)

    if (MegaSyncApp->finished())
    {
        return;
    }

    if (list != nullptr)
    {
        // Process synchronously if list is provided
        populateUserAlerts(list);
    }
    else
    {
        // Process asynchronously if list is not provided
        ThreadPoolSingleton::getInstance()->push([this]()
        {
            // Retrieve the alerts in a separate thread
            mega::MegaUserAlertList* alertList = mMegaApi->getUserAlerts();

            // Queue the processing back to the main thread
            Utilities::queueFunctionInAppThread([this, alertList]()
            {
                populateUserAlerts(alertList);
                alertList->clear();
                delete alertList;
            });
        });
    }
}

void NotificationAlertController::reset()
{
    if (mNotificationAlertModel)
    {
        mNotificationAlertModel.reset();
    }

    if (mAlertsProxyModel)
    {
        mAlertsProxyModel.reset();
    }

    if (mNotificationAlertDelegate)
    {
        mNotificationAlertDelegate.reset();
    }
}

bool NotificationAlertController::alertsAreFiltered()
{
    return mAlertsProxyModel && mAlertsProxyModel->filterAlertType() != AlertType::ALL;
}

bool NotificationAlertController::hasAlerts()
{
    return mNotificationAlertModel && mNotificationAlertModel->hasAlerts();
}

bool NotificationAlertController::hasAlertsOfType(int type)
{
    return mNotificationAlertModel && mNotificationAlertModel->hasAlertsOfType(type);
}

void NotificationAlertController::applyNotificationFilter(AlertType opt)
{
    if (mAlertsProxyModel)
    {
        mAlertsProxyModel->setFilterAlertType(opt);
    }
}
