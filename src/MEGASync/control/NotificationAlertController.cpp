#include "NotificationAlertController.h"

#include "MegaApplication.h"

NotificationAlertController::NotificationAlertController(QObject* parent)
    : QObject(parent)
    , mMegaApi(MegaSyncApp->getMegaApi())
    , mDelegateListener(std::make_unique<mega::QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
    , mGlobalListener(std::make_unique<mega::QTMegaGlobalListener>(MegaSyncApp->getMegaApi(), this))
    , mNotificationAlertModel(nullptr)
    , mNotificationAlertDelegate(nullptr)
{
    mMegaApi->addRequestListener(mDelegateListener.get());
    mMegaApi->addGlobalListener(mGlobalListener.get());
}

void NotificationAlertController::onRequestFinish(mega::MegaApi* api, mega::MegaRequest* request, mega::MegaError* e)
{
    Q_UNUSED(api)

    switch(request->getType())
    {
        case mega::MegaRequest::TYPE_GET_NOTIFICATIONS:
        {
            if (e->getErrorCode() == mega::MegaError::API_OK)
            {
                const mega::MegaNotificationList* notifications = request->getMegaNotifications();
                if (notifications)
                {
                    bool exists = mNotificationAlertModel != nullptr;

                    if(!mNotificationAlertModel)
                    {
                        mNotificationAlertModel = std::make_unique<NotificationAlertModel>(this);
                        mAlertsProxyModel = std::make_unique<NotificationAlertProxyModel>(this);
                        mAlertsProxyModel->setSourceModel(mNotificationAlertModel.get());
                        mAlertsProxyModel->setSortRole(Qt::UserRole); //Role used to sort the model by date.
                    }
                    mNotificationAlertModel->createNotificationModel(notifications);

                    if(!mNotificationAlertDelegate)
                    {
                        mNotificationAlertDelegate = std::make_unique<NotificationAlertDelegate>(this);
                    }
                    mNotificationAlertDelegate->createNotificationDelegate(mNotificationAlertModel->notificationModel());

                    if(!exists)
                    {
                        emit notificationAlertCreated(mAlertsProxyModel.get(), mNotificationAlertDelegate.get());
                    }
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void NotificationAlertController::populateUserAlerts(mega::MegaUserAlertList* alertList)
{
    if (!alertList)
    {
        return;
    }

    // Used by DesktopNotifications because the current architecture
    emit userAlertsUpdated(alertList);

    if (mNotificationAlertModel)
    {
        mNotificationAlertModel->insertAlerts(alertList);
    }
    else
    {
        bool exists = mNotificationAlertModel != nullptr;

        if(!mNotificationAlertModel)
        {
            mNotificationAlertModel = std::make_unique<NotificationAlertModel>(this);
            mAlertsProxyModel = std::make_unique<NotificationAlertProxyModel>(this);
            mAlertsProxyModel->setSourceModel(mNotificationAlertModel.get());
            mAlertsProxyModel->setSortRole(Qt::UserRole); //Role used to sort the model by date.
        }
        mNotificationAlertModel->createAlertModel(alertList);

        if(!mNotificationAlertDelegate)
        {
            mNotificationAlertDelegate = std::make_unique<NotificationAlertDelegate>(this);
        }
        mNotificationAlertDelegate->createAlertDelegate(mNotificationAlertModel->alertModel());

        if(!exists)
        {
            emit notificationAlertCreated(mAlertsProxyModel.get(), mNotificationAlertDelegate.get());
        }
    }

    // Used by InfoDialog because the current architecture
    emit unseenAlertsChanged(mNotificationAlertModel->getUnseenNotifications());
}

void NotificationAlertController::onUserAlertsUpdate(mega::MegaApi* api, mega::MegaUserAlertList* list)
{
    Q_UNUSED(api)

    if (MegaSyncApp->finished())
    {
        return;
    }

    // TESTS - REMOVE ME

    mega::MegaIntegerList* listIds = mega::MegaIntegerList::createInstance();
    listIds->add(1);
    mMegaApi->enableTestNotifications(listIds);
    mMegaApi->getNotifications();

    // TESTS - REMOVE ME

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

bool NotificationAlertController::areAlertsFiltered()
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
