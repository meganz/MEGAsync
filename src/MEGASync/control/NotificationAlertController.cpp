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

void NotificationAlertController::populateUserAlerts(mega::MegaUserAlertList* alertList, bool copyRequired)
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
        AlertModel* alertsModel = new AlertModel(alertList, copyRequired, this);
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

    if (!copyRequired)
    {
        alertList->clear(); //empty the list otherwise they will be deleted
        delete alertList;
    }
}

void NotificationAlertController::onUserAlertsUpdate(mega::MegaApi* api, mega::MegaUserAlertList* list)
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

bool NotificationAlertController::alertsAreFiltered()
{
    return mAlertsProxyModel && mAlertsProxyModel->filterAlertType() != NotificationAlertProxyModel::FilterType::ALL;
}

bool NotificationAlertController::hasAlerts()
{
    return mNotificationAlertModel->hasAlerts();
}

bool NotificationAlertController::hasAlertsOfType(int type)
{
    return mNotificationAlertModel->hasAlertsOfType(type);
}

void NotificationAlertController::applyNotificationFilter(NotificationAlertProxyModel::FilterType opt)
{
    if (mAlertsProxyModel)
    {
        mAlertsProxyModel->setFilterAlertType(opt);
    }
}
