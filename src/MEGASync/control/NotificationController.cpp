#include "NotificationController.h"

#include "MegaApplication.h"

NotificationController::NotificationController(QObject* parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mGlobalListener(std::make_unique<mega::QTMegaGlobalListener>(MegaSyncApp->getMegaApi(), this))
    , mNotificationAlertModel(nullptr)
    , mNotificationAlertDelegate(nullptr)
{
    mMegaApi->addGlobalListener(mGlobalListener.get());
}

void NotificationController::populateUserAlerts(mega::MegaUserAlertList* alertList, bool copyRequired)
{
    if (!alertList)
    {
        return;
    }

    emit userAlertsUpdated(alertList);

    if (mNotificationAlertModel)
    {
        mNotificationAlertModel->insertAlerts(alertList, copyRequired);
    }
    else
    {
        AlertModel* alertsModel = new AlertModel(alertList, copyRequired);
        AlertDelegate* alertDelegate = new AlertDelegate(alertsModel, true, this);

        QNotificationsModel* notificationsModel = new QNotificationsModel();
        MegaNotificationDelegate* notificationDelegate = new MegaNotificationDelegate(notificationsModel, this);

        mNotificationAlertModel = new NotificationAlertModel(notificationsModel, alertsModel);
        mNotificationAlertDelegate = new NotificationAlertDelegate(notificationDelegate, alertDelegate);

        mAlertsProxyModel = new NotificationAlertProxyModel();
        mAlertsProxyModel->setSourceModel(mNotificationAlertModel);
        mAlertsProxyModel->setSortRole(Qt::UserRole); //Role used to sort the model by date.

        emit notificationAlertCreated(mAlertsProxyModel, mNotificationAlertDelegate);
    }

    emit unseenAlertsChanged(mNotificationAlertModel->getUnseenNotifications());

    if (!copyRequired)
    {
        alertList->clear(); //empty the list otherwise they will be deleted
        delete alertList;
    }
}

void NotificationController::onUserAlertsUpdate(mega::MegaApi* api, mega::MegaUserAlertList* list)
{
    Q_UNUSED(api)

    if (MegaSyncApp->finished())
    {
        return;
    }

    // if we have a list, we don't need to query megaApi for it and block the sdk mutex, we do this
    // synchronously, since we are not copying the list, and we need to process it before it goes out of scope.
    bool doSynchronously{list != NULL};

    if (doSynchronously)
    {
        populateUserAlerts(list, true);
    }
    else
    {
        auto funcToThreadPool = [this]()
        { //thread pool function
            mega::MegaUserAlertList* theList;
            theList = mMegaApi->getUserAlerts();
            //queued function
            Utilities::queueFunctionInAppThread([this, theList]()
            {
                populateUserAlerts(theList, false);
            });
        }; // end of thread pool function
        ThreadPoolSingleton::getInstance()->push(funcToThreadPool);
    }
}

bool NotificationController::alertsAreFiltered()
{
    return mAlertsProxyModel && mAlertsProxyModel->filterAlertType() != NotificationAlertProxyModel::FilterType::ALL;
}

bool NotificationController::hasAlerts()
{
    return mNotificationAlertModel->hasAlerts();
}

bool NotificationController::hasAlertsOfType(int type)
{
    return mNotificationAlertModel->hasAlertsOfType(type);
}

void NotificationController::applyNotificationFilter(NotificationAlertProxyModel::FilterType opt)
{
    if (mAlertsProxyModel)
    {
        mAlertsProxyModel->setFilterAlertType(opt);
    }
}
